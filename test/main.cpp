/*
 * Copyright (c) 2021 by GallenShao, All Rights Reserved.
 *
 * main.cpp
 *
 *  Created on: 2021.09.20
 *  Author: gallenshao
 */

#include <gtest/gtest.h>

namespace gs::test {

class Environment : public ::testing::Environment {
 public:
  ~Environment() override = default;

  // Override this to define how to set up the environment.
  void SetUp() override {
  }

  // Override this to define how to tear down the environment.
  void TearDown() override {
  }
};

}  // namespace gs::test

int main(int argc, char **argv) {
  ::testing::InitGoogleTest(&argc, argv);
  testing::AddGlobalTestEnvironment(new gs::test::Environment());
  return RUN_ALL_TESTS();
}