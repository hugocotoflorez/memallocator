#ifndef _HALLOCATOR
#define _HALLOCATOR

#include <stddef.h> // size_t

void *__debug_get_head();
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
#define MAX_ALLOC_SIZE    0x1p14 /* 16384 */

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


/* Interfaces provided to user
 * Notes about implentation can be found in malloc.c */

void  __free(void *ptr);
void *__malloc(size_t size);
void *__calloc(size_t nmemb, size_t size);
void *__realloc(void *ptr, size_t size);

#endif // !_HALLOCATOR
