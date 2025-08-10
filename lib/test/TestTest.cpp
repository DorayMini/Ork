#include <gtest/gtest.h>
#include "graphl.h"

TEST(Test, Case1) {
  graphl g;
  EXPECT_EQ(g.test(), 2);
}
