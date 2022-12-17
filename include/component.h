/*
 * Copyright (c) 2022 by GallenShao, All Rights Reserved.
 *
 * component.h
 *
 *  Created on: 2022.12.17
 *  Author: gallenshao
 */

#pragma once

namespace gs {

class ComponentBase {
 public:
  typedef int Family;

 protected:
  static Family family_count_;
};

template <typename T>
class Component : public ComponentBase {
 public:
  static Family family();
};

}  // namespace gs
