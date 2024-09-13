message(STATUS "[EIS2] test targets enabled")

add_executable (dd99_eis2_scratch_test_01           EXCLUDE_FROM_ALL       scratch/01.cpp)
set_target_warnings(dd99_eis2_scratch_test_01       PRIVATE)
target_link_libraries(dd99_eis2_scratch_test_01         ${package_name}::${component_name})
