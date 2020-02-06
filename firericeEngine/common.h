// Copyright 2015 Red Blob Games <redblobgames@gmail.com>
// License: Apache v2.0 <http://www.apache.org/licenses/LICENSE-2.0.html>

#ifndef COMMON_H
#define COMMON_H

// I use this for debugging
#include <iostream>
#include <SDL.h>

// Put on classes that manage resources and aren't to be copied
struct nocopy {
  nocopy() = default;
  nocopy(const nocopy&) = delete;
  nocopy& operator = (const nocopy&) = delete;
};

#endif
