#include <gtest/gtest.h>

#include "test/TestDecisionTree.h"

#include "test/TestCGEvaluationPath.h"
#include "test/TestCGEvaluationPathsBuilder.h"
#include "test/TestCGConditionVectorVariationsBuilder.h"

#include "test/TestSingleCodegenL1.h"
#include "test/TestSingleCodegenL2.h"
#include "test/TestSingleCodegenL3.h"
#include "test/TestSingleCodegenL4.h"
#include "test/TestSingleCodegenL6.h"

#include "test/TestMixedCodegenL2.h"
#include "test/TestMixedCodegenL3.h"
#include "test/TestMixedCodegenL4.h"
#include "test/TestMixedCodegenL5.h"

int main(int argc, char **argv) {
  ::testing::InitGoogleTest(&argc, argv);

  /*
  ::testing::GTEST_FLAG(filter) =
      "MixedCodegenL2.*:MixedCodegenL3.*:MixedCodegenL4";
  //    ":LXSubtreeSwitch3.PerfectTrivialGradientTree";
  //*/

  return RUN_ALL_TESTS();
}
