
#pragma once

#ifdef _MSC_VER
#ifdef LBUFF_EXPORT
#define LBUFF_API _declspec(dllexport)
#else
#define LBUFF_API _declspec(dllimport)
#endif
#else
#define LBUFF_API extern
#endif

#ifdef __cplusplus
extern "C" {
#endif

#ifndef LBUFF_EXPORT
#include "buffer.h"
#endif

#ifdef __cplusplus
}
#endif
