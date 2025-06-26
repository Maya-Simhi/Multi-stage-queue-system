network TestNetwork
{
     @display("bgb=1000,45000;bgg=1000,2,grey95;bgu=km");
    
    int numScanners ;
    int numOrchs ;
    int numcatalogs ;
    
    submodules:
        scanner[numScanners]: Scanner {
            @display("i=block/source;p=50,${160 * index}");
        }

        queue: Queue {
            @display("p=270,100;i=block/queue");
        }
        queue2: Queue {
            @display("p=630,100;i=block/queue");
        }
        orchestrator[numOrchs]: Orchestrator {
            @display("i=block/buffer2;p=440,${160 * index}");
        }
        catalog[numcatalogs]: Orchestrator {
            @display("p=780,${160 * index};i=block/sink");
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

