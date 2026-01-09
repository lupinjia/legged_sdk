# Test scripts for agilepf_sdk

## test_periodic_task

Test the functionality of PeriodicTask class in terms of average period time, max period time to test the accuracy and jitter of PeriodicTask.

You should install agilepf_sdk under `/opt/agilepf_sdk` to directly compile and execute this project. Or you should modify the `CMAKE_PREFIX_PATH` and `include_directories` in CMakeLists.txt.

## test_unitree_thread

This test is to compare our implementation of periodic_task with that of [unitree_sdk2](https://github.com/unitreerobotics/unitree_sdk2). Most of the code for this test is the same as `test_periodic_task` except that the utilized functions to create the periodic task are from [unitree_sdk2](https://github.com/unitreerobotics/unitree_sdk2).

You should first install unitree_sdk2 under `/opt/unitree_robotics` before compiling and executing this test.

> Make sure that you run them as sudo when you run `test_periodic_task` and `test_unitree_thread`. Or the program cannot set policy and cpu affinity correctly.

## test_dds_pubsub

Test the functionality of publishing and subscribing data through dds, with fast-dds.

You should install agilepf_sdk under `/opt/agilepf_sdk` to directly compile and execute this project. Or you should modify the `CMAKE_PREFIX_PATH` and `include_directories` in CMakeLists.txt.

Open a terminal and run `./test_dds_pub`, then run `./test_dds_sub` in another terminal and you will see the message printed to the terminal every second.