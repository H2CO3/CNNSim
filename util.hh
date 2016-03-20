//
// util.hh
// CNNSim, a simple CNN simulator
//
// Created by Arpad Goretity on 17/03/2016
//
// Licensed under the 2-clause BSD License
//

#ifndef CNNSIM_UTIL_HH
#define CNNSIM_UTIL_HH

// We really love aliasing-based optimizations in this project.
// Too bad C++ hasn't standardized 'restrict' yet.
#if defined(__GNUC__) || defined(__GNUG__)
#define RESTRICT __restrict__
#elif defined(_MSC_VER)
#define RESTRICT __restrict
#else
#define RESTRICT
#endif

#endif // CNNSIM_UTIL_HH
