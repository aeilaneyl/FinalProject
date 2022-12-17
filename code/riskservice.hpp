/**
 * riskservice.hpp
 * Defines the data types and Service for fixed income risk.
 *
 * @author Breman Thuraisingham
 */
#ifndef RISK_SERVICE_HPP
#define RISK_SERVICE_HPP

#include "soa.hpp"
#include "positionservice.hpp"
#include <unordered_map>
#include "tools.h"

/**
 * PV01 risk.
 * Type T is the product type.
 */
template<typename T>
class PV01
{

public:

  // ctor for a PV01 value
	PV01() = default;
  PV01(const T &_product, double _pv01, long _quantity);

  // Get the product on this PV01 value
  const T& GetProduct() const;

  // Get the PV01 value
  double GetPV01() const;

  // Get the quantity that this risk value is associated with
  long GetQuantity() const;

  double GetBucketPV01() const;

  string GetBucketName() const;

  void SetBucketPV01(double v);

  void SetBucketName(string name);

  //string to print
  string To_string();

private:
  T product;
  double pv01;
  long quantity;

  double bucket_pv01;
  string bucket_name;

};

/**
 * A bucket sector to bucket a group of securities.
 * We can then aggregate bucketed risk to this bucket.
 * Type T is the product type.
 */
template<typename T>
class BucketedSector
{

public:

  // ctor for a bucket sector
  BucketedSector(const vector<T> &_products, string _name);

  // Get the products associated with this bucket
  const vector<T>& GetProducts() const;

  // Get the name of the bucket
  const string& GetName() const;

private:
  vector<T> products;
  string name;

};

template<typename T>
class RiskService;

template<typename T>
class RiskListener : public ServiceListener<Position<T>>
{

private:

	RiskService<T>* RS;

public:

	RiskListener(RiskService<T>* service);

	// Listener callback to process an add event to the Service
	void ProcessAdd(Position<T>& data);

	// Listener callback to process a remove event to the Service
	void ProcessRemove(Position<T>& data);

	// Listener callback to process an update event to the Service
	void ProcessUpdate(Position<T>& data);

};



/**
 * Risk Service to vend out risk for a particular security and across a risk bucketed sector.
 * Keyed on product identifier.
 * Type T is the product type.
 */
template<typename T>
class RiskService : public Service<string,PV01 <T> >
{
private:

	unordered_map<string, double> pv01s;
	vector<ServiceListener<PV01<T>>*> listeners;
	RiskListener<T>* listener;
public:
	RiskService();

	// Get data on our service given a key
	PV01<T>& GetData(string key);

	// The callback that a Connector should invoke for any new or updated data
	void OnMessage(PV01<T>& data);

	// Add a listener to the Service for callbacks on add, remove, and update events for data to the Service
	void AddListener(ServiceListener<PV01<T>>* listener);

	// Get all listeners on the Service
	const vector<ServiceListener<PV01<T>>*>& GetListeners() const;

	// Get the listener of the service
	RiskListener<T>* GetListener();
  // Add a position that the service will risk
	void AddPosition(Position<T> &position);

  // Get the bucketed risk for the bucket sector
  const PV01< BucketedSector<T> >& GetBucketedRisk(const BucketedSector<T> &sector) const;

};



/*     implementation    */
template<typename T>
PV01<T>::PV01(const T &_product, double _pv01, long _quantity) 
	:product(_product) {
  pv01 = _pv01;
  quantity = _quantity;
}

template<typename T>
const T& PV01<T>::GetProduct() const
{
	return product;
}

template<typename T>
double PV01<T>::GetPV01() const
{
	return pv01;
}

template<typename T>
long PV01<T>::GetQuantity() const
{
	return quantity;
}

template<typename T>
double PV01<T>::GetBucketPV01() const
{
	return bucket_pv01;
}

template<typename T>
void PV01<T>::SetBucketPV01(double v)
{
	bucket_pv01 = v;
}

template<typename T>
string PV01<T>::GetBucketName() const
{
	return bucket_name;
}

template<typename T>
void PV01<T>::SetBucketName(string n)
{
	bucket_name = n;
}

template<typename T>
string PV01<T>::To_string()
{
	return product.GetTicker() + ", risk: " + to_string(pv01) + ", Quantity: "
		+ to_string(quantity) + ", Bucket " + bucket_name + " risk: " +
		to_string(bucket_pv01);
}

template<typename T>
BucketedSector<T>::BucketedSector(const vector<T>& _products, string _name) :
  products(_products)
{
  name = _name;
}

template<typename T>
const vector<T>& BucketedSector<T>::GetProducts() const
{
  return products;
}

template<typename T>
const string& BucketedSector<T>::GetName() const
{
  return name;
}


template<typename T>
RiskListener<T>::RiskListener(RiskService<T>* service)
	:RS(service) {}

template<typename T>
void RiskListener<T>::ProcessAdd(Position<T>& data)
{
	RS->AddPosition(data);
}

template<typename T>
void RiskListener<T>::ProcessRemove(Position<T>& data) {}

template<typename T>
void RiskListener<T>::ProcessUpdate(Position<T>& data) {}


template<typename T>
RiskService<T>::RiskService()
	:listener(new RiskListener<T>(this)) {}

template<typename T>
PV01<T>& RiskService<T>::GetData(string key) {
    PV01<T>* tmp = new PV01<T>(GetBond(key), pv01s[key], 1);
    return *tmp;
}

template<typename T>
void RiskService<T>::OnMessage(PV01<T>& _data) {}

template<typename T>
void RiskService<T>::AddListener(ServiceListener<PV01<T>>* listener)
{
	listeners.push_back(listener);
}

template<typename T>
const vector<ServiceListener<PV01<T>>*>& RiskService<T>::GetListeners() const
{
	return listeners;
}

template<typename T>
RiskListener<T>* RiskService<T>::GetListener()
{
	return listener;
}

template<typename T>
void RiskService<T>::AddPosition(Position<T>& position)
{
	string ticker = position.GetProduct().GetTicker();
	long quantity = position.GetAggregatePosition();
	pv01s[ticker] = GetPV01(ticker) * quantity;

	vector<string> bucket = GetBucket(ticker);
	double bucket_pv01 = 0;
	for (int i = 1; i < bucket.size(); i++) {
		bucket_pv01 += pv01s[bucket[i]];
	}

	PV01<T> pv01(position.GetProduct(), pv01s[ticker], quantity);
	pv01.SetBucketName(bucket[0]);
	pv01.SetBucketPV01(bucket_pv01);

	for (auto l : listeners)
	{
		l->ProcessAdd(pv01);
	}
}

template<typename T>
const PV01<BucketedSector<T>>& RiskService<T>::GetBucketedRisk(const BucketedSector<T>& sector) const
{
	double pv01 = 0;

	for (auto& p : sector.GetProducts())
	{
		pv01 += pv01s[p.GetTicker()];
	}
	return PV01<BucketedSector<T>>(sector, pv01, 1);
}



#endif
