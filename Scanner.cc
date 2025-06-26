#include "Scanner.h"

#include "Job_m.h"

Define_Module(Scanner);

Scanner::Scanner() {
    sendEvent = nullptr;
}

Scanner::~Scanner() {
    cancelAndDelete(sendEvent);
}

void Scanner::initialize() {
    numJobsCreated = 0;
    sendEvent = new cMessage("sendJob");
    jobCreatedSignal = registerSignal("messagesCreatedCount");
    collector = check_and_cast<StatsCollector*>(getModuleByPath("statsCollector")); // initialize pointer to statistics collection module
    scheduleAt(simTime() + par("interArrivalTime").doubleValue(), sendEvent);
    updateDisplay();
}

void Scanner::handleMessage(cMessage *msg) {
    if (msg == sendEvent) {
        Job *job = new Job(("msg-" + std::to_string(numJobsCreated)).c_str());
        job->setStartTime(simTime()); // Store the initialization time of the job into it
        send(job, "out",0);
        numJobsCreated++;
        emit(jobCreatedSignal, numJobsCreated); //emit number of individual jobs created
        collector->collectGeneratedJobs(1); // increment overall generated jobs counter
        scheduleAt(simTime() + par("interArrivalTime").doubleValue(), sendEvent); //next time we create a message will take interArrivalTime
        updateDisplay();
    }
}

void Scanner::updateDisplay() {
    if (hasGUI()) {
        char buf[64];
        sprintf(buf, "Jobs created: %d", numJobsCreated);
        getDisplayString().setTagArg("t", 0, buf); // tag "t" is for text
        getDisplayString().setTagArg("t", 1, "above"); // position above
        getDisplayString().setTagArg("t", 2, "darkblue"); // text color
    }
}
