#include "ccimp/ccimp_os.h"

void * (*ccimp_malloc)(size_t size) = ccimp_malloc_real;

void * ccimp_malloc_real(size_t size)
{
    return malloc(size);
}

