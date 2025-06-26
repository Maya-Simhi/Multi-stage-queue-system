network StaticNetwork
{
    @display("bgb=51000,215000;bgg=1000,2,grey95;bgu=km");

    int numScanners;
    int numOrchs;
    int numcatalogs;

    submodules:
        scanner[numScanners]: Scanner {
            @display("i=block/source;p=4000,${(4000 * index) + 4000}");
        }

        queue: Queue {
            @display("p=15540,5610;i=block/queue");
        }
        queue2: Queue {
            @display("p=29150,5640;i=block/queue");
        }
        orchestrator[numOrchs]: Orchestrator {
            @display("i=block/buffer2;p=22340,${(3700 * index) + 5640}");
        }
        catalog[numcatalogs]: Catalog {
            @display("p=43210,${5640 + (2200 * index)};i=block/sink");

        }
        statsCollector: StatsCollector {
            @display("i=block/filter;p=47805,1426");
        }
    connections:
        for i=0..numScanners-1 {
            scanner[i].out++ --> queue.in++;
        }
        for i=0..numOrchs-1 {
            queue.out++ --> orchestrator[i].in++;
            orchestrator[i].out++ --> queue2.in++;
        }
        for i=0..numcatalogs-1 {
            queue2.out++ --> catalog[i].in++;
        }
}
