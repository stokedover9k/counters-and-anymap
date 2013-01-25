#define __COUNTER_DEBUG__ on


#include "gtest/gtest.h"

#include "AnyMapTests.hpp"
#include "CounterTests.hpp"
#include "CounterMapTests.hpp"



int main(int argc, char **argv)
{
  testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
