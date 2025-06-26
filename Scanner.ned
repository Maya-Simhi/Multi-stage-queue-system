simple Scanner {
    parameters:
        volatile double interArrivalTime @unit(s) = default(1s);
        @signal[messagesCreatedCount](type="long");
        @statistic[messagesCreatedCount](title="number of messages created"; record=vector,mean,max, avg; interpolationmode=linear);
      
    gates:
        output out[];
}
