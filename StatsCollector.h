#ifndef STATSCOLLECTOR_H_
#define STATSCOLLECTOR_H_

#include <omnetpp.h>
using namespace omnetpp;
class StatsCollector : public cSimpleModule {
public:
    StatsCollector();
    void collectJobLatency(simtime_t JobLatency);
    void collectCompletedJobs(long JobCount);
    void collectGeneratedJobs(long JobCount);
    void collectCompletedOrchWork(long JobCount);
    void collectDroppedJobs(long Dropped);
private:
    simsignal_t JobLatencyCombined;
    simsignal_t CompletedJobsCombined;
    simsignal_t GeneratedJobscombined;
    simsignal_t CompletedOrchWork;
    simsignal_t DroppedJobsCombined;
    long SumCompletedJobs;
    long SumGeneratedJobs;
    long SumCompletedOrchWork;
    long SumDroppedJobs;
    // test
    float TotalDroppedVsGenerated;
    float TotalDroppedVsCompleted;
protected:
    virtual void initialize() override;
    virtual void finish() override;
};

#endif
