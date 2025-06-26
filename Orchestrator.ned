simple Orchestrator {
    parameters:
        @signal[UnitUptime](type="bool");
        @statistic[UnitUptime](title="Uptime of pod"; record=vector,stats,max,min,mean);
        volatile double TimeToConsume @unit(s) = default(1s);
        volatile double TimeToWaitIfQueueIsEmpty @unit(s) = default(0.001s);
        
        
    gates:
        input in[];
        output out[];
}
