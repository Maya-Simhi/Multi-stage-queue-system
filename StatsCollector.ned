simple StatsCollector {
    parameters:
        @signal[JobLatency](type="simtime_t");
        @statistic[JobLatency](title="Merged Job Latency"; unit=s; record=vector,mean,max,min);
		
		@signal[CompletedJobs](type="long");
		@statistic[CompletedJobs](title="Number Of Completed Jobs"; record=vector,mean,max,min,count; interpolationmode=linear); //linear = smooth changes
        
        @signal[GeneratedJobs](type="long");
		@statistic[GeneratedJobs](title="Number Of Generated Jobs"; record=vector,mean,max,min,count; interpolationmode=linear); //linear = smooth changes
        
        @signal[CompletedOrchJobs](type="long");
		@statistic[CompletedOrchJobs](title="Number Of Jobs Orchestrators Finished Processing"; record=vector,mean,max,min,count; interpolationmode=linear); //linear = smooth changes
        
        @signal[DroppedJobs](type="long");
		@statistic[DroppedJobs](title="Number Of Dropped Jobs (across both queues)"; record=count);
        
        // collects ratios between dropped jobs to generated/completed jobs at the end of the simulation
        @statistic[droppedGeneratedRatio](title="Dropped Jobs to Generated Jobs Ratio"; unit="");
        @statistic[droppedCompletedRatio](title="Dropped Jobs to Completed Jobs Ratio"; unit="");
        
        
        
    gates:

}
