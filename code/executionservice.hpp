/**
 * executionservice.hpp
 * Defines the data types and Service for executions.
 *
 * @author Breman Thuraisingham
 */
#ifndef EXECUTION_SERVICE_HPP
#define EXECUTION_SERVICE_HPP

#include <string>
#include "soa.hpp"
#include "marketdataservice.hpp"
#include "algoexecutionservice.h"
#include <unordered_map>
#include "tools.h"

enum OrderType { FOK, IOC, MARKET, LIMIT, STOP };

enum Market { BROKERTEC, ESPEED, CME };

/**
 * An execution order that can be placed on an exchange.
 * Type T is the product type.
 */
template<typename T>
class ExecutionOrder
{

public:

  // ctor for an order
	ExecutionOrder() = default;
  ExecutionOrder(const T &_product, PricingSide _side, string _orderId, OrderType _orderType, double _price, double _visibleQuantity, double _hiddenQuantity, string _parentOrderId, bool _isChildOrder);

  // Get the product
  const T& GetProduct() const;

  // Get the order ID
  const string& GetOrderId() const;

  // Get the order type on this order
  OrderType GetOrderType() const;

  // Get the price on this order
  double GetPrice() const;

  // Get the visible quantity on this order
  long GetVisibleQuantity() const;

  // Get the hidden quantity
  long GetHiddenQuantity() const;

  // Get the parent order ID
  const string& GetParentOrderId() const;

  // Is child order?
  bool IsChildOrder() const;

  void setMarket(Market m);

  PricingSide getPricingSide();

  //string to print
  string To_string();
private:
  T product;
  PricingSide side;
  string orderId;
  OrderType orderType;
  double price;
  double visibleQuantity;
  double hiddenQuantity;
  string parentOrderId;
  bool isChildOrder;
  Market market;

};




template<typename T>
class AlgoExecution;

template<typename T>
class ExecutionService;
/**
* Execution Service Listener s
* Type T is the product type.
*/
template<typename T>
class EListener : public ServiceListener<AlgoExecution<T>>
{

private:

	ExecutionService<T>* ES;

public:

	// Connector and Destructor
	EListener(ExecutionService<T>* service);

	// Listener callback to process an add event to the Service
	void ProcessAdd(AlgoExecution<T>& data);

	// Listener callback to process a remove event to the Service
	void ProcessRemove(AlgoExecution<T>& data);

	// Listener callback to process an update event to the Service
	void ProcessUpdate(AlgoExecution<T>& data);

};


/**
* Service for executing orders on an exchange.
* Keyed on product identifier.
* Type T is the product type.
*/
template<typename T>
class ExecutionService : public Service<string, ExecutionOrder <T> >
{

private:

	unordered_map<string, ExecutionOrder<T>> executionOrders;
	vector<ServiceListener<ExecutionOrder<T>>*> listeners;
	EListener<T>* listener;

public:

	// Constructor and destructor
	ExecutionService();
	// Get data on our service given a key
	ExecutionOrder<T>& GetData(string key);

	// The callback that a Connector should invoke for any new or updated data
	void OnMessage(ExecutionOrder<T>& data);

	// Add a listener to the Service for callbacks on add, remove, and update events for data to the Service
	void AddListener(ServiceListener<ExecutionOrder<T>>* listener);

	// Get all listeners on the Service
	const vector<ServiceListener<ExecutionOrder<T>>*>& GetListeners() const;

	// Get the listener of the service
	EListener<T>* GetListener();

	void ExecuteOrder(ExecutionOrder<T>& order, Market market);

};


/*    implementation      */

template<typename T>
ExecutionOrder<T>::ExecutionOrder(const T &_product, PricingSide _side, string _orderId, OrderType _orderType, double _price, double _visibleQuantity, double _hiddenQuantity, string _parentOrderId, bool _isChildOrder) :
	product(_product)
{
	side = _side;
	orderId = _orderId;
	orderType = _orderType;
	price = _price;
	visibleQuantity = _visibleQuantity;
	hiddenQuantity = _hiddenQuantity;
	parentOrderId = _parentOrderId;
	isChildOrder = _isChildOrder;
}

