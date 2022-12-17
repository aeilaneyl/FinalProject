


#include <iostream>

#include "products.hpp"
#include "pricingservice.hpp"
#include "algostreamingservice.h"
#include "streamingservice.hpp"
#include "historicaldataservice.hpp"
#include "guiservice.h"
#include "tradebookingservice.hpp"
#include "positionservice.hpp"
#include "riskservice.hpp"
#include "marketdataservice.hpp"
#include "algoexecutionservice.h"
#include "executionservice.hpp"
#include "inquiryservice.hpp"



int main() {
    
	//prices.txt
	
	PricingService<Bond> PS;
	AlgoStreamingService<Bond> ASS;
	StreamingService<Bond> SS;
	HistoricalDataService<PriceStream<Bond>> HDSPS("streaming.txt");
	GUIService<Bond> GUIS(300);
	
	PS.AddListener(ASS.GetListener());
	ASS.AddListener(SS.GetListener());
	SS.AddListener(HDSPS.GetListener());
	PS.AddListener(GUIS.GetListener());
	PSConnector<Bond> psc(&PS);
    cout<<"processing prices.txt"<<endl;
	psc.Consume("prices.txt");
	


	//trades.txt
	TradeBookingService<Bond> TBS;
	PositionService<Bond> POSS;
	HistoricalDataService<Position<Bond>> HDSPOS("positions.txt");
	RiskService<Bond> RS;
	HistoricalDataService<PV01<Bond>> HDSRISK("risk.txt");

	TBS.AddListener(POSS.GetListener());
	POSS.AddListener(HDSPOS.GetListener());
	POSS.AddListener(RS.GetListener());
	RS.AddListener(HDSRISK.GetListener());

	TBSConnector<Bond> tbsc(&TBS);
    cout<<"processing trades.txt"<<endl;
	tbsc.Consume("trades.txt");
	

	//marketdata.txt
	MarketDataService<Bond> MDS;
	AlgoExecutionService<Bond> AES;
	ExecutionService<Bond> ES;
	HistoricalDataService<ExecutionOrder<Bond>> HDSE("executions.txt");

	MDS.AddListener(AES.GetListener());
	AES.AddListener(ES.GetListener());
	ES.AddListener(HDSE.GetListener());
	ES.AddListener(TBS.GetListener());
	

	MDConnector<Bond> mdc(&MDS);
    cout<<"processing marketdata.txt"<<endl;
	mdc.Consume("marketdata.txt");
	

	//inquiry.txt
	InquiryService<Bond> IQS;
	HistoricalDataService<Inquiry<Bond>> HDSIQ("allinquiries.txt");
	IQS.AddListener(HDSIQ.GetListener());
    cout<<"processing inquiries.txt"<<endl;
	IQS.getConnector()->Consume("inquiries.txt");


	return 0;

}
