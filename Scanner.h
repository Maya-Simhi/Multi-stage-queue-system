#ifndef __SANNER_H_
#define __SANNER_H_

#include <omnetpp.h>

using namespace omnetpp;
#include "StatsCollector.h"
class StatsCollector; // reference StatsCollector to have a pointer to this class
class Scanner : public cSimpleModule {
private:
    cMessage *sendEvent;
    simsignal_t jobCreatedSignal;
    int numJobsCreated;
    StatsCollector* collector = nullptr;
public:
    Scanner();
    virtual ~Scanner();
protected:
    virtual void initialize() override;
    virtual void handleMessage(cMessage *msg) override;
    void updateDisplay();
};

#endif
