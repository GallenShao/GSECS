/**
 * Copyright 2022 by GallenShao, All Rights Reserved.
 *
 * component.hpp
 *
 *  Created on: 2022.12.17
 *  Author: gallenshao
 */

#pragma once

#include "component.h"

template <typename T>
gs::ComponentBase::Family gs::Component<T>::family() {
  static Family family = family_count_++;
  return family;
}