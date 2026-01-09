# legged_sdk

This sdk is for motion control of Agile-PF(Agile Point-Foot biped robot) and other robots designed by the team of Prof. Yan Huang of Beijing Institute of Technology. 

The architecture is the same as [unitree_sdk2](https://github.com/unitreerobotics/unitree_sdk2). We adopt fast dds as the middleware because it's more friendly for secondary development and we can refer to ROS2 for massive examples.

Author: [Yasen Jia](https://github.com/lupinjia)

Email:  jason_1120202397@163.com

## Dependencies

- [spdlog](https://github.com/gabime/spdlog/tree/v1.x)
    
    install it through package managers or compilation
- [PREEMPT_RT](https://en.wikipedia.org/wiki/PREEMPT_RT)

    Because this lib is designed for real-time control of robots, PREEMPT_RT is required for compiling and executing.

After installing two components above, you can compile and install this lib:

```bash
mkdir build && cd build
# install to /opt/legged_sdk (recommended)
cmake .. -DCMAKE_INSTALL_PREFIX=/opt/legged_sdk
make
sudo make install
```

Then, you need to set library path before compiling test scripts or your own projects:

```bash
# If you have specified different CMAKE_INSTALL_PREFIX, 
# you need to change the LD_LIBRARY_PATH here.
export LD_LIBRARY_PATH=$LD_LIBRARY_PATH:/opt/legged_sdk/lib
```

## Structure

Structure of this project is largely inspired by [unitree_sdk2](https://github.com/unitreerobotics/unitree_sdk2/tree/main):

```bash
.
└── legged_sdk
    ├── CMakeLists.txt     # configurations for compiling and installing
    ├── src 
    ├── cmake
    │  └── legged_sdkConfig.cmake.in # file for generating legged_sdk2Config.cmake
    ├── include
        ├── common
            ├── crc.h               # calculation of crc (cyclic redundancy check)
            ├── legged_joystick.hpp # Abstraction of joystick data
            └── periodic_task.hpp   # Functions and classes for defining periodic tasks, using multi-threading of C++
        ├── dds
            ├── dds_publisher.hpp   # Realtime dds publisher
            └── dds_subscriber.hpp  # Realtime dds subscriber
        ├── dds_wrapper # wrapper for defining dds publisher and subscriber easily
            ├── common
                └── publisher.h     # Realtime publisher with mutex that will publish messages automatically once the lock is released
            └── robot
                └── agilepf
                   └── agilepf_pub.h # Lazy publisher definitions for publishing messages easily
        ├── idl
            ├── agilepf # messages used in agilepf
            └── HelloWorld # Testing message
    ├── test       # test scripts
    └── thirdparty # header files and libs of fastdds, fastcdr and foonathan_memory

```

## Add robots to this sdk

If you want to add support for other robots, you need to complete idl and dds_wrapper.

Under idl directory, you need to create a new folder named by your robot and add your desired idl files. To generate cxx codes from idl files, you need to install [fast dds and fast dds gen](https://fast-dds.docs.eprosima.com/en/stable/installation/sources/sources_linux.html) first.

Then you need to complete the dds_wrapper of your robot by creating a folder named by your robot under `dds_wrapper/robot` and appending files in it.

## Acknowledgements

- [unitree_sdk2](https://github.com/unitreerobotics/unitree_sdk2/tree/main)

- [fast dds](https://github.com/eProsima/Fast-DDS)

