# Additional clean files
cmake_minimum_required(VERSION 3.16)

if("${CONFIG}" STREQUAL "" OR "${CONFIG}" STREQUAL "Debug")
  file(REMOVE_RECURSE
  "CMakeFiles/DNA-v32_autogen.dir/AutogenUsed.txt"
  "CMakeFiles/DNA-v32_autogen.dir/ParseCache.txt"
  "DNA-v32_autogen"
  )
endif()
