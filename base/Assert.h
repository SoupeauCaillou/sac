#pragma once

#include <iostream>
#include <cassert>

#define ASSERT(logic, message) \
   if (!(logic)) { \
      std::cout << message << std::endl; \
   } \
   assert(logic)

