import cavise.capi.CAPIManager;

network CAPIOnlyScenario
{
    parameters:
        @display("bgb=600,220");

    submodules:
        capi: CAPIManager {
            parameters:
                @display("p=300,110");
        }
        registry: MessageRegistry {
            parameters:
                @display("p=120,40");
        }
        dealer: DummyCapiDealer {
            parameters:
                @display("p=120,110");
        }
}
