#!/bin/bash
OPP_RUNALL=/home/ilya-cavise/omnetpp/bin/opp_runall
OPP_RUN=/home/ilya-cavise/omnetpp/bin/opp_run_dbg
NED_FOLDERS="/home/ilya-cavise/artery/src/artery:/home/ilya-cavise/artery/src/traci:/home/ilya-cavise/artery/extern/veins/examples/veins:/home/ilya-cavise/artery/extern/veins/src/veins:/home/ilya-cavise/artery/extern/inet/src:/home/ilya-cavise/artery/extern/inet/examples:/home/ilya-cavise/artery/extern/inet/tutorials:/home/ilya-cavise/artery/extern/inet/showcases"
LIBRARIES="-l/home/ilya-cavise/artery/scenarios/src/artery/libartery_core.so -l/home/ilya-cavise/artery/scenarios/src/traci/libtraci.so -l/home/ilya-cavise/artery/scenarios/extern/libveins.so -l/home/ilya-cavise/artery/scenarios/extern/libINET.so -l/home/ilya-cavise/artery/scenarios/src/artery/storyboard/libartery_storyboard.so -l/home/ilya-cavise/artery/scenarios/src/artery/envmod/libartery_envmod.so"

RUNALL=false
for arg do
    shift
    [[ "$arg" == -j* ]] && RUNALL=true && J=$arg && continue
    [[ "$arg" == -b* ]] && RUNALL=true && B=$arg && continue
    # run opp_runall with default values for -j* and -b* options by just specifying '--all'
    [[ "$arg" == "--all" ]] && RUNALL=true && continue
    set -- "$@" "$arg"
done

if [ "$RUNALL" = true ] ; then
    $OPP_RUNALL $J $B $OPP_RUN -n $NED_FOLDERS $LIBRARIES $@
else
    $OPP_RUN -n $NED_FOLDERS $LIBRARIES $@
fi
