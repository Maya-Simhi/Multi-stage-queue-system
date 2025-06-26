
#ifndef CONTROLLER_H_
#define CONTROLLER_H_
#include <vector>
#include <omnetpp.h>
using namespace omnetpp;

class Controller : public cSimpleModule{
private:
    cMessage* pollSystem;
    int MaxOrchNum;
    int MaxCatalogNum;
    int queue1_sampling_history; // length of sampling history array
    int queue2_sampling_history; // length of sampling history array
    int CurrentOrchAmount; // tracks how many pods are active
    int CurrentCatalogAmount; // tracks how many pods are active
    int ControlMsgsSent;
    //statistics
    simsignal_t StatsOrchPodsNum;
    simsignal_t StatsCatalogPodsNum;
    simsignal_t StatsControlMsgsSent;

    std::string ControlFunction;

    // linear control function related
    int queue1FullThrottle; // queue capacity for which we max out the number of Orch pods
    int queue2FullThrottle; // queue capacity for which we max out the number of Catalog pods
    int numOfMinOrch;
    int numOfMinCatalog;
    std::vector<int> Q1_samples;
    std::vector<int> Q2_samples;
    int counter; // this helps for the samples' arrays
    int PodReductionThreshold; // number of sampling done before we can reduce # of pods

    // QDTE control function related
    int stabilityCounterOrch;
    int stabilityCounterCatalog;
    double serviceRateOrch;
    double serviceRateCatalog;
    int stabilityThreshold;
    double latencyTargetQ1;
    double latencyTargetQ2;
    int increaseRateQ1;
    int increaseRateQ2;

public:
    Controller();
    virtual void initialize() override;
    virtual void handleMessage(cMessage *msg) override;
    int getQueue1Length();
    int getQueue2Length();
    void applyScalingOrch(int averageSample);
    void applyScalingCatalogs(int averageSample);
    int averageSampleArray(const std::vector<int> &sample_array);
    virtual ~Controller();
    void DisableOrchs(int disable_from, int disable_to);
    void EnableOrchs(int enable_from, int enable_to);
    void DisableCatalogs(int disable_from, int disable_to);
    void EnableCatalogs(int enable_from, int enable_to);


};

#endif /* CONTROLLER_H_ */
