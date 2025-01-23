# Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
# file Copyright.txt or https://cmake.org/licensing for details.

cmake_minimum_required(VERSION 3.5)

file(MAKE_DIRECTORY
  "C:/bin/PicoSDKv1.5.0/pico-sdk/tools/elf2uf2"
  "E:/Embedded_C_IOT/Pill_despenser_project/cmake-build-debug/elf2uf2"
  "E:/Embedded_C_IOT/Pill_despenser_project/cmake-build-debug/elf2uf2"
  "E:/Embedded_C_IOT/Pill_despenser_project/cmake-build-debug/elf2uf2/tmp"
  "E:/Embedded_C_IOT/Pill_despenser_project/cmake-build-debug/elf2uf2/src/ELF2UF2Build-stamp"
  "E:/Embedded_C_IOT/Pill_despenser_project/cmake-build-debug/elf2uf2/src"
  "E:/Embedded_C_IOT/Pill_despenser_project/cmake-build-debug/elf2uf2/src/ELF2UF2Build-stamp"
)

set(configSubDirs )
foreach(subDir IN LISTS configSubDirs)
    file(MAKE_DIRECTORY "E:/Embedded_C_IOT/Pill_despenser_project/cmake-build-debug/elf2uf2/src/ELF2UF2Build-stamp/${subDir}")
endforeach()
if(cfgdir)
  file(MAKE_DIRECTORY "E:/Embedded_C_IOT/Pill_despenser_project/cmake-build-debug/elf2uf2/src/ELF2UF2Build-stamp${cfgdir}") # cfgdir has leading slash
endif()
