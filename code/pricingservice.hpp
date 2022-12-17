/**
 * pricingservice.hpp
 * Defines the data types and Service for internal prices.
 *
 * @author Breman Thuraisingham
 */
#ifndef PRICING_SERVICE_HPP
#define PRICING_SERVICE_HPP

#include <vector>
#include <fstream>
#include <iostream>
#include "tools.h"
#include "soa.hpp"
#include <unordered_map>
#include <sstream>

/**
 * A price object consisting of mid and bid/offer spread.
 * Type T is the product type.
 */
template<typename T>
class Price
{

public:

  // ctor for a price
  Price() = default;
  Price(const T &_product, double _mid, double _bidOfferSpread);

  // Get the product
  const T& GetProduct() const;

  // Get the mid price
  double GetMid() const;

  // Get the bid/offer spread around the mid
  double GetBidOfferSpread() const;

  //string to print
  string To_string();

private:
  T product;
  double mid;
  double bidOfferSpread;

};

/**
 * Pricing Service managing mid prices and bid/offers.
 * Keyed on product identifier.
 * Type T is the product type.
 */
template<typename T>
class PricingService : public Service<string,Price <T> >
{
private:
	vector<ServiceListener<Price<T>>*> listeners;
	unordered_map<string, Price<T>> prices;

public:
	PricingService();
	Price <T>& GetData(string key);
	// The callback that a Connector should invoke for any new or updated data
	void OnMessage(Price <T> & p);

	// Add a listener to the Service for callbacks on add, remove, and update events
	// for data to the Service.
	void AddListener(ServiceListener<Price <T>> *listener);

	const vector< ServiceListener<Price <T>>* >& GetListeners() const;
};

template<typename T> // with respect to ps, not template wrt connector itself
class PSConnector : public Connector<Price<T>> {
private:
	PricingService<T>* ps;
public:
	PSConnector(PricingService<T>* _ps);
	void Consume(std::string file_name);
	void Publish(Price<T> &data);
};




/*   implementation     */



template<typename T>
Price<T>::Price(const T &_product, double _mid, double _bidOfferSpread) :
  product(_product)
{
  mid = _mid;
  bidOfferSpread = _bidOfferSpread;
}

template<typename T>
const T& Price<T>::GetProduct() const
{
  return product;
}

template<typename T>
double Price<T>::GetMid() const
{
  return mid;
}

template<typename T>
double Price<T>::GetBidOfferSpread() const
{
  return bidOfferSpread;
}


template<typename T>
string Price<T>::To_string()
{
	return product.GetTicker() + ": " + "mid price " + PriceDTS(mid) +
		", spread " + to_string(bidOfferSpread);
}
template<typename T>
PricingService<T>::PricingService() {
}

template<typename T>
Price <T>& PricingService<T>::GetData(string key) {
	return prices[key];
}

template<typename T>
void PricingService<T>::OnMessage(Price <T> & p) {
	//prices.insert(unordered_map<string, Price<T>>::value_type(p.GetProduct().GetTicker(), p));
	prices[p.GetProduct().GetTicker()] = p;
	//cout << p.GetBidOfferSpread() <<','<< p.GetMid() << endl;
	for (auto l : listeners) {
		l->ProcessAdd(p);
	}
}

template<typename T>
void PricingService<T>::AddListener(ServiceListener<Price <T>> *listener) {
	listeners.push_back(listener);
}

template<typename T>
const vector< ServiceListener<Price <T>>* >& PricingService<T>::GetListeners() const {
	return listeners;
}

template<typename T>
PSConnector<T>::PSConnector(PricingService<T>* _ps): ps(_ps){}

template<typename T>
void PSConnector<T>::Consume(std::string file_name) {
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

		double _bidPrice = PriceSTD(blocks[1]);
		double _offerPrice = PriceSTD(blocks[2]);
		double _midPrice = (_bidPrice + _offerPrice) / 2.0;
		double _spread = _offerPrice - _bidPrice;
		Price<T> _price(GetBond(blocks[0]), _midPrice, _spread);
		ps->OnMessage(_price);
	}
}

template<typename T>
void PSConnector<T>::Publish(Price<T> &data) {}
#endif
