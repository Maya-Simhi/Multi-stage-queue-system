simple Controller {
    parameters:
        @signal[orchPodsNum](type="long");
        @signal[catalogPodsNum](type="long");
        @signal[ControlMessagesSent](type="long");
        @statistic[ControlMessagesSent](title="number of control messages sent"; record=vector,stats,count;);
        @statistic[catalogPodsNum](title = "Number of catalog pods";  record=vector,mean,max;);
        @statistic[orchPodsNum](title="Number of orch pods"; record=vector,mean,max;);

        // parameter declarations
        // permanent parameters
        int numOrchs = default(12); // 11 is the needed amount for queue stability
        int numcatalogs = default(175); // slightly above 171 orchs for stable queue
        int numMinOrchs = default(3); // lowest number of orchs possible
        int numMinCatalog = default(6); // lowest number of catalogs possible
        string controlFuncUsed = default("Linear");
        double SamplingFreq = default(0.5); // sample every 0.5 seconds
        
        // Linear control function related
        int queue1SamplingHistory = default(1200); // 10 minutes' history
        int queue2SamplingHistory = default(1200); // 10 minutes' history
        int queueFullThrottle = default(100); // when queue reaches it, all pods are deployed
        int PodReductionThreshold = default(240); // down-scaling possible only after 2 minutes 
        
        int samplingPairIndex = default(1); // For zipping elements in the .ini file (parameter sweep)
        
        // QDTE control function related
        double orchServiceRate = default(0.25);
        double catalogServiceRate = default(5);
        int stabilityThreshold = default(5);
        double latencyTargetQueue1 = default(0.01); // 10x the avg for static sim
        double latencyTargetQueue2 = default(0.08); // 10x the avg for static sim        
		int increaseRateQueue1 = default(1);
		int increaseRateQueue2 = default(1);
		
		// Trend control function related
		
		
    gates:
        output orch_out[];
        output catalog_out[];
}
