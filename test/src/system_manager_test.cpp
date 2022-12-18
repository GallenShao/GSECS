/*
 * Copyright (c) 2022 by GallenShao, All Rights Reserved.
 *
 * system_manager_test.cpp
 *
 *  Created on: 2022.05.14
 *  Author: gallenshao
 */

#include <gtest/gtest.h>

#include "gs_ecs.h"

thread_local std::string thread_name = "default";
thread_local int thread_index = 0;

bool A_called = false;
bool B_called = false;
bool C_called = false;
bool D_called = false;
bool E_called = false;
bool F_called = false;

template <class T>
class ThreadChecker {
 public:
  static void SetExpectedThread(const std::string& name) {
    expected_thread_ = name;
  }

  void CheckThread() {
    EXPECT_EQ(expected_thread_, thread_name);
  }
 private:
  static std::string expected_thread_;
};
template <class T>
std::string ThreadChecker<T>::expected_thread_ = "default";

class ASystem : public gs::System<ASystem>, public ThreadChecker<ASystem> {
 public:
  void Update(gs::EntityManager& manager) override {
    CheckThread();
    assert(E_called);
    A_called = true;
    std::cout << "ASystem runs on " << thread_name << thread_index << std::endl;
  }
};
class BSystem : public gs::System<BSystem>, public ThreadChecker<BSystem> {
 public:
  void Update(gs::EntityManager& manager) override {
    CheckThread();
    assert(E_called);
    B_called = true;
    std::cout << "BSystem runs on " << thread_name << thread_index << std::endl;
  }
};
class CSystem : public gs::System<CSystem>, public ThreadChecker<CSystem> {
 public:
  void Update(gs::EntityManager& manager) override {
    CheckThread();
    assert(E_called);
    assert(A_called);
    C_called = true;
    std::cout << "CSystem runs on " << thread_name << thread_index << std::endl;
  }
};
class DSystem : public gs::System<DSystem>, public ThreadChecker<DSystem> {
 public:
  void Update(gs::EntityManager& manager) override {
    CheckThread();
    assert(B_called);
    assert(C_called);
    assert(E_called);
    D_called = true;
    std::cout << "DSystem runs on " << thread_name << thread_index << std::endl;
  }
};
class ESystem : public gs::System<ESystem>, public ThreadChecker<ESystem> {
 public:
  void Update(gs::EntityManager& manager) override {
    CheckThread();
    E_called = true;
    std::cout << "ESystem runs on " << thread_name << thread_index << std::endl;
  }
};
class FSystem : public gs::System<FSystem>, public ThreadChecker<FSystem> {
 public:
  void Update(gs::EntityManager& manager) override {
    CheckThread();
    assert(B_called);
    assert(C_called);
    assert(E_called);
    F_called = true;
    std::cout << "FSystem runs on " << thread_name << thread_index << std::endl;
  }
};

static int dummy_thread_count = 0;
class DummyThread : public gs::SystemThread<DummyThread> {
 public:
  void OnInit() override {
    std::cout << "DummyThread OnInit" << std::endl;
    thread_name = "DummyThread";
    thread_index = dummy_thread_count++;
  }
  void OnDestroy() override {
    std::cout << "DummyThread OnDestroy" << std::endl;
    thread_name = "clear";
    dummy_thread_count--;
  }
};

static int open_gl_thread_count = 0;
class OpenGLThread : public gs::SystemThread<OpenGLThread> {
 public:
  void OnInit() override {
    std::cout << "OpenGLThread OnInit" << std::endl;
    thread_name = "OpenGLThread";
    thread_index = open_gl_thread_count++;
  }
  void OnDestroy() override {
    std::cout << "OpenGLThread OnDestroy" << std::endl;
    thread_name = "clear";
    open_gl_thread_count--;
  }
};

void ResetTest() {
  A_called = false;
  B_called = false;
  C_called = false;
  D_called = false;
  E_called = false;
  F_called = false;
  dummy_thread_count = 0;
  open_gl_thread_count = 0;
}

