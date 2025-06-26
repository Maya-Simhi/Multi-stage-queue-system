#include "Queue.h"

Define_Module(Queue);

Queue::~Queue() {
    delete jobServiced;
    cancelAndDelete(endServiceMsg);
}

void Queue::initialize() {
    // Register signals
    droppedSignal = registerSignal("dropped");
    queueLengthSignal = registerSignal("queueLength");
    queueLatency = registerSignal("queueLatency");
    EV << getFullName() << " init: " << queueToSend.getLength() <<endl;
    // Read parameters
    capacity = par("capacity");
    // Initialize variables
    endServiceMsg = new cMessage("endService");
    collector = check_and_cast<StatsCollector*>(getModuleByPath("statsCollector")); // initialize pointer to statistics collection module
    updateDisplay();
}

void Queue::handleMessage(cMessage *msg) {
    if (msg == endServiceMsg) { // End of service
        endService(jobServiced);
        if (!queue.isEmpty()) { // queue is not empty so send another end service message
            jobServiced = check_and_cast<Job *>(queue.pop());
            scheduleAt(simTime() + startService(jobServiced), endServiceMsg);
        } else {
            jobServiced = nullptr;
        }
    } else { // New job arrival
        Job *job = check_and_cast<Job *>(msg);
        EV << getFullName() << " queue length: " << queueToSend.getLength() <<endl;
        job->setQueue_arrivalTime(simTime()); // stores arrival time to queue

        // Check total jobs in queue and queueToSend before accepting new job
        int totalJobs = queue.getLength() + queueToSend.getLength();
        if (capacity >= 0 && totalJobs >= capacity){
            emit(droppedSignal, 1);
            collector->collectDroppedJobs(1);
            delete job;
            if (hasGUI()) bubble("Job dropped!");
        }
        else{ // within queue capacity (if capacity= -1 it'll also redirect here
            if (!jobServiced) { // No job being serviced we can start working on it
                jobServiced = job;
                scheduleAt(simTime() + startService(jobServiced), endServiceMsg);
            }
            else { // job is currently being serviced, add to pre-processing queue
                queue.insert(job);
            }
        }
    }
    updateDisplay();
}

simtime_t Queue::startService(Job *job) {
    return par("serviceTime");
}

void Queue::endService(Job *job) {
    queueToSend.insert(job);
    EV << getFullName() << " queue length: " << queueToSend.getLength() <<endl;
    emit(queueLengthSignal, queueToSend.getLength());
}



Job* Queue::popJob() {
    // Pop a job from queueToSend and return it
    if (!queueToSend.isEmpty()) {
        EV << getFullName() << ":" << queueToSend.getLength() <<endl;
        Job* jb = check_and_cast<Job*>(queueToSend.pop());
        emit(queueLatency,simTime() - jb->getQueue_arrivalTime()); // emits the latency of the queue
        emit(queueLengthSignal, queueToSend.getLength());
        updateDisplay();
        return jb;
    }
    return nullptr;  // Return nullptr if the queue is empty
}

void Queue::updateDisplay() {
    if (hasGUI()) {
        char buf[64];
        sprintf(buf, "Jobs in queue: %ld", queueToSend.getLength());
        getDisplayString().setTagArg("t", 0, buf); // tag "t" is for text
        getDisplayString().setTagArg("t", 1, "above"); // position above
        getDisplayString().setTagArg("t", 2, "green"); // text color
    }
}

int Queue::getQueueLength(){
    return queueToSend.getLength();
}
