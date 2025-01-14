#ifndef _HALLOCATOR
#define _HALLOCATOR

#include <stddef.h> // size_t

#define ALLOC_TEST
// #define USE_LIBC_MMAN

/*
 * Memory management library
 * by Hugo Coto Florez
 *
 * Inspiration: ostep book
 */

/* Info for code readers
 *
 * Name conventions:
 * - ptr: (void*) no relevant info
 * - hptr: (header_t*) current block header
 * - pptr: (page_t*) current page header
 * - nptr: (node_t*) current node header
 *
 * I hope code is as readable as possible. Any doubt
 * about what is happening can be asked directly to
 * myself (contact info in github profile) or oppening
 * an issue.
 *
 * Althought gcc let do arithmetic over void*, as C standard
 * said that void pointer arithmetic has undefined behaviour
 * I cast to char* all this cases.
 *
 * I appreciate any contribution, from finding a new bug
 * to implementing a new feature. Thanks for your time.
 */

/* Info about low level mechanisms
 *
 * Page:
 * Pages are raw chunks of data recived thow a mmap() call.
 * It contains a page header (page_t) at the LSB byte.
 * Remaining block space can be one of the following entries.
 *
 * Node:
 * A node (node_t) is the structure that store info
 * about a free block. It is placed just before the
 * free block.
 *
 * Header:
 * A header (header_t) that contains info about reserved memory.
 * After the header start an used block.
 *
 * Free block.
 * Data or nothing that is not allocated neither used
 * by any header. Just before this block there are a
 * node (node_t).
 *
 * Used block.
 * Data that where allocated. Just before this block
 * there are a header (header_t).
 */

/* Page size is by default DEFAULT_PAGE_SIZE, up to MAX_ALLOC_SIZE */
#define DEFAULT_PAGE_SIZE 0x1p12 /* 4096 */
#define MAX_ALLOC_SIZE    0x1p20 /* 1MB */

/* Magic number. Used to check integrity */
#define MAGIC 0x123456
/* As pages are accesed by address and headers by offset, I use the
 * same magic number. I think its nearly impossible to access a page
 * with a header or viceversa. Hugo's lazzy approach #1 */

typedef struct __page_t page_t;
typedef struct __node_t node_t;

/* Above each free block a node_t node is placed. */
struct __node_t
{
    size_t  size; // free block size in bytes (without node_t size)
    node_t *next; // next free node in the same page, or NULL if last
    node_t *prev; // previous free node in the same page, or NULL if first
    page_t *page; // page header of page that nide is in
};

/* First data of each page. */
struct __page_t
{
    page_t  *next;    // next page
    page_t  *prev;    // prev page
    size_t   size;    // page size (with page header)
    unsigned magic;   // used to check if ptr is page_t
#ifdef ALLOC_TEST
    unsigned page_id; // page number
#endif
};

/* Header above used memory */
typedef struct
{
    size_t   size;  // allocated bytes without header size
    unsigned magic; // magic number to check header
    page_t  *page;  // page header of page that nide is in
} header_t;


/* Interfaces provided to user.
 * Notes about implentation can be found in malloc.c */

/* The  free()  function  frees the memory space pointed to by ptr,
 * which must have been returned by a previous call to malloc()  or
 * related functions.  Otherwise, or if ptr has already been freed,
 * undefined behavior occurs.  If ptr is NULL, no operation is performed.
 * Return no value, and preserve errno.
 */
void  __free(void *ptr);

/* The malloc() function allocates size bytes and returns a pointer
 * to  the  allocated  memory.   The memory is not initialized.
 * Size must be grater than 0 (Non-gnu approach).
 *
 * Return a pointer to the  allocated  memory,  which  is  suitably
 * aligned  for any type that fits into the requested size or less.
 * On error, these functions return NULL and set errno.  Attempting
 * to allocate more than PTRDIFF_MAX bytes is considered an  error,
 * as an object that large could cause later pointer subtraction to
 * overflow.
 */
void *__malloc(size_t size);

/* The calloc() function allocates memory for an array of nmemb elements
 * of size bytes each and returns a pointer to the allocated
 * memory.  The memory is set to zero. Size must be grater than 0 (Non-gnu approach).
 *
 * Return a pointer to the  allocated  memory,  which  is  suitably
 * aligned  for any type that fits into the requested size or less.
 * On error, these functions return NULL and set errno.  Attempting
 * to allocate more than PTRDIFF_MAX bytes is considered an  error,
 * as an object that large could cause later pointer subtraction to
 * overflow.
 */
void *__calloc(size_t nmemb, size_t size);

/*
 * The  realloc()  function  changes  the  size of the memory block
 * pointed to by ptr to size bytes.  The  contents  of  the  memory
 * will  be  unchanged in the range from the start of the region up
 * to the minimum of the old and new sizes.  If  the  new  size  is
 * larger  than the old size, the added memory will not be initialized.
 *
 * If ptr is NULL, then the call is equivalent to malloc(size), for
 * all values of size.
 *
 * If size is equal to zero, and ptr is not NULL, then the call  is
 * equivalent  to  free(ptr).
 *
 * Unless ptr is NULL, it must have been  returned  by  an  earlier
 * call to malloc or related functions.  If the area pointed to was
 * moved, a free(ptr) is done.
 *
 * Return a pointer to the  allocated  memory,  which  is  suitably
 * aligned  for any type that fits into the requested size or less.
 * On error, these functions return NULL and set errno.  Attempting
 * to allocate more than PTRDIFF_MAX bytes is considered an  error,
 * as an object that large could cause later pointer subtraction to
 * overflow.
 * The realloc() function return NULL if ptr is
 * not  NULL and the requested size is zero; this is not considered
 * an error.
 * Otherwise,  the  returned  pointer may be the same as ptr if the
 * allocation was not moved (e.g., there was room to expand the allocation in-place),
 * or different from ptr if the allocation  was
 * moved  to  a new address.  If these functions fail, the original
 * block is left untouched; it is not freed or moved.
 */
void *__realloc(void *ptr, size_t size);

#ifdef ALLOC_TEST
/* Get the pointer to the first page */
void *__debug_get_head();
#endif

#endif // !_HALLOCATOR
