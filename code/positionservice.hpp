/**
 * positionservice.hpp
 * Defines the data types and Service for positions.
 *
 * @author Breman Thuraisingham
 */
#ifndef POSITION_SERVICE_HPP
#define POSITION_SERVICE_HPP

#include <string>
#include <map>
#include "soa.hpp"
#include "tradebookingservice.hpp"
#include "tools.h"

using namespace std;

/**
 * Position class in a particular book.
 * Type T is the product type.
 */
template<typename T>
class Position
{

public:

  // ctor for a position
	Position() = default;
  Position(const T &_product);

  // Get the product
  const T& GetProduct() const;

  // Get the position quantity
  long GetPosition(string &book);

  // Get the aggregate position
  long GetAggregatePosition();

  void AddPosition(string &book, long pos);


  //string to print
  string To_string();

private:
  T product;
  map<string,long> positions;

};

template<typename T>
class PositionService;

template<typename T>
class PositionListener : public ServiceListener<Trade<T>>
{

private:

	PositionService<T>* PS;

public:

	PositionListener(PositionService<T>* _service);

	// Listener callback to process an add event to the Service
	void ProcessAdd(Trade<T>& data);

	// Listener callback to process a remove event to the Service
	void ProcessRemove(Trade<T>& data);

	// Listener callback to process an update event to the Service
	void ProcessUpdate(Trade<T>& data);

};
/**
 * Position Service to manage positions across multiple books and secruties.
 * Keyed on product identifier.
 * Type T is the product type.
 */
template<typename T>
class PositionService : public Service<string,Position <T> >
{
private:
	map<string, Position<T>> positions;
	vector<ServiceListener<Position<T>>*> listeners;
	PositionListener<T>* listener;
public:
	PositionService();

	// Get data on our service given a key
	Position<T>& GetData(string key);

	// The callback that a Connector should invoke for any new or updated data
	void OnMessage(Position<T>& data);

	// Add a listener to the Service for callbacks on add, remove, and update events for data to the Service
	void AddListener(ServiceListener<Position<T>>* listener);

	// Get all listeners on the Service
	const vector<ServiceListener<Position<T>>*>& GetListeners() const;

	// Get the listener of the service
	PositionListener<T>* GetListener();

  // Add a trade to the service
   void AddTrade(const Trade<T> &trade);

};






/*     implementation   */
template<typename T>
Position<T>::Position(const T &_product) :
  product(_product)
{
}

template<typename T>
const T& Position<T>::GetProduct() const
{
  return product;
}

template<typename T>
long Position<T>::GetPosition(string &book)
{
  return positions[book];
}

template<typename T>
long Position<T>::GetAggregatePosition()
{
	long _sum = 0;
	for (auto p : positions)
	{
		_sum += p.second;
	}
	return _sum;
}

template<typename T>
void Position<T>::AddPosition(string &book, long pos)
{
	positions[book] += pos;
}

template<typename T>
string Position<T>::To_string()
{
	string res = product.GetTicker() + ", ";
	long _sum = 0;
	for (auto p : positions)
	{
		_sum += p.second;
		res += p.first + ": " + to_string(p.second) + ", ";
	}

	res += "Total : " + to_string(_sum);
	return res;
}


template<typename T>
PositionListener<T>::PositionListener(PositionService<T>* service)
	:PS(service) {}


template<typename T>
void PositionListener<T>::ProcessAdd(Trade<T>& data)
{
	PS->AddTrade(data);
}

template<typename T>
void PositionListener<T>::ProcessRemove(Trade<T>& data) {}

template<typename T>
void PositionListener<T>::ProcessUpdate(Trade<T>& data) {}




template<typename T>
PositionService<T>::PositionService()
	:listener(new PositionListener<T>(this)) {}


template<typename T>
Position<T>& PositionService<T>::GetData(string key)
{
	return positions[key];
}

template<typename T>
void PositionService<T>::OnMessage(Position<T>& _data)
{
}

template<typename T>
void PositionService<T>::AddListener(ServiceListener<Position<T>>* _listener)
{
	listeners.push_back(_listener);
}

template<typename T>
PositionListener<T>* PositionService<T>::GetListener()
{
	return listener;
}

template<typename T>
const vector<ServiceListener<Position<T>>*>& PositionService<T>::GetListeners() const
{
	return listeners;
}

template<typename T>
void PositionService<T>::AddTrade(const Trade<T>& trade)
{
	string ticker = trade.GetProduct().GetTicker();
	string book = trade.GetBook();
	Side side = trade.GetSide();

	if (positions.find(ticker) == positions.end()) {
		positions[ticker] = Position<T>(GetBond(ticker));
	}

	switch (side)
	{
	case BUY:
		positions[ticker].AddPosition(book, trade.GetQuantity());
		break;
	case SELL:
		positions[ticker].AddPosition(book, -trade.GetQuantity());
		break;
	}


	for (auto l : listeners)
	{
		l->ProcessAdd(positions[ticker]);
	}
}

#endif
