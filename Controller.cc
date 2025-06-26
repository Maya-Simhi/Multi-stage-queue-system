#include "Controller.h"
#include "Queue.h"

Define_Module(Controller);

Controller::Controller() {

}

void Controller::initialize() {
    // Permanent parameters
    MaxOrchNum = gateSize("orch_out"); // The number of overall available pods within simulation
    MaxCatalogNum = gateSize("catalog_out"); // same ^^
    CurrentOrchAmount = par("numOrchs"); // tracks how many pods are active - starts at minimal value
    CurrentCatalogAmount = par("numcatalogs"); // ^^
    numOfMinOrch = par("numMinOrchs"); // minimal number of enabled orchestrators
    numOfMinCatalog = par("numMinCatalog"); // same but for Catalogs ^^

    // Determine which control function to use
    ControlFunction = par("controlFuncUsed").stdstringValue(); // Can be Linear / QDTE

    // Statistics Gathering
    ControlMsgsSent = 0; // how many control signals were sent (enable/disable messages)
    StatsOrchPodsNum = registerSignal("orchPodsNum"); // tracks how many Orchs are active throughout the sim
    StatsCatalogPodsNum = registerSignal("catalogPodsNum"); // tracks how many Catalogs are active throughout the sim
    StatsControlMsgsSent = registerSignal("ControlMessagesSent"); // tracks how many overall control messages were sent to the system (enable/disable messages)

    // -- Define relevant parameters based on current control function --
    if (ControlFunction == "QDTE")
    {
        // QDTE Control function related parameters and definitions
        serviceRateOrch = par("orchServiceRate"); // how long does an orchestrator takes to service 1 job
        serviceRateCatalog = par("catalogServiceRate"); // ^^ same but for catalogs
        stabilityThreshold = par("stabilityThreshold"); // how long we should be stable before reducing the number of pods used
        latencyTargetQ1 = par("latencyTargetQueue1"); // target latency to compare to in this queue (scanners-orchs)
        latencyTargetQ2 = par("latencyTargetQueue2"); // target latency to compare to in this queue (orchs-catalogs)
        stabilityCounterOrch = 0; // keeps track of how long we've had stable # of orchs
        stabilityCounterCatalog = 0; // same but for catalogs ^^
        increaseRateQ1 = par("increaseRateQueue1"); // when increasing # of pods, how many do we increase at once
        increaseRateQ2 = par("increaseRateQueue2"); // ^^
    }
    else if (ControlFunction == "Linear")
    {
        // Linear Control function related parameters
        queue1_sampling_history = par("queue1SamplingHistory"); // how many past-samples we are holding for queue 1
        queue2_sampling_history = par("queue2SamplingHistory"); // same ^^ but for queue 2
        queue1FullThrottle = par("queueFullThrottle"); // # of queue capacity at which we deploy the maximum number of pods we have (i.e. 80)
        queue2FullThrottle = par("queueFullThrottle"); // same ^^
        Q1_samples.resize(queue1_sampling_history ,static_cast<int>((numOfMinOrch+MaxOrchNum)/2)); // array storing all recent samples of queue1's length - initialized to average value
        Q2_samples.resize(queue2_sampling_history, static_cast<int>((numOfMinCatalog+MaxCatalogNum)/2)); // same but for queue2 ^^
        counter = 0; // helper variable to store past samples
        PodReductionThreshold = std::max(queue1_sampling_history, queue2_sampling_history); // number of samples gathered before pods' reduction is enabled
    }

    // Initial disabling of pods to starting value
    DisableOrchs(numOfMinOrch, MaxOrchNum);
    DisableCatalogs(numOfMinCatalog, MaxCatalogNum);

    // emit initial signals
    emit(StatsOrchPodsNum, CurrentOrchAmount);
    emit(StatsCatalogPodsNum, CurrentCatalogAmount);

    EV << "***Load Controller Initialized***\nCurrent pod quantities - Orchestrators: " << CurrentOrchAmount << "/" << MaxOrchNum
       << "\nCatalogs: " << CurrentCatalogAmount << "/" << MaxCatalogNum << endl;

    pollSystem = new cMessage("Survey System"); //schedule next check of queues
    scheduleAt(simTime()+par("SamplingFreq").doubleValue(), pollSystem);
}

Controller::~Controller() {
    cancelAndDelete(pollSystem);
}


