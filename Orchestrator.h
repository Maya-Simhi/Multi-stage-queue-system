#ifndef __ORCHESTRATOR_H_
#define __ORCHESTRATOR_H_

#include <omnetpp.h>
#include "Job_m.h"
#include "Queue.h"
#include <queue>
using namespace omnetpp;
#include "StatsCollector.h"
class StatsCollector; // reference StatsCollector to have a pointer to this class
class Orchestrator : public cSimpleModule {
private:
    bool UnitActive;
    simsignal_t UpTimeSignal;
    long numReceivedAndFinished;
    cMessage *sendEvent;
    cMessage *sendEventNext;
    Job *jobToProcess;
    Queue* InputqueueModule;
    int countOfMessages;
    std::queue<Job*> messageQueue;
    StatsCollector* collector = nullptr;
public:
    Orchestrator();
    virtual ~Orchestrator();
    virtual void initialize() override;
    virtual void handleMessage(cMessage *msg) override;
    void updateDisplay();
    void RequestWork();
};

#endif
