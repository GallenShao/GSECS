/*
 * Copyright (c) 2022 by GallenShao, All Rights Reserved.
 *
 * test_header.h
 *
 *  Created on: 2022.05.22
 *  Author: gallenshao
 */

#pragma once

#include "gs_ecs.h"

class ASystem : public gs::System<ASystem> {
  void Update(gs::EntityManager& manager) override {
    std::cout << "ASystem Update" << std::endl;
  }
};
class BSystem : public gs::System<BSystem> {
  void Update(gs::EntityManager& manager) override {
    std::cout << "BSystem Update" << std::endl;
  }
};
class CSystem : public gs::System<CSystem> {
  void Update(gs::EntityManager& manager) override {
    std::cout << "CSystem Update" << std::endl;
  }
};
class DSystem : public gs::System<DSystem> {
  void Update(gs::EntityManager& manager) override {
    std::cout << "DSystem Update" << std::endl;
  }
};
class ESystem : public gs::System<ESystem> {
  void Update(gs::EntityManager& manager) override {
    std::cout << "ESystem Update" << std::endl;
  }
};
class FSystem : public gs::System<FSystem> {
  void Update(gs::EntityManager& manager) override {
    std::cout << "FSystem Update" << std::endl;
  }
};

class DummyThread : public gs::SystemThread<DummyThread> {
 public:
  void OnInit() override {
    std::cout << "DummyThread OnInit" << std::endl;
  }
  void OnDestroy() override {
    std::cout << "DummyThread OnDestroy" << std::endl;
  }
};

class OpenGLThread : public gs::SystemThread<OpenGLThread> {
 public:
  void OnInit() override {
    std::cout << "OpenGLThread OnInit" << std::endl;
  }
  void OnDestroy() override {
    std::cout << "OpenGLThread OnDestroy" << std::endl;
  }
};