void Controller::handleMessage(cMessage *msg){
    if (msg == pollSystem) {
        // Check which control function to use
        if (ControlFunction == "Linear")
        {
            // Get current queues lengths
            Q2_samples[counter % Q2_samples.size()] = getQueue2Length();
            Q1_samples[counter % Q1_samples.size()] = getQueue1Length();
            // Calculate average queue length
            int avgQueue1Length = averageSampleArray(Q1_samples); // calculate average queue length
            int avgQueue2Length = averageSampleArray(Q2_samples); // calculate average queue length
            // call functions applying the scale-up / scale-down logic
            applyScalingOrch(avgQueue1Length); // apply up/down-scaling function for Orchestrators
            applyScalingCatalogs(avgQueue2Length); // apply up/down-scaling function for Catalogs
            counter++; // increment number of samples taken
        }
        else if (ControlFunction == "QDTE")
        {
            // Get current queue1 length
            int CurrQ1 = getQueue1Length();
            // estimate drain rate
            float drainRateQ1 = static_cast<float>(CurrQ1) / (static_cast<float>(CurrentOrchAmount)*static_cast<float>(serviceRateOrch));
            if (drainRateQ1 > latencyTargetQ1){
                EnableOrchs(static_cast<int>(CurrentOrchAmount), static_cast<int>(std::min(CurrentOrchAmount+increaseRateQ1,MaxOrchNum)));
                stabilityCounterOrch = 0; // re-balanced, reset counter
            }
            else if ((drainRateQ1 <= latencyTargetQ1) && (stabilityCounterOrch >= stabilityThreshold)){ // stable for long, try to reduce pod count
                DisableOrchs(std::max(numOfMinOrch,CurrentOrchAmount-1),CurrentOrchAmount); // decrease pod count by 1
                stabilityCounterOrch++; // increase stability counter
            }
            else { // stable, but not long enough to reduce pod count
                stabilityCounterOrch++;
            }

            // Get current queue2 length
            int CurrQ2 = getQueue2Length();
            // estimate drain rate
            float drainRateQ2 = static_cast<float>(CurrQ2) / (static_cast<float>(CurrentCatalogAmount)*static_cast<float>(serviceRateCatalog));
            if (drainRateQ2 > latencyTargetQ2){
                EnableCatalogs(static_cast<int>(CurrentCatalogAmount), static_cast<int>(std::min(CurrentCatalogAmount+increaseRateQ2,MaxCatalogNum)));
                stabilityCounterCatalog = 0; // re-balanced, reset counter
            }
            else if ((drainRateQ2 <= latencyTargetQ2) && (stabilityCounterCatalog >= stabilityThreshold)){ // stable for long, try to reduce pod count
                DisableCatalogs(std::max(numOfMinCatalog,CurrentCatalogAmount-1),CurrentCatalogAmount); // decrease pod count by 1
                stabilityCounterCatalog++; // increase stability counter
            }
            else { // stable, but not long enough to reduce pod count
                stabilityCounterCatalog++;
            }

        }

        // Emit signals - after treating based on chosen control function
        emit(StatsOrchPodsNum, CurrentOrchAmount);
        emit(StatsCatalogPodsNum, CurrentCatalogAmount);

        // re-send self message to check queue's status again and adjust if necessary
        scheduleAt(simTime() + par("SamplingFreq").doubleValue(), pollSystem);
    }
    else{ // discard any other message received (which we shouldn't have)
        delete(msg);
    }
}

void Controller::applyScalingOrch(int averageSample){
    // Up-scaling mechanism: linear addition of pods, where maximal deployment is achieved for
    // queueFullThrottle capacity, and 0 capacity leads to numOfMinOrch.
    int desired_amount = std::min(
            Q1_samples[counter % Q1_samples.size()]*(MaxOrchNum-numOfMinOrch)/queue1FullThrottle + numOfMinOrch, // up-scaling linear equation
            MaxOrchNum); // lower boundary
    if (desired_amount > CurrentOrchAmount){ // if we need more pods than currently enabled
        EnableOrchs(CurrentOrchAmount, desired_amount); // enable the extra pods
    }
    // Down-scaling mechanism - has an initial ramp-up time
    else if (counter >= PodReductionThreshold){ // if the down-scaling is active (has a starting delay)
        // calculate how many pods are supposed to be deployed based on the last sample's average
        int avgDesiredAmount = averageSample*(MaxOrchNum-numOfMinOrch)/queue1FullThrottle + numOfMinOrch;
        // Down-scaling mechanism: if the # of pods using the average is lower than current deployed number and >= amount suggested by latest sample
        if (avgDesiredAmount < CurrentOrchAmount && avgDesiredAmount >= desired_amount){
            EV << "Too many Orchestrators active, Down-scaling from " << CurrentOrchAmount << "to " << avgDesiredAmount << endl;
            DisableOrchs(avgDesiredAmount, CurrentOrchAmount); // disable the extra pods
        }
    }
}

