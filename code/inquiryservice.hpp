/**
 * inquiryservice.hpp
 * Defines the data types and Service for customer inquiries.
 *
 * @author Breman Thuraisingham
 */
#ifndef INQUIRY_SERVICE_HPP
#define INQUIRY_SERVICE_HPP

#include "soa.hpp"
#include "tradebookingservice.hpp"
#include <unordered_map>
#include <fstream>
#include "tools.h"

// Various inqyury states
enum InquiryState { RECEIVED, QUOTED, DONE, REJECTED, CUSTOMER_REJECTED };

/**
 * Inquiry object modeling a customer inquiry from a client.
 * Type T is the product type.
 */
template<typename T>
class Inquiry
{

public:

  // ctor for an inquiry
	Inquiry() = default;
  Inquiry(string _inquiryId, const T &_product, Side _side, long _quantity, double _price, InquiryState _state);

  // Get the inquiry ID
  const string& GetInquiryId() const;

  // Get the product
  const T& GetProduct() const;

  // Get the side on the inquiry
  Side GetSide() const;

  // Get the quantity that the client is inquiring for
  long GetQuantity() const;

  // Get the price that we have responded back with
  double GetPrice() const;

  // Get the current state on the inquiry
  InquiryState GetState() const;

  void setState(InquiryState s);
  void setPrice(double p);

  //string to print
  string To_string();

private:
  string inquiryId;
  T product;
  Side side;
  long quantity;
  double price;
  InquiryState state;

};

template<typename T>
class IQConnector;
/**
 * Service for customer inquirry objects.
 * Keyed on inquiry identifier (NOTE: this is NOT a product identifier since each inquiry must be unique).
 * Type T is the product type.
 */
template<typename T>
class InquiryService : public Service<string,Inquiry <T> >
{
private:

	unordered_map<string, Inquiry<T>> inquiries;
	vector<ServiceListener<Inquiry<T>>*> listeners;
	IQConnector<T>* connector;

public:
	InquiryService();

	// Get data on our service given a key
	Inquiry<T>& GetData(string key);

	// The callback that a Connector should invoke for any new or updated data
	void OnMessage(Inquiry<T>& data);

	// Add a listener to the Service for callbacks on add, remove, and update events for data to the Service
	void AddListener(ServiceListener<Inquiry<T>>* listener);

	// Get all listeners on the Service
	const vector<ServiceListener<Inquiry<T>>*>& GetListeners() const;

  // Send a quote back to the client
  void SendQuote(const string &inquiryId, double price) ;

  // Reject an inquiry from the client
  void RejectInquiry(const string &inquiryId);

  IQConnector<T>* getConnector();


};

template<typename T>
class IQConnector : public Connector<Inquiry<T>>
{

private:

	InquiryService<T>* IQS;

public:

	// Connector and Destructor
	IQConnector(InquiryService<T>* service);

	// Publish data to the Connector
	void Publish(Inquiry<T>& data);

	// Re-subscribe data from the Connector
	void Consume(string file_name);

};


/*       implementation   */
template<typename T>
Inquiry<T>::Inquiry(string _inquiryId, const T &_product, Side _side, long _quantity, double _price, InquiryState _state) :
  product(_product)
{
  inquiryId = _inquiryId;
  side = _side;
  quantity = _quantity;
  price = _price;
  state = _state;
}

template<typename T>
const string& Inquiry<T>::GetInquiryId() const
{
  return inquiryId;
}

template<typename T>
const T& Inquiry<T>::GetProduct() const
{
  return product;
}

template<typename T>
Side Inquiry<T>::GetSide() const
{
  return side;
}

template<typename T>
long Inquiry<T>::GetQuantity() const
{
  return quantity;
}

template<typename T>
double Inquiry<T>::GetPrice() const
{
  return price;
}

template<typename T>
InquiryState Inquiry<T>::GetState() const
{
  return state;
}

template<typename T>
void Inquiry<T>::setState(InquiryState s)
{
	state = s;
}


template<typename T>
void Inquiry<T>::setPrice(double p)
{
	price = p;
}

template<typename T>
string Inquiry<T>::To_string()
{
	string _side;
	switch (side)
	{
	case BUY:
		_side = "BUY";
		break;
	case SELL:
		_side = "SELL";
		break;
	}
	string _state;
	switch (state)
	{
	case RECEIVED:
		_state = "RECEIVED";
		break;
	case QUOTED:
		_state = "QUOTED";
		break;
	case DONE:
		_state = "DONE";
		break;
	case REJECTED:
		_state = "REJECTED";
		break;
	case CUSTOMER_REJECTED:
		_state = "CUSTOMER_REJECTED";
		break;
	}
	return product.GetTicker() + ' ' + inquiryId + ' ' + _side + ' ' +
		PriceDTS(price) + ' ' + to_string(quantity) + ' ' + _state;
}

template<typename T>
InquiryService<T>::InquiryService()
	:connector(new IQConnector<T>(this)) {}


template<typename T>
Inquiry<T>& InquiryService<T>::GetData(string key)
{
	return inquiries[key];
}

template<typename T>
void InquiryService<T>::OnMessage(Inquiry<T>& data)
{
	InquiryState state = data.GetState();

	inquiries[data.GetInquiryId()] = data;
	switch (state)
	{
	case RECEIVED:
		SendQuote(data.GetInquiryId(), 100);
		break;
	case QUOTED:
		data.setState(DONE);

		for (auto l : listeners)
		{
			l->ProcessAdd(data);
		}

		break;
	}
}

template<typename T>
void InquiryService<T>::AddListener(ServiceListener<Inquiry<T>>* listener)
{
	listeners.push_back(listener);
}

template<typename T>
const vector<ServiceListener<Inquiry<T>>*>& InquiryService<T>::GetListeners() const
{
	return listeners;
}


template<typename T>
void InquiryService<T>::SendQuote(const string& inquiryId, double price)
{
	inquiries[inquiryId].setPrice(price);
	connector->Publish(inquiries[inquiryId]);
}

template<typename T>
void InquiryService<T>::RejectInquiry(const string& inquiryId)
{
	inquiries[inquiryId].setState(REJECTED);
}

template<typename T>
IQConnector<T>* InquiryService<T>::getConnector()
{
	return connector;
}

template<typename T>
IQConnector<T>::IQConnector(InquiryService<T>* service)
	:IQS(service) {}


template<typename T>
void IQConnector<T>::Publish(Inquiry<T>& data)
{
	data.setState(QUOTED);
	IQS->OnMessage(data);
}

template<typename T>
void IQConnector<T>::Consume(string file_name)
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
		Side s = BUY;
		if (blocks[2] == "Sell") s = SELL;

		InquiryState _state;
		if (blocks[5] == "RECEIVED") _state = RECEIVED;
		else if (blocks[5] == "QUOTED") _state = QUOTED;
		else if (blocks[5] == "DONE") _state = DONE;
		else if (blocks[5] == "REJECTED") _state = REJECTED;
		else if (blocks[5] == "CUSTOMER_REJECTED") _state = CUSTOMER_REJECTED;

		Inquiry<T> _inquiry(blocks[1], GetBond(blocks[0]), s,
			stol(blocks[4]), PriceSTD(blocks[3]), _state);
		IQS->OnMessage(_inquiry);
	}

}


#endif
