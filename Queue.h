#include <omnetpp.h>
#include "Job_m.h"
using namespace omnetpp;
#include "StatsCollector.h"
class StatsCollector; // reference StatsCollector to have a pointer to this class
class Queue : public cSimpleModule{
private:
    simsignal_t droppedSignal;
    simsignal_t queueLengthSignal;
    simsignal_t queueLatency;
    Job *jobServiced = nullptr;
    cMessage *endServiceMsg = nullptr;
    cQueue queue;
    int capacity;
    StatsCollector* collector = nullptr;
public:
    virtual ~Queue();
    cQueue queueToSend;
    Job* popJob();
    int getQueueLength();
protected:
    virtual void initialize() override;
    virtual void handleMessage(cMessage *msg) override;
    virtual simtime_t startService(Job *job);
    virtual void endService(Job *job);
    void updateDisplay();
};

