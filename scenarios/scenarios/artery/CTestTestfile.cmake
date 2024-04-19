# CMake generated Testfile for 
# Source directory: /home/ilya-cavise/artery/scenarios/artery
# Build directory: /home/ilya-cavise/artery/scenarios/scenarios/artery
# 
# This file includes the relevant testing commands required for 
# testing this directory and lists subdirectories to be tested as well.
add_test(example-inet "/home/ilya-cavise/omnetpp/bin/opp_run_dbg" "-n" "/home/ilya-cavise/artery/src/artery:/home/ilya-cavise/artery/src/traci:/home/ilya-cavise/artery/extern/veins/examples/veins:/home/ilya-cavise/artery/extern/veins/src/veins:/home/ilya-cavise/artery/extern/inet/src:/home/ilya-cavise/artery/extern/inet/examples:/home/ilya-cavise/artery/extern/inet/tutorials:/home/ilya-cavise/artery/extern/inet/showcases" "-l" "/home/ilya-cavise/artery/scenarios/src/artery/envmod/libartery_envmod.so" "-l" "/home/ilya-cavise/artery/scenarios/src/artery/storyboard/libartery_storyboard.so" "-l" "/home/ilya-cavise/artery/scenarios/extern/libINET.so" "-l" "/home/ilya-cavise/artery/scenarios/extern/libveins.so" "-l" "/home/ilya-cavise/artery/scenarios/src/traci/libtraci.so" "-l" "/home/ilya-cavise/artery/scenarios/src/artery/libartery_core.so" "omnetpp.ini" "-uCmdenv" "-cinet" "--sim-time-limit=20s")
set_tests_properties(example-inet PROPERTIES  WORKING_DIRECTORY "/home/ilya-cavise/artery/scenarios/artery" _BACKTRACE_TRIPLES "/home/ilya-cavise/artery/cmake/AddOppRun.cmake;170;add_test;/home/ilya-cavise/artery/scenarios/artery/CMakeLists.txt;2;add_opp_test;/home/ilya-cavise/artery/scenarios/artery/CMakeLists.txt;0;")
add_test(example-security "/home/ilya-cavise/omnetpp/bin/opp_run_dbg" "-n" "/home/ilya-cavise/artery/src/artery:/home/ilya-cavise/artery/src/traci:/home/ilya-cavise/artery/extern/veins/examples/veins:/home/ilya-cavise/artery/extern/veins/src/veins:/home/ilya-cavise/artery/extern/inet/src:/home/ilya-cavise/artery/extern/inet/examples:/home/ilya-cavise/artery/extern/inet/tutorials:/home/ilya-cavise/artery/extern/inet/showcases" "-l" "/home/ilya-cavise/artery/scenarios/src/artery/envmod/libartery_envmod.so" "-l" "/home/ilya-cavise/artery/scenarios/src/artery/storyboard/libartery_storyboard.so" "-l" "/home/ilya-cavise/artery/scenarios/extern/libINET.so" "-l" "/home/ilya-cavise/artery/scenarios/extern/libveins.so" "-l" "/home/ilya-cavise/artery/scenarios/src/traci/libtraci.so" "-l" "/home/ilya-cavise/artery/scenarios/src/artery/libartery_core.so" "omnetpp.ini" "-uCmdenv" "-cinet_security" "--sim-time-limit=20s")
set_tests_properties(example-security PROPERTIES  WORKING_DIRECTORY "/home/ilya-cavise/artery/scenarios/artery" _BACKTRACE_TRIPLES "/home/ilya-cavise/artery/cmake/AddOppRun.cmake;170;add_test;/home/ilya-cavise/artery/scenarios/artery/CMakeLists.txt;3;add_opp_test;/home/ilya-cavise/artery/scenarios/artery/CMakeLists.txt;0;")
add_test(example-inet-mco "/home/ilya-cavise/omnetpp/bin/opp_run_dbg" "-n" "/home/ilya-cavise/artery/src/artery:/home/ilya-cavise/artery/src/traci:/home/ilya-cavise/artery/extern/veins/examples/veins:/home/ilya-cavise/artery/extern/veins/src/veins:/home/ilya-cavise/artery/extern/inet/src:/home/ilya-cavise/artery/extern/inet/examples:/home/ilya-cavise/artery/extern/inet/tutorials:/home/ilya-cavise/artery/extern/inet/showcases" "-l" "/home/ilya-cavise/artery/scenarios/src/artery/envmod/libartery_envmod.so" "-l" "/home/ilya-cavise/artery/scenarios/src/artery/storyboard/libartery_storyboard.so" "-l" "/home/ilya-cavise/artery/scenarios/extern/libINET.so" "-l" "/home/ilya-cavise/artery/scenarios/extern/libveins.so" "-l" "/home/ilya-cavise/artery/scenarios/src/traci/libtraci.so" "-l" "/home/ilya-cavise/artery/scenarios/src/artery/libartery_core.so" "omnetpp.ini" "-uCmdenv" "-cinet_mco" "--sim-time-limit=20s")
set_tests_properties(example-inet-mco PROPERTIES  WORKING_DIRECTORY "/home/ilya-cavise/artery/scenarios/artery" _BACKTRACE_TRIPLES "/home/ilya-cavise/artery/cmake/AddOppRun.cmake;170;add_test;/home/ilya-cavise/artery/scenarios/artery/CMakeLists.txt;4;add_opp_test;/home/ilya-cavise/artery/scenarios/artery/CMakeLists.txt;0;")
add_test(example-inet-mixed-vehicles "/home/ilya-cavise/omnetpp/bin/opp_run_dbg" "-n" "/home/ilya-cavise/artery/src/artery:/home/ilya-cavise/artery/src/traci:/home/ilya-cavise/artery/extern/veins/examples/veins:/home/ilya-cavise/artery/extern/veins/src/veins:/home/ilya-cavise/artery/extern/inet/src:/home/ilya-cavise/artery/extern/inet/examples:/home/ilya-cavise/artery/extern/inet/tutorials:/home/ilya-cavise/artery/extern/inet/showcases" "-l" "/home/ilya-cavise/artery/scenarios/src/artery/envmod/libartery_envmod.so" "-l" "/home/ilya-cavise/artery/scenarios/src/artery/storyboard/libartery_storyboard.so" "-l" "/home/ilya-cavise/artery/scenarios/extern/libINET.so" "-l" "/home/ilya-cavise/artery/scenarios/extern/libveins.so" "-l" "/home/ilya-cavise/artery/scenarios/src/traci/libtraci.so" "-l" "/home/ilya-cavise/artery/scenarios/src/artery/libartery_core.so" "omnetpp.ini" "-uCmdenv" "-cinet_multiple_vehicle_types" "--sim-time-limit=20s")
set_tests_properties(example-inet-mixed-vehicles PROPERTIES  WORKING_DIRECTORY "/home/ilya-cavise/artery/scenarios/artery" _BACKTRACE_TRIPLES "/home/ilya-cavise/artery/cmake/AddOppRun.cmake;170;add_test;/home/ilya-cavise/artery/scenarios/artery/CMakeLists.txt;5;add_opp_test;/home/ilya-cavise/artery/scenarios/artery/CMakeLists.txt;0;")
add_test(example-inet-nakagami "/home/ilya-cavise/omnetpp/bin/opp_run_dbg" "-n" "/home/ilya-cavise/artery/src/artery:/home/ilya-cavise/artery/src/traci:/home/ilya-cavise/artery/extern/veins/examples/veins:/home/ilya-cavise/artery/extern/veins/src/veins:/home/ilya-cavise/artery/extern/inet/src:/home/ilya-cavise/artery/extern/inet/examples:/home/ilya-cavise/artery/extern/inet/tutorials:/home/ilya-cavise/artery/extern/inet/showcases" "-l" "/home/ilya-cavise/artery/scenarios/src/artery/envmod/libartery_envmod.so" "-l" "/home/ilya-cavise/artery/scenarios/src/artery/storyboard/libartery_storyboard.so" "-l" "/home/ilya-cavise/artery/scenarios/extern/libINET.so" "-l" "/home/ilya-cavise/artery/scenarios/extern/libveins.so" "-l" "/home/ilya-cavise/artery/scenarios/src/traci/libtraci.so" "-l" "/home/ilya-cavise/artery/scenarios/src/artery/libartery_core.so" "omnetpp.ini" "-uCmdenv" "-cinet_nakagami" "--sim-time-limit=20s")
set_tests_properties(example-inet-nakagami PROPERTIES  WORKING_DIRECTORY "/home/ilya-cavise/artery/scenarios/artery" _BACKTRACE_TRIPLES "/home/ilya-cavise/artery/cmake/AddOppRun.cmake;170;add_test;/home/ilya-cavise/artery/scenarios/artery/CMakeLists.txt;6;add_opp_test;/home/ilya-cavise/artery/scenarios/artery/CMakeLists.txt;0;")
add_test(example-inet-rsu "/home/ilya-cavise/omnetpp/bin/opp_run_dbg" "-n" "/home/ilya-cavise/artery/src/artery:/home/ilya-cavise/artery/src/traci:/home/ilya-cavise/artery/extern/veins/examples/veins:/home/ilya-cavise/artery/extern/veins/src/veins:/home/ilya-cavise/artery/extern/inet/src:/home/ilya-cavise/artery/extern/inet/examples:/home/ilya-cavise/artery/extern/inet/tutorials:/home/ilya-cavise/artery/extern/inet/showcases" "-l" "/home/ilya-cavise/artery/scenarios/src/artery/envmod/libartery_envmod.so" "-l" "/home/ilya-cavise/artery/scenarios/src/artery/storyboard/libartery_storyboard.so" "-l" "/home/ilya-cavise/artery/scenarios/extern/libINET.so" "-l" "/home/ilya-cavise/artery/scenarios/extern/libveins.so" "-l" "/home/ilya-cavise/artery/scenarios/src/traci/libtraci.so" "-l" "/home/ilya-cavise/artery/scenarios/src/artery/libartery_core.so" "omnetpp.ini" "-uCmdenv" "-cinet_rsu" "--sim-time-limit=20s")
set_tests_properties(example-inet-rsu PROPERTIES  WORKING_DIRECTORY "/home/ilya-cavise/artery/scenarios/artery" _BACKTRACE_TRIPLES "/home/ilya-cavise/artery/cmake/AddOppRun.cmake;170;add_test;/home/ilya-cavise/artery/scenarios/artery/CMakeLists.txt;7;add_opp_test;/home/ilya-cavise/artery/scenarios/artery/CMakeLists.txt;0;")
add_test(example-veins "/home/ilya-cavise/omnetpp/bin/opp_run_dbg" "-n" "/home/ilya-cavise/artery/src/artery:/home/ilya-cavise/artery/src/traci:/home/ilya-cavise/artery/extern/veins/examples/veins:/home/ilya-cavise/artery/extern/veins/src/veins:/home/ilya-cavise/artery/extern/inet/src:/home/ilya-cavise/artery/extern/inet/examples:/home/ilya-cavise/artery/extern/inet/tutorials:/home/ilya-cavise/artery/extern/inet/showcases" "-l" "/home/ilya-cavise/artery/scenarios/src/artery/envmod/libartery_envmod.so" "-l" "/home/ilya-cavise/artery/scenarios/src/artery/storyboard/libartery_storyboard.so" "-l" "/home/ilya-cavise/artery/scenarios/extern/libINET.so" "-l" "/home/ilya-cavise/artery/scenarios/extern/libveins.so" "-l" "/home/ilya-cavise/artery/scenarios/src/traci/libtraci.so" "-l" "/home/ilya-cavise/artery/scenarios/src/artery/libartery_core.so" "omnetpp.ini" "-uCmdenv" "-cveins" "--sim-time-limit=20s")
set_tests_properties(example-veins PROPERTIES  WORKING_DIRECTORY "/home/ilya-cavise/artery/scenarios/artery" _BACKTRACE_TRIPLES "/home/ilya-cavise/artery/cmake/AddOppRun.cmake;170;add_test;/home/ilya-cavise/artery/scenarios/artery/CMakeLists.txt;8;add_opp_test;/home/ilya-cavise/artery/scenarios/artery/CMakeLists.txt;0;")
add_test(example-veins-rsu "/home/ilya-cavise/omnetpp/bin/opp_run_dbg" "-n" "/home/ilya-cavise/artery/src/artery:/home/ilya-cavise/artery/src/traci:/home/ilya-cavise/artery/extern/veins/examples/veins:/home/ilya-cavise/artery/extern/veins/src/veins:/home/ilya-cavise/artery/extern/inet/src:/home/ilya-cavise/artery/extern/inet/examples:/home/ilya-cavise/artery/extern/inet/tutorials:/home/ilya-cavise/artery/extern/inet/showcases" "-l" "/home/ilya-cavise/artery/scenarios/src/artery/envmod/libartery_envmod.so" "-l" "/home/ilya-cavise/artery/scenarios/src/artery/storyboard/libartery_storyboard.so" "-l" "/home/ilya-cavise/artery/scenarios/extern/libINET.so" "-l" "/home/ilya-cavise/artery/scenarios/extern/libveins.so" "-l" "/home/ilya-cavise/artery/scenarios/src/traci/libtraci.so" "-l" "/home/ilya-cavise/artery/scenarios/src/artery/libartery_core.so" "omnetpp.ini" "-uCmdenv" "-cveins_rsu" "--sim-time-limit=20s")
set_tests_properties(example-veins-rsu PROPERTIES  WORKING_DIRECTORY "/home/ilya-cavise/artery/scenarios/artery" _BACKTRACE_TRIPLES "/home/ilya-cavise/artery/cmake/AddOppRun.cmake;170;add_test;/home/ilya-cavise/artery/scenarios/artery/CMakeLists.txt;9;add_opp_test;/home/ilya-cavise/artery/scenarios/artery/CMakeLists.txt;0;")
add_test(example-envmod "/home/ilya-cavise/omnetpp/bin/opp_run_dbg" "-n" "/home/ilya-cavise/artery/src/artery:/home/ilya-cavise/artery/src/traci:/home/ilya-cavise/artery/extern/veins/examples/veins:/home/ilya-cavise/artery/extern/veins/src/veins:/home/ilya-cavise/artery/extern/inet/src:/home/ilya-cavise/artery/extern/inet/examples:/home/ilya-cavise/artery/extern/inet/tutorials:/home/ilya-cavise/artery/extern/inet/showcases" "-l" "/home/ilya-cavise/artery/scenarios/src/artery/envmod/libartery_envmod.so" "-l" "/home/ilya-cavise/artery/scenarios/src/artery/storyboard/libartery_storyboard.so" "-l" "/home/ilya-cavise/artery/scenarios/extern/libINET.so" "-l" "/home/ilya-cavise/artery/scenarios/extern/libveins.so" "-l" "/home/ilya-cavise/artery/scenarios/src/traci/libtraci.so" "-l" "/home/ilya-cavise/artery/scenarios/src/artery/libartery_core.so" "omnetpp.ini" "-uCmdenv" "-cenvmod" "--sim-time-limit=20s")
set_tests_properties(example-envmod PROPERTIES  WORKING_DIRECTORY "/home/ilya-cavise/artery/scenarios/artery" _BACKTRACE_TRIPLES "/home/ilya-cavise/artery/cmake/AddOppRun.cmake;170;add_test;/home/ilya-cavise/artery/scenarios/artery/CMakeLists.txt;12;add_opp_test;/home/ilya-cavise/artery/scenarios/artery/CMakeLists.txt;0;")