/*
 * Copyright (c) 2022 by GallenShao, All Rights Reserved.
 *
 * multi_thread_traverser.cpp
 *
 *  Created on: 2022.05.15
 *  Author: gallenshao
 */

#include "system.h"

#define DEFAULT_default_thread_COUNT 4
#define DEFAULT_custom_thread_COUNT 1

gs::MultiThreadTraverser::MultiThreadTraverser() {
  all_threads_.resize(1);
  all_threads_[0].reserve(DEFAULT_default_thread_COUNT);
}

gs::MultiThreadTraverser::~MultiThreadTraverser() {
  for (auto& thread_list : all_threads_) {
    for (auto& thread : thread_list) {
      if (thread) {
        thread->StopLoop();
        thread = nullptr;
      }
    }
  }
  all_threads_.clear();
}

void gs::MultiThreadTraverser::Traverse(std::function<void(std::shared_ptr<gs::BaseSystem>&)> func) {
  auto system_manager_ = manager_.lock();
  if (system_manager_ == nullptr) {
    return;
  }

  std::shared_ptr<BaseSystem> current_system = nullptr;
  while (true) {
    if (current_system == nullptr) {
      auto success = system_manager_->GetNext(current_system);
      if (!success) {
        return;
      }
    }

    if (current_system) {
      auto thread_family = current_system->initializer_family_;
      if (thread_family >= all_threads_.size()) {
        all_threads_.resize(thread_family + 1);
        all_threads_[thread_family].reserve(DEFAULT_custom_thread_COUNT);
      }
      auto& target_thread_list = all_threads_[thread_family];

      gs::MultiThreadTraverser::Thread::Task task = [system=current_system->weak_from_this(), &func]() {
        auto system_strong = system.lock();
        if (system_strong) {
          func(system_strong);
        }
      };

      int search_index = 0;
      bool task_posted = false;
      // check if any thread is available
      for (; search_index < target_thread_list.size(); search_index++) {
        auto& thread = target_thread_list[search_index];
        if (thread->PostTask(task)) {
          task_posted = true;
          break;
        }
      }
      if (task_posted) {
        current_system = nullptr;
        continue;
      }

      // create new thread
      if (search_index < target_thread_list.capacity()) {
        std::shared_ptr<SystemThreadBase> system_thread = nullptr;
        if (thread_family < system_manager_->thread_creator_.size()) {
          system_thread = system_manager_->thread_creator_[thread_family]();
        }
        auto thread = std::make_shared<Thread>(system_thread, this);
        target_thread_list.push_back(thread);
        thread->StartLoop();
        if (thread->PostTask(task)) {
          current_system = nullptr;
          continue;
        }
      }

      // failed, try again later
      std::shared_ptr<BaseSystem> next;
      system_manager_->GetNext(next);
      if (next) {
        system_manager_->OnSystemTryAgainLater(current_system->GetFamily());
        current_system = next;
        continue;
      }
    }

    // wait until any system is done
    std::unique_lock<std::mutex> locker(wait_thread_lock_);
    wait_thread_condition_.wait(locker);
  }
}

void gs::MultiThreadTraverser::SetMaxThreadCount(gs::SystemThreadBase::Family family, int count) {
  if (family >= all_threads_.size()) {
    all_threads_.resize(family + 1);
  }
  all_threads_[family].reserve(count);
}

void gs::MultiThreadTraverser::Thread::StartLoop() {
  need_stop_ = false;
  thread_ = std::make_shared<std::thread>([this]() {
    if (system_thread_) {
      system_thread_->OnInit();
    }

    while (!need_stop_) {
      bool has_task = false;
      {
        std::unique_lock<std::mutex> locker(task_lock_);
        has_task = current_task_ != nullptr;
      }

      if (!has_task) {
        std::unique_lock<std::mutex> condition_locker(condition_lock_);
        condition_.wait(condition_locker);
        continue;
      }

      current_task_();
      {
        std::lock_guard<std::mutex> locker(traverser_->wait_thread_lock_);
        traverser_->wait_thread_condition_.notify_all();
      }

      {
        std::unique_lock<std::mutex> locker(task_lock_);
        current_task_ = nullptr;
      }
    }

    if (system_thread_) {
      system_thread_->OnDestroy();
    }
  });
}

bool gs::MultiThreadTraverser::Thread::PostTask(const Task& task) {
  std::unique_lock<std::mutex> locker(task_lock_);
  if (current_task_ == nullptr) {
    current_task_ = task;
    {
      std::lock_guard<std::mutex> condition_locker(condition_lock_);
      condition_.notify_all();
    }
    return true;
  } else{
    return false;
  }
}

void gs::MultiThreadTraverser::Thread::StopLoop() {
  need_stop_ = true;
  {
    std::lock_guard<std::mutex> condition_locker(condition_lock_);
    condition_.notify_all();
  }
  if (thread_ && thread_->joinable()) {
    thread_->join();
    thread_ = nullptr;
  }
}
