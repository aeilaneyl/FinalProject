#ifndef ALGOSTREAMINGSERVICEHPP
#define ALGOSTREAMINGSERVICEHPP

#include "soa.hpp"
#include "streamingservice.hpp"
#include "pricingservice.hpp"
#include <string>
#include <unordered_map>

/**
* AlgoStreaming
* Type T is the product type.
*/
template<typename T>
class AlgoStream {
private:
	PriceStream<T>* priceStream;
public:
	AlgoStream() = default;
	AlgoStream(PriceStream<T>* ps);
	PriceStream<T>* GetPriceStream();
};



template<typename T>
class AlgoStreamingService;

/**
* AlgoStreamingListener
* Type T is the product type.
*/
template<typename T>
class AlgoStreamingListener : public ServiceListener<Price<T>>
{

private:

	AlgoStreamingService<T>* AS;

public:

	AlgoStreamingListener(AlgoStreamingService<T>* _service);

	// Listener callback to process an add event to the Service
	void ProcessAdd(Price<T>& data);

	// Listener callback to process a remove event to the Service
	void ProcessRemove(Price<T>& data);

	// Listener callback to process an update event to the Service
	void ProcessUpdate(Price<T>& data);

};

/**
* AlgoStreamingService
* Type T is the product type.
*/
template<typename T>
class AlgoStreamingService : public Service<string, AlgoStream<T>>
{

private:
	unordered_map<string, AlgoStream<T>> algoStreams;
	vector<ServiceListener<AlgoStream<T>>*> listeners;
	ServiceListener<Price<T>>* algostrlistener;
	// whether the visible size is 1 million
	bool VisibleS1M;

public:

	AlgoStreamingService();

	// Get data on our service given a key
	AlgoStream<T>& GetData(string key);

	// The callback that a Connector should invoke for any new or updated data
	void OnMessage(AlgoStream<T>& data);

	// Add a listener to the Service for callbacks on add, remove, and update events for data to the Service
	void AddListener(ServiceListener<AlgoStream<T>>* listener);

	// Get all listeners on the Service
	const vector<ServiceListener<AlgoStream<T>>*>& GetListeners() const;

	// return the listener of the service
	ServiceListener<Price<T>>* GetListener();

	// Publish prices
	void PublishPrice(Price<T>& price);

};





/*   implementation     */
template<typename T>
AlgoStream<T>::AlgoStream(PriceStream<T>* ps): priceStream(ps) {}

template<typename T>
PriceStream<T>* AlgoStream<T>::GetPriceStream() {
	return priceStream;
}

template<typename T>
AlgoStreamingListener<T>::AlgoStreamingListener(AlgoStreamingService<T>* _service)
:AS(_service) {}

template<typename T>
void AlgoStreamingListener<T>::ProcessAdd(Price<T>& data)
{
	AS->PublishPrice(data);
}

template<typename T>
void AlgoStreamingListener<T>::ProcessRemove(Price<T>& data) {}

template<typename T>
void AlgoStreamingListener<T>::ProcessUpdate(Price<T>& data) {}


template<typename T>
AlgoStreamingService<T>::AlgoStreamingService()
:algostrlistener(new AlgoStreamingListener<T>(this)), VisibleS1M(true) {}

template<typename T>
AlgoStream<T>& AlgoStreamingService<T>::GetData(string key) {
	return algoStreams[key];
}

template<typename T>
void  AlgoStreamingService<T>::OnMessage(AlgoStream<T>& data) {
	algoStreams[data.GetPriceStream()->GetProduct().GetTicker()] = data;
}

template<typename T>
void AlgoStreamingService<T>::AddListener(ServiceListener<AlgoStream<T>> *listener) {
	listeners.push_back(listener);
}

template<typename T>
const vector<ServiceListener<AlgoStream<T>>*>& AlgoStreamingService<T>::GetListeners() const {
	return listeners;
}

template<typename T>
ServiceListener<Price<T>>* AlgoStreamingService<T>::GetListener() {
	return algostrlistener;
}

template<typename T>
void AlgoStreamingService<T>::PublishPrice(Price<T>& price) {
	double mid = price.GetMid();
	double Spread = price.GetBidOfferSpread();
	double bid = mid - Spread / 2.0;
	double offer = mid + Spread / 2.0;
	long visibleQuantity = (VisibleS1M + 1) * 10000000;
	long hiddenQuantity = visibleQuantity * 2;
	VisibleS1M = !VisibleS1M;

	PriceStreamOrder _bidOrder(bid, visibleQuantity, hiddenQuantity, BID);
	PriceStreamOrder _offerOrder(offer, visibleQuantity, hiddenQuantity, OFFER);
	AlgoStream<T> _algoStream(new PriceStream<T>(price.GetProduct(), _bidOrder, _offerOrder));
	OnMessage(_algoStream);
	for (auto l : listeners)
	{
		l->ProcessAdd(_algoStream);
	}

}
#endif
