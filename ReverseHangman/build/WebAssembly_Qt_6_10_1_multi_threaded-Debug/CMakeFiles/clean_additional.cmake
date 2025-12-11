# Additional clean files
cmake_minimum_required(VERSION 3.16)

if("${CONFIG}" STREQUAL "" OR "${CONFIG}" STREQUAL "Debug")
  file(REMOVE_RECURSE
  "CMakeFiles\\ReverseHangman_autogen.dir\\AutogenUsed.txt"
  "CMakeFiles\\ReverseHangman_autogen.dir\\ParseCache.txt"
  "ReverseHangman_autogen"
  )
endif()