TEST(SystemManagerTest, SingleThreadTraverser) {
  ResetTest();
  auto manager = gs::SystemManager::MakeFromTraverser<gs::SingleThreadTraverser>();
  manager->AddSystem<ESystem>();
  manager->AddSystem<ASystem>();
  manager->AddSystem<BSystem>();
  manager->AddSystem<CSystem>().WhichDependsOn<ASystem>();
  manager->AddSystem<DSystem>().WhichDependsOn<BSystem>().And<CSystem>();
  manager->AddSystem<FSystem>().WhichDependsOn<CSystem>().And<ESystem>();

  gs::EntityManager dummy;
  manager->Update(dummy);
}

//  group_0
//        ----------
//        | A -> C | \    / --> D
// E ---> |        |  ----
//  \     | B      | /    \
//   \    ----------       ---> F
//    \                   /
//     -------------------
TEST(SystemManagerTest, MultiThreadTraverser) {
  ResetTest();
  auto manager = gs::SystemManager::MakeFromTraverser<gs::MultiThreadTraverser>();
  manager->AddSystem<ESystem>().WithThread<DummyThread>();
  ESystem::SetExpectedThread("DummyThread");

  gs::SystemGroup group_0;
  group_0.AddSystem<ASystem>();
  group_0.AddSystem<BSystem>();
  group_0.AddSystem<CSystem>().WhichDependsOn<ASystem>();
  manager->AddSystemGroup(group_0).WithThread<OpenGLThread>().WhichDependsOn<ESystem>();
  ASystem::SetExpectedThread("OpenGLThread");
  BSystem::SetExpectedThread("OpenGLThread");
  CSystem::SetExpectedThread("OpenGLThread");

  manager->AddSystem<DSystem>().WhichDependsOn(group_0);
  manager->AddSystem<FSystem>().WhichDependsOn(group_0).And<ESystem>();

  manager->SetMaxThreadCount(4);
  manager->SetMaxThreadCount<DummyThread>(2);
  manager->SetMaxThreadCount<OpenGLThread>(2);

  gs::EntityManager dummy;
  manager->Update(dummy);
  EXPECT_GT(dummy_thread_count, 0);
  EXPECT_LE(dummy_thread_count, 2);
  EXPECT_GT(open_gl_thread_count, 0);
  EXPECT_LE(open_gl_thread_count, 2);

  manager = nullptr;
  EXPECT_EQ(dummy_thread_count, 0);
  EXPECT_EQ(open_gl_thread_count, 0);
}

//  group_0
//        ----------
//        | A -> C | \    / --> D
// E ---> |        |  ----
//  \     | B      | /    \
//   \    ----------       ---> F
//    \                   /
//     -------------------
TEST(SystemManagerTest, MultiThreadTraverser2) {
  ResetTest();
  auto manager = gs::SystemManager::MakeFromTraverser<gs::MultiThreadTraverser>();
  manager->AddSystem<ESystem>().WithThread<DummyThread>();
  ESystem::SetExpectedThread("DummyThread");

  gs::SystemGroup group_0;
  group_0.AddSystem<ASystem>();
  group_0.AddSystem<BSystem>();
  group_0.AddSystem<CSystem>().WhichDependsOn<ASystem>();
  manager->AddSystemGroup(group_0).WithThread<OpenGLThread>().WhichDependsOn<ESystem>();
  ASystem::SetExpectedThread("OpenGLThread");
  BSystem::SetExpectedThread("OpenGLThread");
  CSystem::SetExpectedThread("OpenGLThread");

  manager->AddSystem<DSystem>().WhichDependsOn(group_0);
  manager->AddSystem<FSystem>().WhichDependsOn(group_0).And<ESystem>();

  manager->SetMaxThreadCount(4);
  manager->SetMaxThreadCount<DummyThread>(2);
  manager->SetMaxThreadCount<OpenGLThread>(1);

  gs::EntityManager dummy;
  manager->Update(dummy);
  EXPECT_GT(dummy_thread_count, 0);
  EXPECT_LE(dummy_thread_count, 2);
  EXPECT_GT(open_gl_thread_count, 0);
  EXPECT_LE(open_gl_thread_count, 1);

  manager = nullptr;
  EXPECT_EQ(dummy_thread_count, 0);
  EXPECT_EQ(open_gl_thread_count, 0);
}

class CountThread : public gs::SystemThread<CountThread> {};

static int count;
template <int T>
class CountSystem : public gs::System<CountSystem<T>> {
 public:
  void Update(gs::EntityManager& manager) override {
    count++;
  }
};

