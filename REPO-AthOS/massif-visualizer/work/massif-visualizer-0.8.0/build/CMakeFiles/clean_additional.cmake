# Additional clean files
cmake_minimum_required(VERSION 3.16)

if("${CONFIG}" STREQUAL "" OR "${CONFIG}" STREQUAL "RelWithDebInfo")
  file(REMOVE_RECURSE
  "app/CMakeFiles/massif-visualizer_autogen.dir/AutogenUsed.txt"
  "app/CMakeFiles/massif-visualizer_autogen.dir/ParseCache.txt"
  "app/massif-visualizer_autogen"
  "massifdata/CMakeFiles/mv-massifdata_autogen.dir/AutogenUsed.txt"
  "massifdata/CMakeFiles/mv-massifdata_autogen.dir/ParseCache.txt"
  "massifdata/mv-massifdata_autogen"
  "test/CMakeFiles/datamodeltest_autogen.dir/AutogenUsed.txt"
  "test/CMakeFiles/datamodeltest_autogen.dir/ParseCache.txt"
  "test/CMakeFiles/massifparser_autogen.dir/AutogenUsed.txt"
  "test/CMakeFiles/massifparser_autogen.dir/ParseCache.txt"
  "test/datamodeltest_autogen"
  "test/massifparser_autogen"
  "visualizer/CMakeFiles/mv-visualizer_autogen.dir/AutogenUsed.txt"
  "visualizer/CMakeFiles/mv-visualizer_autogen.dir/ParseCache.txt"
  "visualizer/mv-visualizer_autogen"
  )
endif()
