/**
 * Copyright 2021 by GallenShao, All Rights Reserved.
 *
 * system.h
 *
 *  Created on: 2022.05.14
 *  Author: gallenshao
 */

#pragma once

#include "system.h"

template <typename T>
gs::BaseSystem::Family gs::System<T>::family() {
  static Family family = family_count_++;
  return family;
}

template <typename T>
gs::BaseSystem::Family gs::System<T>::GetFamily() {
  return family();
}

template <typename T>
gs::SystemThreadBase::Family gs::SystemThread<T>::family() {
  static Family family = family_count_++;
  return family;
}

template <typename T>
typename std::enable_if<std::is_base_of<gs::System<T>, T>::value, gs::SystemGroupBuilder>::type
gs::SystemGroup::AddSystem() {
  assert(editable_);
  auto family = T::family();
  if (all_systems_.size() <= family) {
    all_systems_.resize(family + 1);
  }
  assert(all_systems_[family] == nullptr);
  all_systems_[family] = std::make_shared<T>();
  all_systems_mask_.set(family);
  start_node_families_.insert(family);

  std::set<BaseSystem::Family> current;
  current.insert(family);
  return {this, current};
}

template <typename T>
typename std::enable_if<std::is_base_of<gs::System<T>, T>::value, gs::SystemGroupBuilderItem>::type
gs::SystemGroupBuilder::WhichDependsOn() {
  gs::SystemGroupBuilderItem item = {group_, current_};
  return item.And<T>();
}

template <typename T>
typename std::enable_if<std::is_base_of<gs::SystemThread<T>, T>::value, gs::SystemGroupBuilder>::type
gs::SystemGroupBuilder::WithThread() {
  auto initializer_family = T::family();
  if (group_->thread_creator_.size() <= initializer_family) {
    group_->thread_creator_.resize(initializer_family + 1);
  }
  auto& creator = group_->thread_creator_[initializer_family];
  if (creator == nullptr) {
    creator = []() {
      return std::static_pointer_cast<SystemThreadBase>(std::make_shared<T>());
    };
  }
  for (auto& system_family : current_) {
    auto& system = group_->all_systems_[system_family];
    assert(system != nullptr);
    assert(system->initializer_family_ == DefaultThread::family());
    system->initializer_family_ = initializer_family;
  }
  return *this;
}

template <typename T>
typename std::enable_if<std::is_base_of<gs::System<T>, T>::value, gs::SystemGroupBuilderItem>::type
gs::SystemGroupBuilderItem::And() {
  auto dependency_family = T::family();
  assert(group_->all_systems_mask_.test(dependency_family));
  assert(current_.find(dependency_family) == current_.end());

  auto dependency_system = group_->all_systems_[dependency_family];
  for (auto& family : current_) {
    auto system = group_->all_systems_[family];
    system->dependencies_[dependency_family] = true;
    group_->start_node_families_.erase(family);
    dependency_system->next_.insert(family);
  }

  return *this;
}

template <typename T, typename ... Args>
typename std::enable_if<std::is_base_of<gs::SystemTraverser, T>::value, std::shared_ptr<gs::SystemManager>>::type
gs::SystemManager::MakeFromTraverser(Args && ... args) {
  auto system_manager = std::make_shared<gs::SystemManager>();
  system_manager->system_traverser_ = std::make_unique<T>(std::forward<Args>(args) ...);
  system_manager->system_traverser_->manager_ = system_manager->shared_from_this();

  auto default_initializer_family = DefaultThread::family();
  system_manager->thread_creator_.resize(default_initializer_family + 1);
  system_manager->thread_creator_[default_initializer_family] = []() {
    return std::static_pointer_cast<SystemThreadBase>(std::make_shared<DefaultThread>());
  };
  return system_manager;
}

template <typename T>
typename std::enable_if<std::is_base_of<gs::SystemThread<T>, T>::value, void>::type
gs::SystemManager::SetMaxThreadCount(int count) {
  system_traverser_->SetMaxThreadCount(T::family(), count);
}
