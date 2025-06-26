#include "Catalog.h"

Define_Module(Catalog);

Catalog::Catalog() {
    sendEvent = nullptr;
    sendEventNext = nullptr;
    jobToProcess = nullptr;
}

Catalog::~Catalog() {
    // Ensure all messages are disposed-of properly
    while (!messageQueue.empty()) {
        Job* job = messageQueue.front();
        messageQueue.pop();
        delete job;
        }
}


void Catalog::initialize() {
    UnitActive = true; // Marks whether the unit is active (true) or disabled (false)
    UpTimeSignal = registerSignal("UnitUptime");
    InputqueueModule = check_and_cast<Queue*>(getModuleByPath("queue2"));
    countOfMessages = 0;
    numReceivedAndFinished = 0;  // Initialize the variable
    collector = check_and_cast<StatsCollector*>(getModuleByPath("statsCollector")); // initialize pointer to statistics collection module
    // Initialize the first scheduled event for sendEvent - self-message
    sendEvent = new cMessage(("poll queue-" + std::to_string(countOfMessages)).c_str());
    countOfMessages++;
    scheduleAt(simTime(), sendEvent);
    updateDisplay();
}


void Catalog::handleMessage(cMessage *msg){
    // initialize pointers
    sendEventNext = nullptr;
    emit(UpTimeSignal, UnitActive);
    // Differentiate between messages received - Enable/disable unit, Re-check queue for jobs & end of internal job processing
    if (strncmp(msg->getName(), "end processing-", 15) == 0) {
        // Received self-msg for end of processing
        Job* finishedJob = messageQueue.front(); // store finished job into output buffer
        messageQueue.pop();

        numReceivedAndFinished++;  // Increment the counter
        collector->collectCompletedJobs(1); // increment the overall completed jobs counter
        simtime_t latency = simTime() - finishedJob->getStartTime(); // TOTAL time in system - from creation to retirement
        collector->collectJobLatency(latency); // NEW
        // Retire the message
        cancelAndDelete(finishedJob);  // delete the message
        // Further action - first check if unit is enabled.
        // if enabled - request more work normally ; if disabled - do nothing
        if (UnitActive)
            RequestWork();

    }
    else if (strncmp(msg->getName(), "poll queue-", 11) == 0) {
        // Received self-msg to re-check input queue for work
        if (UnitActive)
            RequestWork();

    }
    else if (strncmp(msg->getName(), "enable unit-", 12) == 0){
        // Received message from controller (if applicable), to change the unit's status
        EV << "*Received enable signal - starting " << getFullName() << "*" << endl;
        UnitActive = true;
        emit(UpTimeSignal, UnitActive);
        if (messageQueue.empty())
            RequestWork();
    }
    else if (strncmp(msg->getName(), "disable unit-", 13) == 0){
        // Received message from controller (if applicable), to change the unit's status
        EV << "*Received disable signal - stopping " << getFullName() << "*" << endl;
        UnitActive = false;
        emit(UpTimeSignal, UnitActive);
        if (sendEventNext != nullptr && sendEventNext -> isScheduled()) {
            if (sendEventNext->getKind() == 2){
                EV << "Received 'stop' signal, canceling pending polling of queue" << endl;
                cancelEvent(sendEventNext);
            }
        }
    }

    delete msg; // end of treatment, discard the message
    updateDisplay();

}

void Catalog::updateDisplay() {
    if (hasGUI()) {
        char buf[64];
        sprintf(buf, "  Jobs processed:%d", numReceivedAndFinished);
        getDisplayString().setTagArg("t", 0, buf); // tag "t" is for text
        getDisplayString().setTagArg("t", 1, "right"); // position
        if (UnitActive){
            if (!messageQueue.empty()) // job being processed at the moment
                getDisplayString().setTagArg("t", 2, "darkorange"); // set text color to Orange, indicating currently-processing
            else
                getDisplayString().setTagArg("t", 2, "darkgreen"); // set text color to dark green, indicating active and idle
        }
        else
            getDisplayString().setTagArg("t", 2, "darkred"); // set text color to dark red, indicating inactive
    }
}



void Catalog::RequestWork(){
        Job *jobToProcess = InputqueueModule->popJob(); // Try to get a job from input queue

        if (jobToProcess != nullptr)
        {   // If we got a valid job from queue (if not - nullptr is returned)
            // create & time a self-message indicating the end of the processing for current job
            if (hasGUI()) bubble("Pulled job from queue!");
            sendEventNext = new cMessage(("end processing-" + std::to_string(countOfMessages)).c_str());
            sendEventNext->setKind(1); // to be able to differentiate between types of self-messages
            messageQueue.push(jobToProcess);
            countOfMessages++;
            scheduleAt(simTime() + par("TimeToConsume").doubleValue(), sendEventNext);
        }
        else
        {   // Input queue was empty - no valid jobs received.
            // Schedule a self-message to re-check queue
            sendEventNext = new cMessage(("poll queue-" + std::to_string(countOfMessages)).c_str());
            sendEventNext->setKind(2); // to be able to differentiate between types of self-messages
            scheduleAt(simTime() + par("TimeToWaitIfQueueIsEmpty").doubleValue(), sendEventNext);
        }
    }
