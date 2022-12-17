/**
 * marketdataservice.hpp
 * Defines the data types and Service for order book market data.
 *
 * @author Breman Thuraisingham
 */
#ifndef MARKET_DATA_SERVICE_HPP
#define MARKET_DATA_SERVICE_HPP

#include <string>
#include <vector>
#include <unordered_map>
#include <fstream>
#include "soa.hpp"
#include <sstream>

using namespace std;

// Side for market data
enum PricingSide { BID, OFFER };

/**
 * A market data order with price, quantity, and side.
 */
class Order
{

public:

  // ctor for an order
	Order() = default;
  Order(double _price, long _quantity, PricingSide _side);

  // Get the price on the order
  double GetPrice() const;

  // Get the quantity on the order
  long GetQuantity() const;

  // Get the side on the order
  PricingSide GetSide() const;

private:
  double price;
  long quantity;
  PricingSide side;

};

/**
 * Class representing a bid and offer order
 */
class BidOffer
{

public:

  // ctor for bid/offer
  BidOffer(const Order &_bidOrder, const Order &_offerOrder);

  // Get the bid order
  const Order& GetBidOrder() const;

  // Get the offer order
  const Order& GetOfferOrder() const;

private:
  Order bidOrder;
  Order offerOrder;

};

/**
 * Order book with a bid and offer stack.
 * Type T is the product type.
 */
template<typename T>
class OrderBook
{

public:

  // ctor for the order book
	OrderBook() = default;
  OrderBook(const T &_product, const vector<Order> &_bidStack, const vector<Order> &_offerStack);

  // Get the product
  const T& GetProduct() const;

  // Get the bid stack
  const vector<Order>& GetBidStack() const;

  // Get the offer stack
  const vector<Order>& GetOfferStack() const;

  const BidOffer& GetBestBidOffer() const;

private:
  T product;
  vector<Order> bidStack;
  vector<Order> offerStack;

};



/**
 * Market Data Service which distributes market data
 * Keyed on product identifier.
 * Type T is the product type.
 */
template<typename T>
class MarketDataService : public Service<string,OrderBook <T> >
{
private:

	unordered_map<string, OrderBook<T>> orderBooks;
	vector<ServiceListener<OrderBook<T>>*> listeners;
	int bookDepth;
public:

	MarketDataService();

	// Get data on our service given a key
	OrderBook<T>& GetData(string _key);

	// The callback that a Connector should invoke for any new or updated data
	void OnMessage(OrderBook<T>& _data);

	// Add a listener to the Service for callbacks on add, remove, and update events for data to the Service
	void AddListener(ServiceListener<OrderBook<T>>* _listener);

	// Get all listeners on the Service
	const vector<ServiceListener<OrderBook<T>>*>& GetListeners() const;

  // Get the best bid/offer order
  const BidOffer& GetBestBidOffer(const string &ticker);

  // Aggregate the order book
  const OrderBook<T>& AggregateDepth(const string &ticker);

};


/**
* Market Data connector
* Keyed on product identifier.
* Type T is the product type.
*/
template<typename T>
class MDConnector : public Connector<OrderBook<T>>
{

private:

	MarketDataService<T>* MDS;

public:

	// Connector and Destructor
	MDConnector(MarketDataService<T>* service);

	// Publish data to the Connector
	void Publish(OrderBook<T>& data);

	// Subscribe data from the Connector
	void Consume(string file_name);

};


/*      implementation      */
Order::Order(double _price, long _quantity, PricingSide _side)
{
  price = _price;
  quantity = _quantity;
  side = _side;
}

double Order::GetPrice() const
{
  return price;
}
 
long Order::GetQuantity() const
{
  return quantity;
}
 
PricingSide Order::GetSide() const
{
  return side;
}

BidOffer::BidOffer(const Order &_bidOrder, const Order &_offerOrder) :
  bidOrder(_bidOrder), offerOrder(_offerOrder)
{
}

const Order& BidOffer::GetBidOrder() const
{
  return bidOrder;
}

const Order& BidOffer::GetOfferOrder() const
{
  return offerOrder;
}

template<typename T>
OrderBook<T>::OrderBook(const T &_product, const vector<Order> &_bidStack, const vector<Order> &_offerStack) :
  product(_product), bidStack(_bidStack), offerStack(_offerStack)
{
}

template<typename T>
const T& OrderBook<T>::GetProduct() const
{
  return product;
}

template<typename T>
const vector<Order>& OrderBook<T>::GetBidStack() const
{
  return bidStack;
}

template<typename T>
const vector<Order>& OrderBook<T>::GetOfferStack() const
{
  return offerStack;
}


template<typename T>
const BidOffer& OrderBook<T>::GetBestBidOffer() const
{
	double bestbid = bidStack[0].GetPrice();
	Order bestbidOrder;
	for (auto tmp : bidStack)
	{
		double price = tmp.GetPrice();
		if (price >= bestbid)
		{
			bestbid = price;
			bestbidOrder = tmp;
		}
	}

	double bestoffer = offerStack[0].GetPrice();
	Order bestofferOrder;
	for (auto tmp : offerStack)
	{
		double price = tmp.GetPrice();
		if (price  <= bestoffer)
		{
			bestoffer = price;
			bestofferOrder = tmp;
		}
	}

	return BidOffer(bestbidOrder, bestofferOrder);
}


template<typename T>
MarketDataService<T>::MarketDataService()
	:bookDepth(5) {}


template<typename T>
OrderBook<T>& MarketDataService<T>::GetData(string key)
{
	return orderBooks[key];
}

template<typename T>
void MarketDataService<T>::OnMessage(OrderBook<T>& data)
{
	orderBooks[data.GetProduct().GetTicker()] = data;

	for (auto l : listeners)
	{
		l->ProcessAdd(data);
	}
}

template<typename T>
void MarketDataService<T>::AddListener(ServiceListener<OrderBook<T>>* listener)
{
	listeners.push_back(listener);
}

template<typename T>
const vector<ServiceListener<OrderBook<T>>*>& MarketDataService<T>::GetListeners() const
{
	return listeners;
}


template<typename T>
const BidOffer& MarketDataService<T>::GetBestBidOffer(const string& ticker)
{
	return orderBooks[ticker].GetBestBidOffer();
}

template<typename T>
const OrderBook<T>& MarketDataService<T>::AggregateDepth(const string& ticker)
{
	return orderBooks[ticker];
}



template<typename T>
MDConnector<T>::MDConnector(MarketDataService<T>* service)
	:MDS(service) {}


template<typename T>
void MDConnector<T>::Publish(OrderBook<T>& _data) {}


template<typename T>
void MDConnector<T>::Consume(string file_name)
{
	ifstream file(file_name);
	string line;
	while (getline(file, line))
	{
		stringstream linestream(line); string block;
		vector<string> blocks;
		while (getline(linestream, block, ','))
		{
            if ((block[block.size()-1] == '\r') || (block[block.size()-1] == '\n')){
                block.erase(block.size()-1);
            }
			blocks.push_back(block);
		}

		vector<Order> bidStack;
		vector<Order> offerStack;

		for (int i = 1; i < 11; i += 2) {
			Order order(PriceSTD(blocks[i]), stol(blocks[i + 1]), BID);
			bidStack.push_back(order);
		}

		for (int i = 11; i < 21; i += 2) {
			Order order(PriceSTD(blocks[i]), stol(blocks[i + 1]), OFFER);
			offerStack.push_back(order);
		}
		OrderBook<T> orderBook(GetBond(blocks[0]), bidStack, offerStack);
		MDS->OnMessage(orderBook);
	}
}

#endif
