#include "system.hpp"

namespace gs {

BaseSystem::Family BaseSystem::family_count_ = 0;
SystemThreadBase::Family SystemThreadBase::family_count_ = 0;

SystemGroupBuilder SystemGroup::AddSystemGroup(SystemGroup& group) {
  assert(!group.all_systems_.empty());
  assert((all_systems_mask_ & group.all_systems_mask_).none());
  assert(group.editable_);
  assert(editable_);
  group.editable_ = false;

  if (all_systems_.size() < group.all_systems_.size()) {
    all_systems_.resize(group.all_systems_.size());
  }
  std::set<BaseSystem::Family> current;
  for (int family = 0; family < group.all_systems_.size(); family++) {
    auto& system = group.all_systems_[family];
    if (system != nullptr) {
      assert(all_systems_[family] == nullptr);
      all_systems_[family] = system;
      current.insert(family);
    }
  }
  all_systems_mask_ |= group.all_systems_mask_;

  if (thread_creator_.size() < group.thread_creator_.size()) {
    thread_creator_.resize(group.thread_creator_.size());
  }
  for (int family = 0; family < group.thread_creator_.size(); family++) {
    auto& creator = group.thread_creator_[family];
    if (creator != nullptr) {
      thread_creator_[family] = creator;
    }
  }

  start_node_families_.insert(group.start_node_families_.begin(), group.start_node_families_.end());
  return {this, current};
}

SystemGroupBuilderItem SystemGroupBuilder::WhichDependsOn(SystemGroup& group) {
  gs::SystemGroupBuilderItem item = {group_, current_};
  return item.And(group);
}

SystemGroupBuilderItem SystemGroupBuilderItem::And(SystemGroup& group) {
  assert(((group_->all_systems_mask_ & group.all_systems_mask_) ^ group.all_systems_mask_).none());

  for (auto& family : current_) {
    assert(group.all_systems_mask_.test(family) == false);
    auto current_system = group_->all_systems_[family];
    current_system->dependencies_ |= group.all_systems_mask_;
    group_->start_node_families_.erase(family);
  }

  for (int family = 0; family < group.all_systems_.size(); family++) {
    auto& system = group.all_systems_[family];
    if (system != nullptr) {
      assert(system == group_->all_systems_[family]);
      system->next_.insert(current_.begin(), current_.end());
    }
  }

  return *this;
}

void SystemManager::Reset() {
  std::lock_guard<std::mutex> autoLock(locker);
  runnable_systems_ = start_node_families_;
  system_finished_.reset();
}

bool SystemManager::GetNext(std::shared_ptr<BaseSystem>& next) {
  std::lock_guard<std::mutex> autoLock(locker);
  if (runnable_systems_.empty()) {
    next = nullptr;
    return all_systems_mask_ != system_finished_;
  }
  auto iter = runnable_systems_.begin();
  next = all_systems_[*iter];
  runnable_systems_.erase(iter);
  return true;
}

void SystemManager::OnSystemFinished(BaseSystem::Family family) {
  std::lock_guard<std::mutex> autoLock(locker);
  auto system = all_systems_[family];
  if (system == nullptr) {
    return;
  }

  system_finished_[family] = true;
  for (auto& next_family : system->next_) {
    auto next_system = all_systems_[next_family];
    assert(next_system != nullptr);

    if ((system_finished_ & next_system->dependencies_) == next_system->dependencies_) {
      runnable_systems_.insert(next_family);
    }
  }
}

void SystemManager::OnSystemTryAgainLater(BaseSystem::Family family) {
  std::lock_guard<std::mutex> autoLock(locker);
  runnable_systems_.insert(family);
}

void SystemManager::Configure(EntityManager& entityManager) {
  Reset();
  system_traverser_->Traverse([this, &entityManager](std::shared_ptr<BaseSystem>& system) {
    system->Configure(entityManager);
    OnSystemFinished(system->GetFamily());
  });
}

void SystemManager::Update(EntityManager& entityManager) {
  Reset();
  system_traverser_->Traverse([this, &entityManager](std::shared_ptr<BaseSystem>& system) {
    system->Update(entityManager);
    OnSystemFinished(system->GetFamily());
  });
}

void SystemManager::SetMaxThreadCount(int count) {
  SetMaxThreadCount<DefaultThread>(count);
}

}  // namespace gs