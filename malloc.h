#ifndef _HALLOCATOR
#define _HALLOCATOR

#include <stddef.h> // size_t

void *__debug_get_head();
/*
 * Memory management library
 *
 * Author: Hugo Coto Florez
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


/* Page size */
#define SIZE 4096

/* Magic number. Used to check integrity */
#define MAGIC  0x123456
/* As pages are accesed by address and headers by offset, I use the
 * same magic number. I think its nearly impossible to access a page
 * with a header or viceversa. */

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
    size_t   size;    // page size (with page header)
    page_t  *next;    // next page
    page_t  *prev;    // prev page
    unsigned page_id; // page number
    unsigned magic;   // used to check if ptr is page_t
};


/* Header above used memory */
typedef struct
{
    size_t   size;  // allocated bytes without header size
    unsigned magic; // magic number to check header
    page_t  *page;  // page header of page that nide is in
} header_t;


/* Frees the memory space pointed to by PTR which must have been
 * returned by a previous call to malloc() or related functions.
 * Otherwise, or if PTR has already been freed, undefined behavior
 * occurs. If PTR is NULL, no operation is performed.
 *
 */
void __free(void *ptr);

/* Allocates SIZE bytes and returns a pointer to the allocated memory.
 * The memody is not initialized.
 */
void *__malloc(size_t size);

/* Allocates memory for an array of NMEMB elements of SIZE bytes each
 * and returns a pointer to the allocated memory. The memory is set to
 * zero.
 */
void *__calloc(size_t nmemb, size_t size);

/* Changes the size of the memory block pointed to by PTR to
 * SIZE bytes. The contents of the memory will be unchanged in the
 * range from the start of the region up to the minimum of the old
 * and new sizes. If the new size os larger than the old size, the
 * added memory will not be initialized.
 *
 * If PTR is NULL, then the call is equivalent to malloc(size),
 * for all values of SIZE.
 *
 * If size is equal to zero, and PTR is not NULL, then the call is
 * equivalent to free(ptr)
 *
 * Unless PTR is NULL, it must have been returned by an earlier call
 * to malloc or related functions. If the area pointed to was
 * moved, a free(ptr) is done.
 */
void *__realloc(void *ptr, size_t size);

#endif // !_HALLOCATOR
