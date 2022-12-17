/**
 * historicaldataservice.hpp
 * historicaldataservice.hpp
 *
 * @author Breman Thuraisingham
 * Defines the data types and Service for historical data.
 *
 * @author Breman Thuraisingham
 */
#ifndef HISTORICAL_DATA_SERVICE_HPP
#define HISTORICAL_DATA_SERVICE_HPP
#include "soa.hpp"
#include <unordered_map>
#include <string>
#include <fstream>
#include "tools.h"

template<typename T>
class HistoricalDataService;
 /**
 * Historical Data Service Listener subscribing data to Historical Data.
 * 
 */
template<typename T>
class HistoricalDataListener : public ServiceListener<T>
{

private:

	HistoricalDataService<T>* HS;

public:

	HistoricalDataListener(HistoricalDataService<T>* _service);

	// Listener callback to process an add event to the Service
	void ProcessAdd(T& data);

	// Listener callback to process a remove event to the Service
	void ProcessRemove(T& data);

	// Listener callback to process an update event to the Service
	void ProcessUpdate(T& data);


};

template<typename T>
class HistoricalDataConnector;
/**
 * Service for processing and persisting historical data to a persistent store.
 * Keyed on some persistent key.
 * Type T is the data type to persist.
 */
template<typename T>
class HistoricalDataService : Service<string,T>
{
private:
	unordered_map<string, T> Datas;
	vector<ServiceListener<T>*> listeners;
	HistoricalDataConnector<T>* connector;
	ServiceListener<T>* listener;
	string file_name;
public:

	HistoricalDataService(string file_name);

	// Get data on our service given a key
	T& GetData(string key);

	// The callback that a Connector should invoke for any new or updated data
	void OnMessage(T& data);

	// Add a listener to the Service for callbacks on add, remove, and update events for data to the Service
	void AddListener(ServiceListener<T>* listener);

	// Get all listeners on the Service
	const vector<ServiceListener<T>*>& GetListeners() const;

	// Get the connector of the service
	HistoricalDataConnector<T>* GetConnector();

	// Get the listener of the service
	ServiceListener<T>* GetListener();

	// Persist data to a store
	void PersistData(string persistKey, T& data);

	string GetFileName();
};

/**
* Historical Data Connector 
* Type T is the data type to persist.
*/
template<typename T>
class HistoricalDataConnector : public Connector<T>
{

private:

	HistoricalDataService<T>* HS;

public:

	HistoricalDataConnector(HistoricalDataService<T>* _service);

	// Publish data to the Connector
	void Publish(T& _data);


};





/*    implementation       */


template<typename T>
HistoricalDataListener<T>::HistoricalDataListener(HistoricalDataService<T>* _service)
	:HS(_service) {}



template<typename T>
void HistoricalDataListener<T>::ProcessAdd(T & data)
{
	HS->PersistData(data.GetProduct().GetTicker(), data);
}

template<typename T>
void HistoricalDataListener<T>::ProcessRemove(T& data) {}

template<typename T>
void HistoricalDataListener<T>::ProcessUpdate(T& data) {}


template<typename T>
HistoricalDataService<T>::HistoricalDataService(string _file)
	:listener(new HistoricalDataListener<T>(this)),
	connector(new HistoricalDataConnector<T>(this)),
	file_name(_file) {}



template<typename T>
T& HistoricalDataService<T>::GetData(string key)
{
	return Datas[key];
}

template<typename T>
void HistoricalDataService<T>::OnMessage(T& data)
{
	Datas[data.GetProduct().GetTicker()] = data;
}

template<typename T>
void HistoricalDataService<T>::AddListener(ServiceListener<T>* _listener)
{
	listeners.push_back(_listener);
}

template<typename T>
const vector<ServiceListener<T>*>& HistoricalDataService<T>::GetListeners() const
{
	return listeners;
}



template<typename T>
HistoricalDataConnector<T>* HistoricalDataService<T>::GetConnector()
{
	return connector;
}

template<typename T>
ServiceListener<T>* HistoricalDataService<T>::GetListener()
{
	return listener;
}



template<typename T>
void HistoricalDataService<T>::PersistData(string _persistKey, T& data)
{
	connector->Publish(data);
}


template<typename T>
string HistoricalDataService<T>::GetFileName()
{
	return file_name;
}


template<typename T>
HistoricalDataConnector<T>::HistoricalDataConnector(HistoricalDataService<T>* _service)
	:HS(_service) {}

template<typename T>
void HistoricalDataConnector<T>::Publish(T& data)
{
	std::ofstream outfile;
	// append instead of overwrite
	outfile.open(HS->GetFileName(), ios_base::app);

	outfile << getCurrentTimestamp()<<", ";
	outfile << data.To_string() << endl;
	outfile.close();
}


#endif
