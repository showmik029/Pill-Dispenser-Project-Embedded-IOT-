# Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
# file Copyright.txt or https://cmake.org/licensing for details.

cmake_minimum_required(VERSION 3.5)

file(MAKE_DIRECTORY
  "C:/bin/PicoSDKv1.5.0/pico-sdk/tools/pioasm"
  "E:/Embedded_C_IOT/Pill_dispenser_project/cmake-build-debug/pioasm"
  "E:/Embedded_C_IOT/Pill_dispenser_project/cmake-build-debug/pico-sdk/src/rp2_common/tinyusb/pioasm"
  "E:/Embedded_C_IOT/Pill_dispenser_project/cmake-build-debug/pico-sdk/src/rp2_common/tinyusb/pioasm/tmp"
  "E:/Embedded_C_IOT/Pill_dispenser_project/cmake-build-debug/pico-sdk/src/rp2_common/tinyusb/pioasm/src/PioasmBuild-stamp"
  "E:/Embedded_C_IOT/Pill_dispenser_project/cmake-build-debug/pico-sdk/src/rp2_common/tinyusb/pioasm/src"
  "E:/Embedded_C_IOT/Pill_dispenser_project/cmake-build-debug/pico-sdk/src/rp2_common/tinyusb/pioasm/src/PioasmBuild-stamp"
)

set(configSubDirs )
foreach(subDir IN LISTS configSubDirs)
    file(MAKE_DIRECTORY "E:/Embedded_C_IOT/Pill_dispenser_project/cmake-build-debug/pico-sdk/src/rp2_common/tinyusb/pioasm/src/PioasmBuild-stamp/${subDir}")
endforeach()
if(cfgdir)
  file(MAKE_DIRECTORY "E:/Embedded_C_IOT/Pill_dispenser_project/cmake-build-debug/pico-sdk/src/rp2_common/tinyusb/pioasm/src/PioasmBuild-stamp${cfgdir}") # cfgdir has leading slash
endif()
