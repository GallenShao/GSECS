/**
 * Copyright 2021 by GallenShao, All Rights Reserved.
 *
 * system.h
 *
 *  Created on: 2022.05.04
 *  Author: gallenshao
 */

#pragma once

#include "entity.h"
#include "thread.h"
#include <bitset>
#include <cassert>
#include <cstdint>
#include <functional>
#include <memory>
#include <mutex>
#include <set>
#include <thread>
#include <utility>
#include <vector>

#define GS_DEBUG true

#if !GS_DEBUG
#define assert(e)
#endif

/**
 * ECS的System部分实现，功能：
 *   1. System支持多线程运行
 *   2. System支持绑定指定类型的Thread，Thread可自定义初始化、销毁函数
 *   3. 支持自定义System执行时依赖
 *
 * 关键对外类：SystemManager、SystemGroup、System、SystemThread
 */
namespace gs {

#define MAX_SYSTEM_COUNT 256

class SystemTraverser;
class SystemGroup;
class SystemGroupBuilder;
class SystemGroupBuilderItem;

class BaseSystem : public std::enable_shared_from_this<BaseSystem> {
 public:
  typedef int Family;
  virtual void Configure(EntityManager& manager) {}
  virtual void Update(EntityManager& manager) {}

 protected:
  static Family family_count_;
  virtual Family GetFamily() = 0;

 private:
  std::bitset<MAX_SYSTEM_COUNT> dependencies_;
  std::set<Family> next_;
  SystemThreadBase::Family initializer_family_ = DefaultThread::family();

  friend class SingleThreadTraverser;
  friend class MultiThreadTraverser;
  friend class SystemManager;
  friend class SystemGroupBuilder;
  friend class SystemGroupBuilderItem;
};

/**
 * Example:
 *
 * class ASystem : public gs::System<ASystem> {
 *   void Update(gs::EntityManager& manager) override {
 *     std::cout << "ASystem Update" << std::endl;
 *   }
 * };
 */
template <typename T>
class System : public BaseSystem {
 public:
  static Family family();
  Family GetFamily() override;
};

class SystemGroupBuilderItem {
 public:
  template <typename T>
  typename std::enable_if<std::is_base_of<System<T>, T>::value, SystemGroupBuilderItem>::type And();
  SystemGroupBuilderItem And(SystemGroup& group);

 private:
  SystemGroupBuilderItem(SystemGroup* group, std::set<BaseSystem::Family>& current)
      : group_(group), current_(std::move(current)) {}

  SystemGroup* group_;
  std::set<BaseSystem::Family> current_;

  friend class SystemGroupBuilder;
};

class SystemGroupBuilder {
 public:
  template <typename T>
  typename std::enable_if<std::is_base_of<System<T>, T>::value, SystemGroupBuilderItem>::type WhichDependsOn();
  SystemGroupBuilderItem WhichDependsOn(SystemGroup& group);

  template <typename T>
  typename std::enable_if<std::is_base_of<SystemThread<T>, T>::value, SystemGroupBuilder>::type WithThread();

 private:
  SystemGroupBuilder(SystemGroup* group, std::set<BaseSystem::Family>& current)
      : group_(group), current_(std::move(current)) {}

  SystemGroup* group_;
  std::set<BaseSystem::Family> current_;

  friend class SystemGroup;
};

/**
 * Example：
 *
 * gs::SystemGroup group;
 * group.AddSystem<ASystem>();
 * group.AddSystem<BSystem>();
 * group.AddSystem<CSystem>().WhichDependsOn<BSystem>();
 */
class SystemGroup {
 public:
  template <typename T>
  typename std::enable_if<std::is_base_of<System<T>, T>::value, SystemGroupBuilder>::type AddSystem();

  SystemGroupBuilder AddSystemGroup(SystemGroup& group);

 protected:
  bool editable_ = true;
  std::vector<std::shared_ptr<BaseSystem>> all_systems_;
  std::bitset<MAX_SYSTEM_COUNT> all_systems_mask_;
  std::set<BaseSystem::Family> start_node_families_;

  std::vector<ThreadCreator> thread_creator_;