// thread test
TEST(SystemManagerTest, MultiThreadTraverser3) {
  count = 0;
  for (int i = 0; i < 1000; i++) {
    auto manager = gs::SystemManager::MakeFromTraverser<gs::MultiThreadTraverser>();

    manager->AddSystem<CountSystem<0>>().WithThread<CountThread>();
    manager->AddSystem<CountSystem<1>>().WithThread<CountThread>();
    manager->AddSystem<CountSystem<2>>().WithThread<CountThread>();
    manager->AddSystem<CountSystem<3>>().WithThread<CountThread>();
    manager->AddSystem<CountSystem<4>>().WithThread<CountThread>();
    manager->AddSystem<CountSystem<5>>().WithThread<CountThread>();
    manager->AddSystem<CountSystem<6>>().WithThread<CountThread>();
    manager->AddSystem<CountSystem<7>>().WithThread<CountThread>();
    manager->AddSystem<CountSystem<8>>().WithThread<CountThread>();
    manager->AddSystem<CountSystem<9>>().WithThread<CountThread>();
    manager->AddSystem<CountSystem<10>>().WithThread<CountThread>();
    manager->AddSystem<CountSystem<11>>().WithThread<CountThread>();
    manager->AddSystem<CountSystem<12>>().WithThread<CountThread>();
    manager->AddSystem<CountSystem<13>>().WithThread<CountThread>();
    manager->AddSystem<CountSystem<14>>().WithThread<CountThread>();
    manager->AddSystem<CountSystem<15>>().WithThread<CountThread>();
    manager->AddSystem<CountSystem<16>>().WithThread<CountThread>();
    manager->AddSystem<CountSystem<17>>().WithThread<CountThread>();
    manager->AddSystem<CountSystem<18>>().WithThread<CountThread>();
    manager->AddSystem<CountSystem<19>>().WithThread<CountThread>();

    manager->SetMaxThreadCount<CountThread>(3);

    gs::EntityManager dummy;
    manager->Update(dummy);
  }
  std::cout << "count: " << count << std::endl;
  // There is a high probability that thread conflicts will occur, resulting in `count` is less than expected
  EXPECT_LT(count, 1000 * 20);
}

// thread test
TEST(SystemManagerTest, MultiThreadTraverser4) {
  count = 0;
  for (int i = 0; i < 1000; i++) {
    auto manager = gs::SystemManager::MakeFromTraverser<gs::MultiThreadTraverser>();

    manager->AddSystem<CountSystem<0>>().WithThread<CountThread>();
    manager->AddSystem<CountSystem<1>>().WithThread<CountThread>();
    manager->AddSystem<CountSystem<2>>().WithThread<CountThread>();
    manager->AddSystem<CountSystem<3>>().WithThread<CountThread>();
    manager->AddSystem<CountSystem<4>>().WithThread<CountThread>();
    manager->AddSystem<CountSystem<5>>().WithThread<CountThread>();
    manager->AddSystem<CountSystem<6>>().WithThread<CountThread>();
    manager->AddSystem<CountSystem<7>>().WithThread<CountThread>();
    manager->AddSystem<CountSystem<8>>().WithThread<CountThread>();
    manager->AddSystem<CountSystem<9>>().WithThread<CountThread>();
    manager->AddSystem<CountSystem<10>>().WithThread<CountThread>();
    manager->AddSystem<CountSystem<11>>().WithThread<CountThread>();
    manager->AddSystem<CountSystem<12>>().WithThread<CountThread>();
    manager->AddSystem<CountSystem<13>>().WithThread<CountThread>();
    manager->AddSystem<CountSystem<14>>().WithThread<CountThread>();
    manager->AddSystem<CountSystem<15>>().WithThread<CountThread>();
    manager->AddSystem<CountSystem<16>>().WithThread<CountThread>();
    manager->AddSystem<CountSystem<17>>().WithThread<CountThread>();
    manager->AddSystem<CountSystem<18>>().WithThread<CountThread>();
    manager->AddSystem<CountSystem<19>>().WithThread<CountThread>();

    manager->SetMaxThreadCount<CountThread>(1);

    gs::EntityManager dummy;
    manager->Update(dummy);
  }
  std::cout << "count: " << count << std::endl;
  // If there is only one CountThread, there will be no thread conflict.
  EXPECT_EQ(count, 1000 * 20);
}