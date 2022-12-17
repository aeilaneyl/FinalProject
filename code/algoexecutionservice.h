#ifndef ALGOEXECUTIONSERVICEHPP
#define ALGOEXECUTIONSERVICEHPP

#include "soa.hpp"
#include "executionservice.hpp"
#include <unordered_map>
#include "tools.h"


/**
* An algo execution that process algo execution.
* Type T is the product type.
*/
template<typename T>
class AlgoExecution
{
private:
	ExecutionOrder<T>* executionOrder;

public:

	AlgoExecution() = default;
	AlgoExecution(const T& product, PricingSide side, string orderId, OrderType orderType, double price, long visibleQuantity, long hiddenQuantity, string parentOrderId, bool isChildOrder);

	ExecutionOrder<T>* GetExecutionOrder() const;


};

template<typename T>
class AlgoExecutionService;

template<typename T>
class AEListener : public ServiceListener<OrderBook<T>>
{

private:

	AlgoExecutionService<T>* AES;

public:

	AEListener(AlgoExecutionService<T>* service);

	// Listener callback to process an add event to the Service
	void ProcessAdd(OrderBook<T>& data);

	// Listener callback to process a remove event to the Service
	void ProcessRemove(OrderBook<T>& data);

	// Listener callback to process an update event to the Service
	void ProcessUpdate(OrderBook<T>& data);

};


/**
* AlgoExecutionService
* Keyed on product identifier.
* Type T is the product type.
*/
template<typename T>
class AlgoExecutionService : public Service<string, AlgoExecution<T>>
{

private:

	unordered_map<string, AlgoExecution<T>> algoExecutions;
	vector<ServiceListener<AlgoExecution<T>>*> listeners;
	AEListener<T>* listener;
	double aggresing_spread;
	bool isBid;

public:

	AlgoExecutionService();

	// Get data on our service given a key
	AlgoExecution<T>& GetData(string key);

	// The callback that a Connector should invoke for any new or updated data
	void OnMessage(AlgoExecution<T>& data);

	// Add a listener to the Service for callbacks on add, remove, and update events for data to the Service
	void AddListener(ServiceListener<AlgoExecution<T>>* listener);

	// Get all listeners on the Service
	const vector<ServiceListener<AlgoExecution<T>>*>& GetListeners() const;

	// Get the listener of the service
	AEListener<T>* GetListener();

	void AlgoExecuteOrder(OrderBook<T>& _orderBook);
};




/*     implementation      */
template<typename T>
AlgoExecution<T>::AlgoExecution(const T& product, PricingSide side, string orderId, OrderType orderType, double price, long visibleQuantity, long hiddenQuantity, string parentOrderId, bool isChildOrder)
	:executionOrder(new ExecutionOrder<T>(product, side, orderId, orderType, price, visibleQuantity, hiddenQuantity, parentOrderId, isChildOrder)) {}

template<typename T>
ExecutionOrder<T>* AlgoExecution<T>::GetExecutionOrder() const
{
	return executionOrder;
}


template<typename T>
AEListener<T>::AEListener(AlgoExecutionService<T>* service)
	:AES(service) {}


template<typename T>
void AEListener<T>::ProcessAdd(OrderBook<T>& _data)
{
	AES->AlgoExecuteOrder(_data);
}

template<typename T>
void AEListener<T>::ProcessRemove(OrderBook<T>& _data) {}

template<typename T>
void AEListener<T>::ProcessUpdate(OrderBook<T>& _data) {}



template<typename T>
AlgoExecutionService<T>::AlgoExecutionService()
	:listener(new AEListener<T>(this))
{
	aggresing_spread = 1.0/128;
	isBid = true;
}


template<typename T>
AlgoExecution<T>& AlgoExecutionService<T>::GetData(string key)
{
	return algoExecutions[key];
}

template<typename T>
void AlgoExecutionService<T>::OnMessage(AlgoExecution<T>& data){
}

template<typename T>
void AlgoExecutionService<T>::AddListener(ServiceListener<AlgoExecution<T>>* _listener)
{
	listeners.push_back(_listener);
}

template<typename T>
const vector<ServiceListener<AlgoExecution<T>>*>& AlgoExecutionService<T>::GetListeners() const
{
	return listeners;
}

template<typename T>
AEListener<T>* AlgoExecutionService<T>::GetListener()
{
	return listener;
}

template<typename T>
void AlgoExecutionService<T>::AlgoExecuteOrder(OrderBook<T>& data)
{
	T product = data.GetProduct();
	string ticker = product.GetTicker();

	BidOffer bidOffer = data.GetBestBidOffer();
	Order bidOrder = bidOffer.GetBidOrder();
	Order offerOrder = bidOffer.GetOfferOrder();

	if (offerOrder.GetPrice() - bidOrder.GetPrice() == aggresing_spread)
	{

		PricingSide side = OFFER; long Q = offerOrder.GetQuantity(); double p = offerOrder.GetPrice();
		if (isBid) {
			side = BID;
			Q = bidOrder.GetQuantity();
			p = bidOrder.GetPrice();
		}
		isBid = !isBid;

		AlgoExecution<T> algoExecution(product, side, genID(), MARKET,
			p, Q, 0, "", false);

		algoExecutions[ticker] = algoExecution;

		for (auto l : listeners)
		{
			l->ProcessAdd(algoExecution);
		}
	}
}

#endif