void Controller::applyScalingCatalogs(int averageSample){
    // Up-scaling mechanism: linear addition of pods, where maximal deployment is achieved for
    // queueFullThrottle capacity, and 0 capacity leads to numOfMinOrch.
    int desired_amount = std::min(
            Q2_samples[counter % Q2_samples.size()]*(MaxCatalogNum-numOfMinCatalog)/queue2FullThrottle + numOfMinCatalog,  // up-scaling linear equation
            MaxCatalogNum); // lower boundary
    if (desired_amount > CurrentCatalogAmount){ // if we need more pods than currently enabled
        EnableCatalogs(CurrentCatalogAmount, desired_amount); // enable the extra pods
    }
    // Down-scaling mechanism - has an initial ramp-up time
    else if (counter >= PodReductionThreshold){ // if the down-scaling is active (has a starting delay)
        // calculate how many pods are supposed to be deployed based on the last sample's average
        int avgDesiredAmount = averageSample*(MaxCatalogNum-numOfMinCatalog)/queue2FullThrottle + numOfMinCatalog;
        // Down-scaling mechanism: if the # of pods using the average is lower than current deployed number and >= amount suggested by latest sample
        if (avgDesiredAmount < CurrentCatalogAmount && avgDesiredAmount >= desired_amount){
            EV << "Too many Catalogs active, Down-scaling from " << CurrentCatalogAmount << "to " << avgDesiredAmount << endl;
            DisableCatalogs(avgDesiredAmount, CurrentCatalogAmount); // disable the extra pods
        }
    }
}

int Controller::averageSampleArray(const std::vector<int> &sample_array){
    int sum = 0;
    if (sample_array.empty()) return 0;
    for (int value : sample_array){
        sum += value;
    }
    return static_cast<int>(std::round(sum/sample_array.size()));
}

int Controller::getQueue1Length(){
    Queue* queue = check_and_cast<Queue*>(getModuleByPath("queue"));
    return queue->getQueueLength();
}

int Controller::getQueue2Length(){
    Queue* queue = check_and_cast<Queue*>(getModuleByPath("queue2"));
    return queue->getQueueLength();
}

void Controller::DisableOrchs(int disable_from, int disable_to) {
    for (int i=disable_from; i<disable_to; i++){
        cMessage *disableMsg = new cMessage("disable unit-"); //create a disable message
        send(disableMsg, "orch_out", i); // send message to orch[i]
        ControlMsgsSent++; // log control message sent
    }
    CurrentOrchAmount -= (disable_to - disable_from);
    emit(StatsControlMsgsSent, (disable_to - disable_from));
}

void Controller::EnableOrchs(int enable_from, int enable_to){
    for (int i=enable_from; i<enable_to; i++){
        cMessage *enableMsg = new cMessage("enable unit-"); //create an enable message
        send(enableMsg, "orch_out", i); // send message to orch[i]
        ControlMsgsSent++; // log control message sent
    }
    CurrentOrchAmount += (enable_to - enable_from);
    emit(StatsControlMsgsSent,(enable_to - enable_from));
}

void Controller::DisableCatalogs(int disable_from, int disable_to) {
    for (int i=disable_from; i<disable_to; i++){
        cMessage *disableMsg = new cMessage("disable unit-"); //create a disable message
        send(disableMsg, "catalog_out", i); // send message to catalog[i]
        ControlMsgsSent++; // log control message sent
       }
    CurrentCatalogAmount-= (disable_to - disable_from);
    emit(StatsControlMsgsSent, (disable_to - disable_from));
}

void Controller::EnableCatalogs(int enable_from, int enable_to){
    for (int i=enable_from; i<enable_to; i++){
        cMessage *enableMsg = new cMessage("enable unit-"); //create an enable message
        send(enableMsg, "catalog_out", i); // send message to catalog[i]
        ControlMsgsSent++; // log control message sent
    }
    CurrentCatalogAmount += (enable_to - enable_from);
    emit(StatsControlMsgsSent,(enable_to - enable_from));
}


