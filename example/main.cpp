#include <iostream>

#include "gs_ecs.h"
#include "test_header.h"

int main() {
  // example 1, System
  {
    std::cout << "----- example 1 -----" << std::endl;
    auto manager = gs::SystemManager::MakeFromTraverser<gs::SingleThreadTraverser>();
    manager->AddSystem<ASystem>();
    manager->AddSystem<BSystem>();
    manager->AddSystem<CSystem>().WhichDependsOn<ASystem>();
    manager->AddSystem<DSystem>().WhichDependsOn<BSystem>().And<CSystem>();
    manager->AddSystem<ESystem>();
    manager->AddSystem<FSystem>().WhichDependsOn<CSystem>().And<ESystem>();

    gs::EntityManager dummy;
    manager->Configure(dummy);
    manager->Update(dummy);
  }

  // example 2, SystemGroup
  {
    std::cout << "----- example 2 -----" << std::endl;
    auto manager = gs::SystemManager::MakeFromTraverser<gs::SingleThreadTraverser>();
    gs::SystemGroup group_0;
    group_0.AddSystem<DSystem>();
    group_0.AddSystem<CSystem>().WhichDependsOn<DSystem>();
    manager->AddSystemGroup(group_0);
    gs::SystemGroup group_1;
    group_1.AddSystem<ASystem>();
    group_1.AddSystem<BSystem>();
    manager->AddSystemGroup(group_1).WhichDependsOn(group_0);

    gs::EntityManager dummy;
    manager->Configure(dummy);
    manager->Update(dummy);
  }

  // example 3, mixed
  {
    std::cout << "----- example 3 -----" << std::endl;
    auto manager = gs::SystemManager::MakeFromTraverser<gs::SingleThreadTraverser>();
    manager->AddSystem<ASystem>();
    gs::SystemGroup group_0;
    group_0.AddSystem<BSystem>();
    group_0.AddSystem<CSystem>().WhichDependsOn<BSystem>();
    manager->AddSystemGroup(group_0).WhichDependsOn<ASystem>();
    manager->AddSystem<DSystem>();
    manager->AddSystem<ESystem>().WhichDependsOn(group_0).And<DSystem>();

    gs::EntityManager dummy;
    manager->Configure(dummy);
    manager->Update(dummy);
  }

  // example 4, single thread traverser with initializer
  //      ----------
  // A -> | B -> D | \
  //      | C      |  \
  //      ----------   -> F
  //                  /
  // E --------------/
  {
    std::cout << "----- example 4 -----" << std::endl;
    auto manager = gs::SystemManager::MakeFromTraverser<gs::SingleThreadTraverser>();
    manager->AddSystem<ASystem>().WithThread<DummyThread>();

    gs::SystemGroup group_0;
    group_0.AddSystem<BSystem>();
    group_0.AddSystem<CSystem>();
    group_0.AddSystem<DSystem>().WhichDependsOn<BSystem>();
    manager->AddSystemGroup(group_0).WithThread<OpenGLThread>().WhichDependsOn<ASystem>();

    manager->AddSystem<ESystem>();
    manager->AddSystem<FSystem>().WhichDependsOn(group_0).And<ESystem>();

    manager->SetMaxThreadCount<DummyThread>(2);
    manager->SetMaxThreadCount<OpenGLThread>(1);

    gs::EntityManager dummy;
    manager->Configure(dummy);
    manager->Update(dummy);
  }

  // example 5, multi thread traverser
  //      ----------
  // A -> | B -> D | \
  //      | C      |  \
  //      ----------   -> F
  //                  /
  // E --------------/
  {
    std::cout << "----- example 5 -----" << std::endl;
    auto manager = gs::SystemManager::MakeFromTraverser<gs::MultiThreadTraverser>();
    manager->AddSystem<ASystem>().WithThread<DummyThread>();

    gs::SystemGroup group_0;
    group_0.AddSystem<BSystem>();
    group_0.AddSystem<CSystem>();
    group_0.AddSystem<DSystem>().WhichDependsOn<BSystem>();
    manager->AddSystemGroup(group_0).WithThread<OpenGLThread>().WhichDependsOn<ASystem>();

    manager->AddSystem<ESystem>();
    manager->AddSystem<FSystem>().WhichDependsOn(group_0).And<ESystem>();

    manager->SetMaxThreadCount(4);
    manager->SetMaxThreadCount<DummyThread>(2);
    manager->SetMaxThreadCount<OpenGLThread>(1);

    gs::EntityManager dummy;
    manager->Configure(dummy);
    manager->Update(dummy);
  }

  return 0;
}