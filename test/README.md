# Test scripts for agilepf_sdk

## test_periodic_task

Test the functionality of PeriodicTask class in terms of average period time, max period time to test the accuracy and jitter of PeriodicTask.

You should install agilepf_sdk under `/opt/agilepf_sdk` to directly compile and execute this project. Or you should modify the `CMAKE_PREFIX_PATH` and `include_directories` in CMakeLists.txt.

## test_unitree_thread

This test is to compare our implementation of periodic_task with that of [unitree_sdk2](https://github.com/unitreerobotics/unitree_sdk2). Most of the code for this test is the same as `test_periodic_task` except that the utilized functions to create the periodic task are from [unitree_sdk2](https://github.com/unitreerobotics/unitree_sdk2).

You should first install unitree_sdk2 under `/opt/unitree_robotics` before compiling and executing this test.

> Make sure that you run them as sudo when you run `test_periodic_task` and `test_unitree_thread`. Or the program cannot set policy and cpu affinity correctly.

## test_dds_pubsub

Test the functionality of publishing and subscribing data through dds, with fast-dds. By default, the network interface is "lo". You can specify another interface by appending the name after `./test_dds_pub`.

```bash
# open a terminal and run the following cmd
./test_dds_pub interface_name 
# in another terminal
./test_dds_sub interface_name
```

You should see the message printed to the terminal every second.

test_low_state_sub is for testing LowState subcription from the upper board of the robot. You should see information like below when you enable publication and subscription:

```bash
[2026-01-11 15:53:33.733] [info] LowState message received.
[2026-01-11 15:53:33.733] [info] Quaternion: [0.99945647, 0.000135151, -0.032912523, 0.0048236223]
[2026-01-11 15:53:33.733] [info] Motor Pos: [0.33207417, 1.1938276, -1.9811935, -0.3164339, 1.2170973, -1.9838638]
```

# Tips
You should install agilepf_sdk under `/opt/agilepf_sdk` to directly compile and execute this project. Or you should modify the `CMAKE_PREFIX_PATH` and `include_directories` in CMakeLists.txt.