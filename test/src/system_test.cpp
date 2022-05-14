/*
 * Copyright (c) 2022 by GallenShao, All Rights Reserved.
 *
 * system_test.cpp
 *
 *  Created on: 2022.05.14
 *  Author: gallenshao
 */

#include <gtest/gtest.h>

#include "gs_ecs.h"
#include "gs_ecs_test_header.h"

TEST(SystemGroupTest, SimpleSystemGroup) {
  gs::SystemGroup manager;
  manager.AddSystem<ASystem>();
  manager.AddSystem<BSystem>();
  manager.AddSystem<CSystem>().WhichDependsOn<ASystem>();
  manager.AddSystem<DSystem>().WhichDependsOn<BSystem>().And<CSystem>();
  manager.AddSystem<ESystem>();
  manager.AddSystem<FSystem>().WhichDependsOn<CSystem>().And<ESystem>();
}

// Add an existed system
TEST(SystemGroupTest, SystemGroupWrongUsage1) {
  gs::SystemGroup manager;
  manager.AddSystem<ASystem>();
  EXPECT_DEATH(manager.AddSystem<ASystem>(), "");
}

// depends on an non-existed system in function `WhichDependsOn`
TEST(SystemGroupTest, SystemGroupWrongUsage2) {
  gs::SystemGroup manager;
  EXPECT_DEATH(manager.AddSystem<ASystem>().WhichDependsOn<BSystem>(), "");
}

// depends on self in function `WhichDependsOn`
TEST(SystemGroupTest, SystemGroupWrongUsage3) {
  gs::SystemGroup manager;
  EXPECT_DEATH(manager.AddSystem<ASystem>().WhichDependsOn<ASystem>(), "");
}

// depends on an non-existed system in function `And`
TEST(SystemGroupTest, SystemGroupWrongUsage4) {
  gs::SystemGroup manager;
  manager.AddSystem<ASystem>();
  EXPECT_DEATH(manager.AddSystem<BSystem>().WhichDependsOn<ASystem>().And<CSystem>(), "");
}

// depends on self in function `And`
TEST(SystemGroupTest, SystemGroupWrongUsage5) {
  gs::SystemGroup manager;
  manager.AddSystem<ASystem>();
  EXPECT_DEATH(manager.AddSystem<BSystem>().WhichDependsOn<ASystem>().And<BSystem>(), "");
}

TEST(SystemGroupTest, GroupedSystemGroup) {
  gs::SystemGroup manager;
  gs::SystemGroup group_0;
  group_0.AddSystem<ASystem>();
  group_0.AddSystem<BSystem>().WhichDependsOn<ASystem>();
  manager.AddSystemGroup(group_0);
  gs::SystemGroup group_1;
  group_1.AddSystem<CSystem>();
  group_1.AddSystem<DSystem>();
  manager.AddSystemGroup(group_1).WhichDependsOn(group_0);
}

// Add an existed system group
TEST(SystemGroupTest, GroupedSystemGroupWrongUsage1) {
  gs::SystemGroup manager;
  gs::SystemGroup group_0;
  group_0.AddSystem<ASystem>();
  manager.AddSystemGroup(group_0);
  EXPECT_DEATH(manager.AddSystemGroup(group_0), "");
}

// Edit system group after add
TEST(SystemGroupTest, GroupedSystemGroupWrongUsage2) {
  gs::SystemGroup manager;
  gs::SystemGroup group_0;
  group_0.AddSystem<ASystem>();
  manager.AddSystemGroup(group_0);
  EXPECT_DEATH(group_0.AddSystem<ASystem>(), "");
}

// Add group which belongs to an other group
TEST(SystemGroupTest, GroupedSystemGroupWrongUsage3) {
  gs::SystemGroup manager;
  gs::SystemGroup group_0;
  group_0.AddSystem<ASystem>();
  manager.AddSystemGroup(group_0);
  gs::SystemGroup group_1;
  EXPECT_DEATH(group_1.AddSystemGroup(group_0), "");
}

// Add group which has duplicate systems
TEST(SystemGroupTest, GroupedSystemGroupWrongUsage4) {
  gs::SystemGroup manager;
  manager.AddSystem<ASystem>();
  gs::SystemGroup group_0;
  group_0.AddSystem<ASystem>();
  EXPECT_DEATH(manager.AddSystemGroup(group_0), "");
}

// Add self
TEST(SystemGroupTest, GroupedSystemGroupWrongUsage5) {
  gs::SystemGroup manager;
  manager.AddSystem<ASystem>();
  EXPECT_DEATH(manager.AddSystemGroup(manager), "");
}

// depends on an non-existed group in function `WhichDependsOn`
TEST(SystemGroupTest, GroupedSystemGroupWrongUsage6) {
  gs::SystemGroup manager;
  gs::SystemGroup group_0;
  group_0.AddSystem<BSystem>();
  EXPECT_DEATH(manager.AddSystem<ASystem>().WhichDependsOn(group_0), "");
}

// depends on self in function `WhichDependsOn`
TEST(SystemGroupTest, GroupedSystemGroupWrongUsage7) {
  gs::SystemGroup manager;
  EXPECT_DEATH(manager.AddSystem<ASystem>().WhichDependsOn(manager), "");
}

// depends on an non-existed group in function `And`
TEST(SystemGroupTest, GroupedSystemGroupWrongUsage8) {
  gs::SystemGroup manager;
  manager.AddSystem<ASystem>();
  gs::SystemGroup group_0;
  group_0.AddSystem<BSystem>();
  EXPECT_DEATH(manager.AddSystem<CSystem>().WhichDependsOn<ASystem>().And(group_0), "");
}

// depends on self in function `And`
TEST(SystemGroupTest, GroupedSystemGroupWrongUsage9) {
  gs::SystemGroup manager;
  manager.AddSystem<ASystem>();
  EXPECT_DEATH(manager.AddSystem<BSystem>().WhichDependsOn<ASystem>().And(manager), "");
}

// add empty group
TEST(SystemGroupTest, GroupedSystemGroupWrongUsage10) {
  gs::SystemGroup manager;
  gs::SystemGroup group_0;
  EXPECT_DEATH(manager.AddSystemGroup(group_0), "");
}

// depends on self in function `WhichDependsOn`
TEST(SystemGroupTest, GroupedSystemGroupWrongUsage11) {
  gs::SystemGroup manager;
  gs::SystemGroup group_0;
  group_0.AddSystem<ASystem>();
  EXPECT_DEATH(manager.AddSystemGroup(group_0).WhichDependsOn(group_0), "");
}

// depends on self in function `And`
TEST(SystemGroupTest, GroupedSystemGroupWrongUsage12) {
  gs::SystemGroup manager;
  gs::SystemGroup group_0;
  group_0.AddSystem<ASystem>();
  manager.AddSystem<BSystem>();
  EXPECT_DEATH(manager.AddSystemGroup(group_0).WhichDependsOn<BSystem>().And(group_0), "");
}