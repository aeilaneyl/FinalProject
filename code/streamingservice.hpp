/**
 * streamingservice.hpp
 * Defines the data types and Service for price streams.
 *
 * @author Breman Thuraisingham
 */
#ifndef STREAMING_SERVICE_HPP
#define STREAMING_SERVICE_HPP

#include "soa.hpp"
#include "marketdataservice.hpp"
#include "algostreamingservice.h"
#include <unordered_map>
#include "tools.h"

/**
 * A price stream order with price and quantity (visible and hidden)
 */
class PriceStreamOrder
{

public:

  // ctor for an order
	PriceStreamOrder() = default;
  PriceStreamOrder(double _price, long _visibleQuantity, long _hiddenQuantity, PricingSide _side);

  // The side on this order
  PricingSide GetSide() const;

  // Get the price on this order
  double GetPrice() const;

  // Get the visible quantity on this order
  long GetVisibleQuantity() const;

  // Get the hidden quantity on this order
  long GetHiddenQuantity() const;

  //get the string to print 
  string To_string();

private:
  double price;
  long visibleQuantity;
  long hiddenQuantity;
  PricingSide side;

};

/**
 * Price Stream with a two-way market.
 * Type T is the product type.
 */
template<typename T>
class PriceStream
{

public:

  // ctor
	PriceStream() = default;
  PriceStream(const T &_product, const PriceStreamOrder &_bidOrder, const PriceStreamOrder &_offerOrder);

  // Get the product
  const T& GetProduct() const;

  // Get the bid order
  const PriceStreamOrder& GetBidOrder() const;

  // Get the offer order
  const PriceStreamOrder& GetOfferOrder() const;

  //get the string to print 
  string To_string();
private:
  T product;
  PriceStreamOrder bidOrder;
  PriceStreamOrder offerOrder;

};


template<typename T>
class StreamingService;
template<typename T>
class AlgoStream;
/**
* Streaming listener to algostreaming service
* Keyed on product identifier.
* Type T is the product type.
*/
template<typename T>
class StreamingListener : public ServiceListener<AlgoStream<T>> {
private:
	StreamingService<T>* SS;
public:
	StreamingListener(StreamingService<T>* _service);
	// Listener callback to process an add event to the Service
	void ProcessAdd(AlgoStream<T>& data);

	// Listener callback to process a remove event to the Service
	void ProcessRemove(AlgoStream<T>& data);

	// Listener callback to process an update event to the Service
	void ProcessUpdate(AlgoStream<T>& data);
};
/**
 * Streaming service to publish two-way prices.
 * Keyed on product identifier.
 * Type T is the product type.
 */
template<typename T>
class StreamingService : public Service<string,PriceStream <T> >
{
private:
	ServiceListener<AlgoStream<T>>* listener;
	unordered_map<string, PriceStream<T>> priceStreams;
	vector<ServiceListener<PriceStream<T>>*> listeners;
public:
	StreamingService();
	// Get data on our service given a key
	PriceStream<T>& GetData(string key);

	// The callback that a Connector should invoke for any new or updated data
	void OnMessage(PriceStream<T>& data);

	// Add a listener to the Service for callbacks on add, remove, and update events for data to the Service
	void AddListener(ServiceListener<PriceStream<T>>* listener);

	// Get all listeners on the Service
	const vector<ServiceListener<PriceStream<T>>*>& GetListeners() const;

	// Get the stream service listener
	ServiceListener<AlgoStream<T>>* GetListener();
    // Publish two-way prices
    void PublishPrice(PriceStream<T>& priceStream);

};





/*         implementation      */


PriceStreamOrder::PriceStreamOrder(double _price, long _visibleQuantity, long _hiddenQuantity, PricingSide _side)
{
  price = _price;
  visibleQuantity = _visibleQuantity;
  hiddenQuantity = _hiddenQuantity;
  side = _side;
}

double PriceStreamOrder::GetPrice() const
{
  return price;
}

long PriceStreamOrder::GetVisibleQuantity() const
{
  return visibleQuantity;
}

long PriceStreamOrder::GetHiddenQuantity() const
{
  return hiddenQuantity;
}

string PriceStreamOrder::To_string() {
	string _side;
	switch (side)
	{
	case BID:
		_side = "Bid";
		break;
	case OFFER:
		_side = "Offer";
		break;
	}
	return _side + ": " + PriceDTS(price) + 
		" visibleQ " + to_string(visibleQuantity) +
		" hiddenQ " + to_string(hiddenQuantity);
}

template<typename T>
PriceStream<T>::PriceStream(const T &_product, const PriceStreamOrder &_bidOrder, const PriceStreamOrder &_offerOrder) :
  product(_product), bidOrder(_bidOrder), offerOrder(_offerOrder)
{
}

template<typename T>
const T& PriceStream<T>::GetProduct() const
{
  return product;
}

template<typename T>
const PriceStreamOrder& PriceStream<T>::GetBidOrder() const
{
  return bidOrder;
}

template<typename T>
const PriceStreamOrder& PriceStream<T>::GetOfferOrder() const
{
  return offerOrder;
}

template<typename T>
string PriceStream<T>::To_string() {
	string ticker = product.GetTicker();
	return product.GetTicker() + ", " +  bidOrder.To_string() + 
		", " + offerOrder.To_string();
}




template<typename T>
StreamingListener<T>::StreamingListener(StreamingService<T>* _service)
	:SS(_service){}


template<typename T>
void StreamingListener<T>::ProcessAdd(AlgoStream<T>& data)
{
	PriceStream<T>* _priceStream = data.GetPriceStream();
	SS->PublishPrice(*_priceStream);
}

template<typename T>
void StreamingListener<T>::ProcessRemove(AlgoStream<T>& data) {}

template<typename T>
void StreamingListener<T>::ProcessUpdate(AlgoStream<T>& data) {}



template<typename T>
StreamingService<T>::StreamingService()
	:listener(new StreamingListener<T>(this)) {}


template<typename T>
PriceStream<T>& StreamingService<T>::GetData(string key)
{
	return priceStreams[key];
}

template<typename T>
void StreamingService<T>::OnMessage(PriceStream<T>& data)
{
	priceStreams[data.GetProduct().GetTicker()] = data;
}

template<typename T>
void StreamingService<T>::AddListener(ServiceListener<PriceStream<T>>* _listener)
{
	listeners.push_back(_listener);
}

template<typename T>
const vector<ServiceListener<PriceStream<T>>*>& StreamingService<T>::GetListeners() const
{
	return listeners;
}

template<typename T>
ServiceListener<AlgoStream<T>>* StreamingService<T>::GetListener()
{
	return listener;
}

template<typename T>
void StreamingService<T>::PublishPrice(PriceStream<T>& priceStream)
{
	OnMessage(priceStream);
		for (auto& l : listeners)
		{
			l->ProcessAdd(priceStream);
		}
}

#endif
