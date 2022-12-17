/*
 * Copyright (c) 2022 by GallenShao, All Rights Reserved.
 *
 * thread.h
 *
 *  Created on: 2022.12.17
 *  Author: gallenshao
 */

#pragma once

#include <functional>

namespace gs {

class SystemThreadBase {
 public:
  typedef int Family;
  virtual void OnInit(){};
  virtual void OnDestroy(){};

 protected:
  static Family family_count_;
};

typedef std::function<std::shared_ptr<SystemThreadBase>()> ThreadCreator;

/**
 * Example:
 *
 * class OpenGLThread : public gs::SystemThread<OpenGLThread> {
 * public:
 *   void OnInit() override {
 *     // Init OpenGL Context
 *     std::cout << "OpenGLThread OnInit" << std::endl;
 *   }
 *   void OnDestroy() override {
 *     // Destroy OpenGL Context
 *     std::cout << "OpenGLThread OnDestroy" << std::endl;
 *   }
 * };
 */
template <typename T>
class SystemThread : public SystemThreadBase {
 public:
  static Family family();
};

class DefaultThread : public SystemThread<DefaultThread> {};

}  // namespace gs