  friend class SystemGroupBuilder;
  friend class SystemGroupBuilderItem;
};

/**
 * Example:
 *
 * auto manager = gs::SystemManager::MakeFromTraverser<gs::SingleThreadTraverser>();
 * manager->AddSystem<ASystem>().WithThread<DummyThread>();

 * gs::SystemGroup group_0;
 * group_0.AddSystem<BSystem>();
 * group_0.AddSystem<CSystem>();
 * group_0.AddSystem<DSystem>().WhichDependsOn<BSystem>();
 * manager->AddSystemGroup(group_0).WithThread<OpenGLThread>().WhichDependsOn<ASystem>();

 * manager->AddSystem<ESystem>();
 * manager->AddSystem<FSystem>().WhichDependsOn(group_0).And<ESystem>();

 * manager->SetMaxThreadCount<DummyThread>(2);
 * manager->SetMaxThreadCount<OpenGLThread>(1);

 * gs::EntityManager dummy;
 * manager->Configure(dummy);
 * manager->Update(dummy);
 */
class SystemManager : public SystemGroup, public std::enable_shared_from_this<SystemManager> {
 public:
  template <typename T, typename... Args>
  static typename std::enable_if<std::is_base_of<SystemTraverser, T>::value, std::shared_ptr<SystemManager>>::type
  MakeFromTraverser(Args&&... args);

  template <typename T>
  typename std::enable_if<std::is_base_of<SystemThread<T>, T>::value, void>::type SetMaxThreadCount(int count);
  void SetMaxThreadCount(int count);

  void Configure(EntityManager& entityManager);
  void Update(EntityManager& entityManager);

 private:
  void Reset();
  bool GetNext(std::shared_ptr<BaseSystem>& next);
  void OnSystemFinished(BaseSystem::Family family);
  void OnSystemTryAgainLater(BaseSystem::Family family);

  std::mutex locker;
  std::set<BaseSystem::Family> runnable_systems_;
  std::bitset<MAX_SYSTEM_COUNT> system_finished_;

  std::unique_ptr<SystemTraverser> system_traverser_;

  friend class SingleThreadTraverser;
  friend class MultiThreadTraverser;
};

class SystemTraverser {
 public:
  virtual ~SystemTraverser() = default;

  virtual void Traverse(std::function<void(std::shared_ptr<BaseSystem>&)> func) = 0;

  virtual void SetMaxThreadCount(SystemThreadBase::Family family, int count) {}

 protected:
  std::weak_ptr<SystemManager> manager_;

  friend class SystemManager;
};

class SingleThreadTraverser : public SystemTraverser {
 public:
  ~SingleThreadTraverser();

  void Traverse(std::function<void(std::shared_ptr<BaseSystem>&)> func) override;

 private:
  void CheckThread(SystemThreadBase::Family family);
  std::vector<std::shared_ptr<SystemThreadBase>> threads_;
};

class MultiThreadTraverser : public SystemTraverser {
 public:
  MultiThreadTraverser();
  ~MultiThreadTraverser();

  void Traverse(std::function<void(std::shared_ptr<BaseSystem>&)> func) override;

  void SetMaxThreadCount(SystemThreadBase::Family family, int count) override;

  class Thread {
   public:
    typedef std::function<void(void)> Task;

    Thread(std::shared_ptr<SystemThreadBase>& system_thread, MultiThreadTraverser* traverser)
        : system_thread_(system_thread), traverser_(traverser) {}
    void StartLoop();
    bool PostTask(const Task& task);
    void StopLoop();

   private:
    bool need_stop_ = false;
    std::shared_ptr<SystemThreadBase> system_thread_ = nullptr;
    std::shared_ptr<std::thread> thread_ = nullptr;

    std::mutex task_lock_;
    Task current_task_ = nullptr;

    std::mutex condition_lock_;
    std::condition_variable condition_ = {};

    MultiThreadTraverser* traverser_;
  };

 private:
  std::vector<std::vector<std::shared_ptr<Thread>>> all_threads_;

  std::mutex wait_thread_lock_;
  std::condition_variable wait_thread_condition_ = {};
};

}  // namespace gs
