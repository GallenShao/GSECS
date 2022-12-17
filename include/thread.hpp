/**
 * Copyright 2022 by GallenShao, All Rights Reserved.
 *
 * thread.hpp
 *
 *  Created on: 2022.12.17
 *  Author: gallenshao
 */

#include "thread.h"

template <typename T>
gs::SystemThreadBase::Family gs::SystemThread<T>::family() {
  static Family family = family_count_++;
  return family;
}
