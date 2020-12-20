#include <RAKU/core/memory.h>

#include <stdlib.h>
#include <string.h>
#include <errno.h>

RAKU_API
RakuStatus raku_alloc(size_t size, void **out)
{
    void *m = malloc(size);
    if (m == NULL && errno == ENOMEM)
    {
        return RAKU_NO_MEMORY;
    }

    *out = m;
    return RAKU_OK;
}

RAKU_API
RakuStatus raku_realloc(void *block, size_t new_size, void **out)
{
    void *m = realloc(block, new_size);
    if (m == NULL && errno == ENOMEM)
    {
        return RAKU_NO_MEMORY;
    }

    *out = m;
    return RAKU_OK;
}

RAKU_API
void raku_free(void *block)
{
    free(block);
}

RAKU_API
void raku_zero_memory(void *block, size_t size)
{
    memset(block, 0, size);
}