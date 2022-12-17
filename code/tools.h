#ifndef TOOLS_HPP
#define TOOLS_HPP
#include "products.hpp"
#include <chrono>
#include <string>
#include <iomanip>
#include <sstream>
#include <vector>
#include <random>

#pragma warning(disable : 4996)
//get current time in millisecond precision.
string getCurrentTimestamp()
{
    auto now = std::chrono::system_clock::now();
    auto millis = (chrono::duration_cast<chrono::milliseconds> (now.time_since_epoch())).count() % 1000;
    
    std::time_t tt = std::chrono::system_clock::to_time_t(now);
    std::tm tm = *std::localtime(&tt);
    std::stringstream ss;
    ss << std::put_time( &tm, "%Y-%m-%d %H:%M:%S") <<':'<<std::setw(3) << std::setfill('0') << millis;
        
    return ss.str();
}

string genID(const int len = 8) {
	static const char alphanum[] =
		"0123456789"
		"ABCDEFGHIJKLMNOPQRSTUVWXYZ"
		"abcdefghijklmnopqrstuvwxyz";
	static random_device dev;
	static mt19937 rng(dev());
	static uniform_int_distribution<mt19937::result_type> dist6(0, 1000); 


	string tmp_s;
	tmp_s.reserve(len);



	for (int i = 0; i < len; ++i) {
		tmp_s += alphanum[dist6(rng) % (sizeof(alphanum) - 1)];
	}

	return tmp_s;
}
// Convert fractional price to numerical price.
double PriceSTD(string stringPrice)
{
	int sep = stringPrice.find("-");
	string Price100 = stringPrice.substr(0,sep);
	string Price32 = stringPrice.substr(sep+1,2);
	string Price8 = stringPrice.substr(sep+3,1);
	if (Price8 == "+") Price8 = "4";
	return stod(Price100) + stod(Price32) / 32.0 + stod(Price8) / 256.0;

}

// Convert numerical price to fractional price.
string PriceDTS(double doublePrice)
{
	int Price100 = floor(doublePrice);
	int Price256 = floor((doublePrice - Price100) * 256);
	int Price32 = Price256 / 8;
	int Price8 = Price256 % 8;

	string stringPrice100 = to_string(Price100);
	string stringPrice32 = to_string(Price32);
	string stringPrice8 = to_string(Price8);

	if (Price32 < 10) stringPrice32 = "0" + stringPrice32;
	if (Price8 == 4) stringPrice8 = "+";

	return stringPrice100 + "-" + stringPrice32 + stringPrice8;

}

Bond GetBond(string ticker)
{
	if (ticker == "T2Y") return Bond("91282CFX4", CUSIP, "T2Y", 0.045, "11/30/2024");
	if (ticker == "T3Y") return Bond("91282CFW6", CUSIP, "T3Y", 0.045, "11/15/2025");
	if (ticker == "T5Y") return Bond("91282CFZ9", CUSIP, "T5Y", 0.03875, "11/30/2027");
	if (ticker == "T7Y") return Bond("91282CFY2", CUSIP, "T7Y", 0.03875, "11/30/2029");
	if (ticker == "T10Y") return Bond("91282CFV8", CUSIP, "T10Y", 0.04125, "11/15/2032");
	if (ticker == "T20Y") return Bond("912810TM0", CUSIP, "T20Y", 0.04, "11/15/2042");
	if (ticker == "T30Y") return Bond("912810TL2", CUSIP, "T30Y", 0.04, "11/15/2052");
    return Bond();
}


vector<string> GetBucket(string ticker) {
	static vector<string> FrontEnd {"FrontEnd","T2Y","T3Y" };
	static vector<string> Belly {"Belly","T5Y","T7Y","T10Y" };
	static vector<string> LongEnd {"LongEnd","T20Y","T30Y" };
	if (ticker == "T2Y") return FrontEnd;
	if (ticker == "T3Y") return FrontEnd;
	if (ticker == "T5Y") return Belly;
	if (ticker == "T7Y") return Belly;
	if (ticker == "T10Y") return Belly;
	if (ticker == "T20Y") return LongEnd;
	if (ticker == "T30Y") return LongEnd;
    return Belly;
}

double GetPV01(string ticker) {
	if (ticker == "T2Y") return 0.01879;
	if (ticker == "T3Y") return 0.02761;
	if (ticker == "T5Y") return 0.04526;
	if (ticker == "T7Y") return 0.06170;
	if (ticker == "T10Y") return 0.08598;
	if (ticker == "T20Y") return 0.14420;
	if (ticker == "T30Y") return 0.19917;
    return 1;
}

#endif
