#ifndef GUISERVICEHPP
#define GUISERVICEHPP

#include "soa.hpp"
#include "pricingservice.hpp"
#include <unordered_map>
#include "tools.h"
#include <fstream>
#include <chrono>
using namespace std::chrono;
template<typename T>
class GUIService;

/**
* guilistener
* Keyed on product identifier.
* Type T is the product type.
*/
template<typename T>
class GUIListener : public ServiceListener<Price<T>>
{

private:

	GUIService<T>* GUIS;
	long long time;


public:

	GUIListener(GUIService<T>* _service);

	// Listener callback to process an add event to the Service
	void ProcessAdd(Price<T>& data);

	// Listener callback to process a remove event to the Service
	void ProcessRemove(Price<T>& data);

	// Listener callback to process an update event to the Service
	void ProcessUpdate(Price<T>& data);


};


template<typename T>
class GUIConnector;

template<typename T>
class GUIService : Service<string, Price<T>> {
private:

	unordered_map<string, Price<T>> prices;
	vector<ServiceListener<Price<T>>*> listeners;
	GUIConnector<T>* connector;
	ServiceListener<Price<T>>* listener;
	int throttle;

public:

	// Constructor and destructor
	GUIService(int _throttle);

	// Get data on our service given a key
	Price<T>& GetData(string key);

	// The callback that a Connector should invoke for any new or updated data
	void OnMessage(Price<T>& data);

	// Add a listener to the Service for callbacks on add, remove, and update events for data to the Service
	void AddListener(ServiceListener<Price<T>>* listener);

	// Get all listeners on the Service
	const vector<ServiceListener<Price<T>>*>& GetListeners() const;

	// Get the connector of the service
	GUIConnector<T>* GetConnector();

	// Get the listener of the service
	ServiceListener<Price<T>>* GetListener();

	// Get the throttle of the service
	int GetThrottle() const;



};

/**
* GUI Connector 
* Type T is the product type.
*/
template<typename T>
class GUIConnector : public Connector<Price<T>>
{

private:

	GUIService<T>* GUIS;
	//we only print first 100 updates
	int cnt;

public:

	GUIConnector(GUIService<T>* _service);

	// Publish data to the Connector
	void Publish(Price<T>& _data);
};




/*    implementation     */
template<typename T>
GUIListener<T>::GUIListener(GUIService<T>* _service)
	:GUIS(_service), time(0){}

template<typename T>
void GUIListener<T>::ProcessAdd(Price<T>& data)
{
	int _throttle = GUIS->GetThrottle();
	long long now = duration_cast< milliseconds >(system_clock::now().time_since_epoch()
		).count();
	if (now - time >= _throttle)
	{
		GUIS->OnMessage(data);
		time = now;
	}
}

template<typename T>
void GUIListener<T>::ProcessRemove(Price<T>& data) {}

template<typename T>
void GUIListener<T>::ProcessUpdate(Price<T>& data) {}




template<typename T>
GUIService<T>::GUIService(int _throttle)
	:throttle(_throttle), listener(new GUIListener<T>(this)), 
	connector(new GUIConnector<T>(this)){}


template<typename T>
Price<T>& GUIService<T>::GetData(string key)
{
	return prices[key];
}

template<typename T>
void GUIService<T>::OnMessage(Price<T>& data)
{
	prices[data.GetProduct().GetTicker()] = data;
	connector->Publish(data);
}

template<typename T>
void GUIService<T>::AddListener(ServiceListener<Price<T>>* listener)
{
	listeners.push_back(listener);
}

template<typename T>
const vector<ServiceListener<Price<T>>*>& GUIService<T>::GetListeners() const
{
	return listeners;
}

template<typename T>
GUIConnector<T>* GUIService<T>::GetConnector()
{
	return connector;
}

template<typename T>
ServiceListener<Price<T>>* GUIService<T>::GetListener()
{
	return listener;
}

template<typename T>
int GUIService<T>::GetThrottle() const
{
	return throttle;
}


template<typename T>
GUIConnector<T>::GUIConnector(GUIService<T>* _service)
	:GUIS(_service), cnt(0) {}


template<typename T>
void GUIConnector<T>::Publish(Price<T>& data)
{
	//only print first 100 updates
	if (cnt >= 100) return;

	ofstream file;
	file.open("gui.txt", ios_base::app);

	file << getCurrentTimestamp() << "," << data.To_string()<<endl;
	file.close();
	cnt++;
}

#endif
