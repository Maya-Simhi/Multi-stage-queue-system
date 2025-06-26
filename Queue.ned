simple Queue {
    parameters:
        int capacity = default(-1);
        volatile double serviceTime @unit(s) = default(1s);
        @display("i=block/server");
    	@signal[dropped](type="long");
        @signal[queueLength](type="long");
    	@signal[queueLatency](type="simtime_t");
        @statistic[dropped](title="drop event"; record=count);
        @statistic[queueLength](title="queue length"; record=vector,max, avg; interpolationmode=sample-hold);// sample-hold means the values are const
        @statistic[queueLatency](title="Job's Latency within queue"; unit=s; record=vector,mean,max,stats,histogram);
    gates:
        input in[];
        output out[];

}
