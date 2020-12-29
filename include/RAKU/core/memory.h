#ifndef RAKU_CORE_MEMORY_H
#define RAKU_CORE_MEMORY_H

#include <RAKU/export.h>
#include <RAKU/core/defs.h>
#include <RAKU/core/status.h>

#if defined(__cplusplus)
#include <cstdint>
extern "C" {
#else
#include <stdint.h>
#endif

RAKU_API
RakuStatus raku_alloc(size_t size, void **out);

RAKU_API
RakuStatus raku_realloc(void *block, size_t new_size, void **out);

RAKU_API
void raku_free(void *block);

RAKU_API
void raku_zero_memory(void *block, size_t size);

#if defined(__cplusplus)
}
#endif

#endif