template<typename T>
const T& ExecutionOrder<T>::GetProduct() const
{
	return product;
}

template<typename T>
const string& ExecutionOrder<T>::GetOrderId() const
{
	return orderId;
}

template<typename T>
OrderType ExecutionOrder<T>::GetOrderType() const
{
	return orderType;
}

template<typename T>
double ExecutionOrder<T>::GetPrice() const
{
	return price;
}

template<typename T>
long ExecutionOrder<T>::GetVisibleQuantity() const
{
	return visibleQuantity;
}

template<typename T>
long ExecutionOrder<T>::GetHiddenQuantity() const
{
	return hiddenQuantity;
}

template<typename T>
const string& ExecutionOrder<T>::GetParentOrderId() const
{
	return parentOrderId;
}

template<typename T>
bool ExecutionOrder<T>::IsChildOrder() const
{
	return isChildOrder;
}


template<typename T>
void ExecutionOrder<T>::setMarket(Market m)
{
	market = m;
}


template<typename T>
PricingSide ExecutionOrder<T>::getPricingSide()
{
	return side;
}

template<typename T>
string ExecutionOrder<T>::To_string()
{
	string _side;
	switch (side)
	{
	case BID:
		_side = "BID";
		break;
	case OFFER:
		_side = "OFFER";
		break;
	}

	string _orderType;
	switch (orderType)
	{
	case FOK:
		_orderType = "FOK";
		break;
	case IOC:
		_orderType = "IOC";
		break;
	case MARKET:
		_orderType = "MARKET";
		break;
	case LIMIT:
		_orderType = "LIMIT";
		break;
	case STOP:
		_orderType = "STOP";
		break;
	}
	string _market;
	switch (market)
	{
	case BROKERTEC:
		_market = "BROKERTEC";
		break;
	case ESPEED:
		_market = "ESPEED";
		break;
	case CME:
		_market = "CME";
		break;
	}
	string _IsChildOrder = "IsChildOrder";
	if (!isChildOrder) _IsChildOrder = "NotChildOrder";
	return product.GetTicker() + ' ' + orderId + ' ' + _market + ' ' + 
		_side + ' ' + _orderType
		+ ' ' + PriceDTS(price) + ' ' + to_string(visibleQuantity)
		+ ' ' + to_string(hiddenQuantity) + ' '
		+ parentOrderId + ' ' + _IsChildOrder;

}

template<typename T>
EListener<T>::EListener(ExecutionService<T>* service)
	:ES(service) {}


template<typename T>
void EListener<T>::ProcessAdd(AlgoExecution<T>& data)
{
	ExecutionOrder<T>* executionOrder = data.GetExecutionOrder();
	ES->ExecuteOrder(*executionOrder, CME);
}

template<typename T>
void EListener<T>::ProcessRemove(AlgoExecution<T>& data) {}

template<typename T>
void EListener<T>::ProcessUpdate(AlgoExecution<T>& data) {}


template<typename T>
ExecutionService<T>::ExecutionService()
	:listener(new EListener<T>(this)) {}


template<typename T>
ExecutionOrder<T>& ExecutionService<T>::GetData(string key)
{
	return executionOrders[key];
}

template<typename T>
void ExecutionService<T>::OnMessage(ExecutionOrder<T>& data)
{
	executionOrders[data.GetProduct().GetTicker()] = data;
}

template<typename T>
void ExecutionService<T>::AddListener(ServiceListener<ExecutionOrder<T>>* listener)
{
	listeners.push_back(listener);
}

template<typename T>
const vector<ServiceListener<ExecutionOrder<T>>*>& ExecutionService<T>::GetListeners() const
{
	return listeners;
}

template<typename T>
EListener<T>* ExecutionService<T>::GetListener()
{
	return listener;
}

template<typename T>
void ExecutionService<T>::ExecuteOrder(ExecutionOrder<T>& order, Market market)
{
	order.setMarket(market);
	OnMessage(order);

	for (auto l : listeners)
	{
		l->ProcessAdd(order);
	}
}

#endif
