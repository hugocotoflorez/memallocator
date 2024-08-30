#include "malloc.h"
#include <assert.h>

/* If no sefgault raises and no
 * assert test return anything,
 * all test are passed. */
int
main()
{
    void **ptr_arr;
    assert((ptr_arr = __malloc(10 * sizeof(void *))));
    assert((ptr_arr[0] = __malloc(100)));
    assert((ptr_arr[1] = __malloc(100)));
    assert((ptr_arr[2] = __malloc(100)));

    for (signed char i = 0; i < 100; i++)
        *((char *) (ptr_arr[2]) + i) = i;

    for (signed char i = 0; i < 100; i++)
        assert(*((char *) (ptr_arr[2]) + i) == i);

    assert((ptr_arr = __realloc(ptr_arr, 20 * sizeof(void *))));

    for (signed char i = 0; i < 100; i++)
        assert(*((char *) (ptr_arr[2]) + i) == i);

    __free(NULL);
    __free(ptr_arr + 243);
    __free(ptr_arr[0]);
    __free(ptr_arr[1]);
    __free(ptr_arr[2]);
    __free(ptr_arr);
    assert(__debug_get_head() == NULL);

    ptr_arr = __calloc(10, sizeof(void *));

    for (signed char i = 0; i < 10; i++)
        assert(ptr_arr[i] == NULL);
}
