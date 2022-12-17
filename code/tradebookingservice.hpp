/**
 * tradebookingservice.hpp
 * Defines the data types and Service for trade booking.
 *
 * @author Breman Thuraisingham
 */
#ifndef TRADE_BOOKING_SERVICE_HPP
#define TRADE_BOOKING_SERVICE_HPP

#include <string>
#include <vector>
#include "soa.hpp"
#include <fstream>
#include <unordered_map>
#include <sstream>
#include "algoexecutionservice.h"
// Trade sides
enum Side { BUY, SELL };

/**
 * Trade object with a price, side, and quantity on a particular book.
 * Type T is the product type.
 */
template<typename T>
class Trade
{

public:

  // ctor for a trade
	Trade() = default;
  Trade(const T &_product, string _tradeId, double _price, string _book, long _quantity, Side _side);

  // Get the product
  const T& GetProduct() const;

  // Get the trade ID
  const string& GetTradeId() const;

  // Get the mid price
  double GetPrice() const;

  // Get the book
  const string& GetBook() const;

  // Get the quantity
  long GetQuantity() const;

  // Get the side
  Side GetSide() const;

private:
  T product;
  string tradeId;
  double price;
  string book;
  long quantity;
  Side side;

};
template<typename T>
class TradeBookingService;
/**
* Trade Booking Service Listener 
* Type T is the product type.
*/
template<typename T>
class TBListener : public ServiceListener<ExecutionOrder<T>>
{

private:

	TradeBookingService<T>* TBS;
	long cnt;

public:

	// Connector and Destructor
	TBListener(TradeBookingService<T>* service);

	// Listener callback to process an add event to the Service
	void ProcessAdd(ExecutionOrder<T>& data);

	// Listener callback to process a remove event to the Service
	void ProcessRemove(ExecutionOrder<T>& data);

	// Listener callback to process an update event to the Service
	void ProcessUpdate(ExecutionOrder<T>& data);

};

/**
 * Trade Booking Service to book trades to a particular book.
 * Keyed on trade id.
 * Type T is the product type.
 */
template<typename T>
class TradeBookingService : public Service<string,Trade <T> >
{
private:
	unordered_map<string, Trade<T>> trades;
	vector<ServiceListener<Trade<T>>*> listeners;
	TBListener<T>* listener;
	
public:
	TradeBookingService();

	// Get data on our service given a key
	Trade<T>& GetData(string _key);

	// The callback that a Connector should invoke for any new or updated data
	void OnMessage(Trade<T>& _data);

	// Add a listener to the Service for callbacks on add, remove, and update events for data to the Service
	void AddListener(ServiceListener<Trade<T>>* _listener);

	// Get all listeners on the Service
	const vector<ServiceListener<Trade<T>>*>& GetListeners() const;

	// Get the listener of the service
	TBListener<T>* GetListener();
  // Book the trade
	void BookTrade(Trade<T> &trade);

};



template<typename T>
class TBSConnector : public Connector<Trade<T>>
{

private:

	TradeBookingService<T>* TBS;

public:

	TBSConnector(TradeBookingService<T>* service);

	// Publish data to the Connector
	void Publish(Trade<T>& _data);

	// Subscribe data from the Connector
	void Consume(string file_name);

};





/*     implementation   */
template<typename T>
Trade<T>::Trade(const T &_product, string _tradeId, double _price, string _book, long _quantity, Side _side) :
  product(_product)
{
  tradeId = _tradeId;
  price = _price;
  book = _book;
  quantity = _quantity;
  side = _side;
}

template<typename T>
const T& Trade<T>::GetProduct() const
{
  return product;
}

template<typename T>
const string& Trade<T>::GetTradeId() const
{
  return tradeId;
}

template<typename T>
double Trade<T>::GetPrice() const
{
  return price;
}

template<typename T>
const string& Trade<T>::GetBook() const
{
  return book;
}

template<typename T>
long Trade<T>::GetQuantity() const
{
  return quantity;
}

template<typename T>
Side Trade<T>::GetSide() const
{
  return side;
}

template<typename T>
TBListener<T>::TBListener(TradeBookingService<T>* service)
	:TBS(service), cnt(0) {}


template<typename T>
void TBListener<T>::ProcessAdd(ExecutionOrder<T>& data)
{
	T _product = data.GetProduct();

	Side _side;
	switch (data.getPricingSide())
	{
	case BID:
		_side = SELL;
		break;
	case OFFER:
		_side = BUY;
		break;
	}
	string _book;
	switch (cnt % 3)
	{
	case 0:
		_book = "TRSY1";
		break;
	case 1:
		_book = "TRSY2";
		break;
	case 2:
		_book = "TRSY3";
		break;
	}
	long Q = data.GetVisibleQuantity() + data.GetHiddenQuantity();
	cnt++;
	Trade<T> _trade(_product, data.GetOrderId(), data.GetPrice(), 
		_book, Q, _side);
	TBS->BookTrade(_trade);
}

template<typename T>
void TBListener<T>::ProcessRemove(ExecutionOrder<T>& _data) {}

template<typename T>
void TBListener<T>::ProcessUpdate(ExecutionOrder<T>& _data) {}


template<typename T>
TradeBookingService<T>::TradeBookingService()
	:listener(new TBListener<T>(this)) {}

template<typename T>
Trade<T>& TradeBookingService<T>::GetData(string key)
{
	return trades[key];
}

template<typename T>
void TradeBookingService<T>::OnMessage(Trade<T>& data)
{
	trades[data.GetTradeId()] = data;

	for (auto l : listeners)
	{
		l->ProcessAdd(data);
	}
}

template<typename T>
void TradeBookingService<T>::AddListener(ServiceListener<Trade<T>>* listener)
{
	listeners.push_back(listener);
}

template<typename T>
const vector<ServiceListener<Trade<T>>*>& TradeBookingService<T>::GetListeners() const
{
	return listeners;
}



template<typename T>
TBListener<T>* TradeBookingService<T>::GetListener()
{
	return listener;
}


template<typename T>
void TradeBookingService<T>::BookTrade(Trade<T>& trade)
{
	trades[trade.GetTradeId()] = trade;

	for (auto l : listeners)
	{
		l->ProcessAdd(trade);
	}
}





template<typename T>
TBSConnector<T>::TBSConnector(TradeBookingService<T>* service)
	:TBS(service) {}

template<typename T>
void TBSConnector<T>::Publish(Trade<T>& _data) {}

template<typename T>
void TBSConnector<T>::Consume(string file_name)
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

		Side side;
		if (blocks[2] == "Buy") side = BUY;
		else  side = SELL;
		Trade<T> _trade(GetBond(blocks[0]), blocks[1], PriceSTD(blocks[3]),
			blocks[5], stol(blocks[4]), side);
		TBS->BookTrade(_trade);
	}
}
#endif
