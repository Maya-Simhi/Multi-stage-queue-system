#include "StatsCollector.h"

Define_Module(StatsCollector);

StatsCollector::StatsCollector() {
}


void StatsCollector::initialize() {
    JobLatencyCombined = registerSignal("JobLatency");
    CompletedJobsCombined = registerSignal("CompletedJobs");
    GeneratedJobscombined = registerSignal("GeneratedJobs");
    CompletedOrchWork = registerSignal("CompletedOrchJobs");
    DroppedJobsCombined = registerSignal("DroppedJobs");
    SumCompletedJobs = 0;                                   // initialize at 0 completed jobs at start
    SumGeneratedJobs = 0;                                   // initialize at 0 - generated jobs by Scanners
    SumCompletedOrchWork = 0;                               // initialize at 0 - orchestrator's done work
    SumDroppedJobs = 0;                                     // initialize at 0 - no dropped jobs when starting
    TotalDroppedVsGenerated = 0;                            // ratio between dropped jobs to overall generated jobs
    TotalDroppedVsCompleted = 0;                            // ratio between dropped jobs to overall completed jobs
}

void StatsCollector:: collectJobLatency(simtime_t JobLatency){
    emit(JobLatencyCombined, JobLatency);
}

void StatsCollector::collectCompletedJobs(long JobCount){
    SumCompletedJobs += JobCount;
    emit(CompletedJobsCombined, SumCompletedJobs);
}

void StatsCollector::collectGeneratedJobs(long JobCount){
    SumGeneratedJobs += JobCount;
    emit(GeneratedJobscombined, SumGeneratedJobs);
}

void StatsCollector::collectCompletedOrchWork(long JobCount){
    SumCompletedOrchWork += JobCount;
    emit(CompletedOrchWork, SumCompletedOrchWork);
}

void StatsCollector::collectDroppedJobs(long Dropped){
    SumDroppedJobs += Dropped;
    emit(DroppedJobsCombined, Dropped);
}

void StatsCollector::finish(){
    TotalDroppedVsGenerated = static_cast<float>(SumDroppedJobs)/static_cast<float>(SumGeneratedJobs);
    TotalDroppedVsCompleted = static_cast<float>(SumDroppedJobs)/static_cast<float>(SumCompletedJobs);
    recordScalar("droppedGeneratedRatio", TotalDroppedVsGenerated);
    recordScalar("droppedCompletedRatio", TotalDroppedVsCompleted);
}

