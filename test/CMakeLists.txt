SET(unit_tester_src Tests.cpp)

enable_testing() 
find_package(GTest REQUIRED) 
include_directories(${GTEST_INCLUDE_DIRS}) 

add_executable(alltests ${unit_tester_src}) 
target_link_libraries(alltests ${GTEST_LIBRARIES} pthread)
add_test(AllTestsForEachItr alltests) 

SET(test_data data/rock-n-roll-nerd)
configure_file(${test_data} ${CMAKE_CURRENT_BINARY_DIR}/${test_data} COPYONLY)