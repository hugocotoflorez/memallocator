#include "malloc.h"
#include <assert.h> // just for test
#include <errno.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <sys/mman.h> // mmap, munmap

#include <stdio.h>

/* See malloc.h for further information about
 * how this shit work and interface usage for
 * programmers.
 */

/* head is the first page.
 * If head is NULL there are no pages initialized
 */
/* static */
page_t         *head       = NULL;
static unsigned page_count = 0;

inline void *
__debug_get_head()
{
    return head;
}

/* Get last page. O(n) */
static page_t *
get_last_page()
{
    page_t *pptr = head;

    if (pptr == NULL)
        return NULL;

    while (pptr->next != NULL)
        pptr = pptr->next;

    return pptr;
}

static void *
get_prev_node(page_t *pptr, node_t *cur_node)
{
    assert(pptr->magic == MAGIC);

    node_t *nptr = (node_t *) (pptr + 1);

    if (nptr == NULL)
        return NULL;

    while (nptr->next && nptr->next < cur_node)
        nptr = nptr->next;

    return nptr;
}

static node_t *
get_best_fit(size_t size)
{
    page_t *pptr;
    node_t *nptr;
    node_t *best_fit = NULL;
    size_t  real_size;
    signed  assigned = 0;

    /* Go througth page list */
    for (pptr = head; pptr != NULL; pptr = pptr->next)
    {
        /* first node into page */
        nptr = (node_t *) ((char *) pptr + sizeof(page_t));

        for (; nptr != NULL; nptr = nptr->next)
        {
            real_size = nptr->size + sizeof(node_t) - sizeof(header_t);
            /* Best fit changes if size fits in current node and:
             * - best fit is not assigned yet
             * - new node size is smaller than best fit size */
            if (real_size >= size && (!best_fit || real_size < best_fit->size))
                best_fit = nptr;
        }
    }
    return best_fit;
}

/* Return a page to os. All its contents would be lost.
 * Have to be called once all page contents are free.
 * PAGE is deleted from pages list and head updated if needed.
 * PAGE is set to NULL on success.
 */
static void
__page_destroy(page_t *page)
{
    if (page == NULL || page->magic != MAGIC)
        return;

    /* Unlink page */
    if (page->next)
        page->next->prev = page->prev;

    if (page->prev)
        page->prev->next = page->next;

    /* Update head if needed */
    else
        head = page->next;


    /* Free current page */
    munmap(page, page->size);
}

/* Ask for a new page and update PAGE to the new reserved memory.
 * New chunk is formated with page and header, ready to use.
 * If PAGE is not NULL at function call, PAGE is returned without
 * any action or change.
 * If PAGE is NULL an still NULL at return, some error happened.
 */
static void *
__page_create(size_t size)
{
    node_t *nptr;
    page_t *pptr;

    pptr = mmap(NULL, size, PROT_READ | PROT_WRITE, MAP_ANON | MAP_PRIVATE, -1, 0);

    /* Page can not be allocated */
    if (pptr == NULL)
        return NULL;

    /* Fill pptr header */
    *pptr = (page_t){
        .page_id = page_count++,
        .next    = NULL,
        .prev    = get_last_page(),
        .size    = size,
        .magic   = MAGIC,
    };

    /* Create first node */
    nptr  = (node_t *) (pptr + 1);
    *nptr = (node_t){
        .page = pptr,
        .size = pptr->size - sizeof(page_t) - sizeof(node_t),
        .next = NULL,
        .prev = NULL,
    };

    /* Link current pptr */
    if (pptr->prev)
        pptr->prev->next = pptr;

    /* If first pptr update head */
    else
        head = pptr;

    return pptr;
}

static void
destroy_all_pages()
{
    page_t *pptr = head;
    page_t *next = NULL;

    /* Go througth page list */
    for (; pptr != NULL; pptr = next)
    {
        next = pptr->next;
        __page_destroy(pptr);
    }
}

__attribute__((constructor)) static void
__init__()
{
    atexit(destroy_all_pages);
}

void
__free(void *ptr)
{
    header_t *hptr;
    node_t   *nptr;
    header_t  temp_header;

    /* Avoid NULL as argument */
    if (ptr == NULL)
        return;

    hptr = (header_t *) ptr - 1;

    /* Check if it is not valid */
    if (hptr == NULL || hptr->magic != MAGIC)
        return;

    /* Unset header as valid */
    hptr->magic = 0;

    /* New node to add */
    temp_header = *hptr;
    nptr        = (node_t *) hptr;
    nptr->size  = temp_header.size + sizeof(header_t) - sizeof(node_t);
    nptr->page  = temp_header.page;
    nptr->prev  = get_prev_node(nptr->page, nptr);

    /* Insert new node */
    nptr->next = nptr->prev->next;
    if (nptr->next)
        nptr->next->prev = nptr;
    if (nptr->prev)
        nptr->prev->next = nptr;


    /* join with next node if also free */
    if ((char *) nptr + sizeof(node_t) + nptr->size == (char *) nptr->next)
    {
        nptr->size += nptr->next->size + sizeof(node_t);
        nptr->next = nptr->next->next;
    }

    /* join with previous node if also free */
    if ((char *) nptr->prev + sizeof(node_t) + nptr->prev->size == (char *) nptr)
    {
        nptr->prev->size += nptr->size + sizeof(node_t);
        nptr->prev->next = nptr->next;
    }

    /* if any memory is used, return it to os */
    if (nptr->prev->size == nptr->page->size - sizeof(node_t) - sizeof(page_t))
    {
        __page_destroy(nptr->page);
    }
}

void *
__malloc(size_t size)
{
    if (size == 0 || size >= SIZE)
        return NULL;

    header_t *hptr;
    node_t   *nptr;
    page_t   *pptr;
    size_t    realsize = size + sizeof(header_t);

    /* Find a free node to allocate SIZE bytes */
    nptr = get_best_fit(realsize);

    /* If cant be allocated in any page, create a new one */
    if (nptr == NULL)
    {
        /* Error at page creation */
        if ((pptr = __page_create(SIZE)) == NULL)
            return NULL;

        /* node is first page node */
        nptr = (node_t *) (pptr + 1);
    }

    /* If it fits, selected page is nptr page */
    else
        pptr = nptr->page;

    /* set hptr as the last SIZE bytes of hptr */
    hptr = (header_t *) ((char *) nptr + sizeof(node_t) + nptr->size - realsize);

    *hptr = (header_t){
        .size  = size,
        .magic = MAGIC,
        .page  = pptr,
    };

    nptr->size -= realsize;

    return (hptr + 1);
}

void *
__calloc(size_t nmemb, size_t size)
{
    return NULL;
}

void *
__realloc(void *ptr, size_t size)
{
    return NULL;
}
