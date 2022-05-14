/*
 * Copyright (c) 2022 by GallenShao, All Rights Reserved.
 *
 * single_thread_traverser.cpp
 *
 *  Created on: 2022.05.15
 *  Author: gallenshao
 */

#include "system.h"

gs::SingleThreadTraverser::~SingleThreadTraverser() {
  for (auto& thread : threads_) {
    if (thread) {
      thread->OnDestroy();
    }
  }
  threads_.clear();
}

void gs::SingleThreadTraverser::CheckThread(SystemThreadBase::Family family) {
  if (family < 0) {
    return;
  }

  auto system_manager_ = manager_.lock();
  if (system_manager_ == nullptr) {
    return;
  }

  if (family >= threads_.size()) {
    threads_.resize(family + 1);
  }
  auto& thread = threads_[family];
  if (thread == nullptr) {
    auto creator = system_manager_->thread_creator_[family];
    if (creator) {
      thread = creator();
      thread->OnInit();
    }
  }
}

void gs::SingleThreadTraverser::Traverse(std::function<void(std::shared_ptr<gs::BaseSystem>&)> func) {
  auto system_manager_ = manager_.lock();
  if (system_manager_ == nullptr) {
    return;
  }

  while (true) {
    std::shared_ptr<BaseSystem> next;
    auto success = system_manager_->GetNext(next);
    if (!success) {
      return;
    }
    if (next) {
      CheckThread(next->initializer_family_);
      func(next);
    }
  }
}