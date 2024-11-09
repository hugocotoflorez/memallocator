#include "malloc.h"
#include <errno.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <sys/mman.h> // mmap, munmap

#include <stdio.h>
/* See malloc.h for further information about
 * how this shit work and interface usage for
 * programmers. */

/* head is the first page.
 * If head is NULL there are no pages initialized */
static page_t *head = NULL;

#ifdef ALLOC_TEST
/* Unused variable, just for print info (at testing) */
static unsigned page_count = 0;

/* Just used to pass head to test.c */
inline void *
__debug_get_head()
{
    return head;
}
#endif

static page_t *
get_last_page()
{
    /* I use a linear search instead of a direct pointer
     * because most times there are a few pages, so
     * a loop over a few nodes is harmless and 8bytes are
     * allways 8 bytes (Hugo's lazzy approach #2). */
    page_t *pptr = head;

    /* No pages initialized */
    if (pptr == NULL)
        return NULL;

    while (pptr->next != NULL)
        pptr = pptr->next;

    return pptr;
}

/* Get the free block node before PTR.
 * PTR is a valid address in the page
 * pointed by PPTR.*/
static void *
get_prev_node(page_t *pptr, void *ptr)
{
    node_t *nptr = (node_t *) (pptr + 1);

    /* Note for debugging:
     * A SEGFAULT in the next line is not
     * caused by this funcion, something is
     * not assigned correctly so nptr->next
     * never becames NULL as espected.
     * Trust me <3 */
    while (nptr->next && (void *) nptr->next < ptr)
        nptr = nptr->next;

    return nptr;
}

static node_t *
get_best_fit(size_t size)
{
    /* Fit algorithm
     * Note: algorithm can be changed by
     * changing this funcion implementation
     *
     * First smallest block algorithm.
     * This algorithm return the smallest block
     * of SIZE or more bytes in the first page
     * with a block that big.
     *
     * Its quite inefficient but simple.
     */
    page_t *pptr;
    node_t *nptr;
    node_t *best_fit = NULL;
    size_t  real_size;

    /* Go througth page list */
    for (pptr = head; pptr != NULL; pptr = pptr->next)
    {
        /* first node into page */
        nptr = (node_t *) ((char *) pptr + sizeof(page_t));

        for (; nptr != NULL; nptr = nptr->next)
        {
            real_size = nptr->size + sizeof(header_t);
            /* Best fit changes if size fits in current node and:
             * - best fit is not assigned yet
             * - new node size is smaller than best fit size */
            if (real_size >= size && (!best_fit || real_size < best_fit->size))
                best_fit = nptr;
        }
        if (best_fit != NULL)
            return best_fit;
    }
    return NULL;
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
    if (pptr == MAP_FAILED)
    {
        /* Set errno as OUT OF MEMORY */
        errno = ENOMEM;
        return NULL;
    }

    /* Fill pptr header */
    *pptr = (page_t){
        .next  = NULL,
        .prev  = get_last_page(),
        .size  = size,
        .magic = MAGIC,
#ifdef ALLOC_TEST
        .page_id = page_count++,
#endif
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
    nptr->prev  = get_prev_node(hptr->page, ptr);

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
    header_t *hptr;
    node_t   *nptr;
    page_t   *pptr;
    size_t    realsize;
    size_t    greater_size_needed;
    size_t    block_size = DEFAULT_PAGE_SIZE;

    /* SIZE with both page and block headers */
    greater_size_needed = size + sizeof(page_t) + sizeof(header_t);

    /* Invalid size checks */
    if (size == 0 || greater_size_needed > MAX_ALLOC_SIZE)
        return NULL;

    /* Total size, requested size + header size */
    realsize = size + sizeof(header_t);

    /* Find a free node to allocate SIZE bytes */
    nptr = get_best_fit(realsize);

    /* If cant be allocated in any page, create a new one */
    if (nptr == NULL)
    {
        /* Get a block size with at least SIZE bytes avaliable (or max size) */
        while (greater_size_needed >= block_size && block_size < MAX_ALLOC_SIZE)
            block_size <<= 1;

        /* Error at page creation */
        if ((pptr = __page_create(block_size)) == NULL)
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

    /* return the address of the first byte after the header */
    return (hptr + 1);
}

void *
__calloc(size_t nmemb, size_t size)
{
    void *ret = __malloc(nmemb * size);

    if (ret)
        return memset(ret, 0, nmemb * size);

    return NULL;
}

void *
realloc_use_next(void *ptr, size_t size, header_t *hptr)
{
    /* +------------------------+
     * | Used                   | <- ptr
     * +------------------------+
     * | Free                   | <- next
     * +------------------------+
     * First block use part of the second, and node is moved.
     */
    void     *next;
    header_t *hnext;
    node_t    nnext;
    size_t    needed_size;
    node_t   *nnode;

    next = (char *) ptr + hptr->size;

    /* Check if next is out of page */
    if ((char *) next >= (char *) hptr->page + hptr->page->size)
        return NULL;


    /* Next block is not free */
    hnext = (header_t *) next;
    if (hnext->magic == MAGIC)
        return NULL;

    needed_size = size - hptr->size;
    nnext       = *(node_t *) next;

    /* Invalid size (not enought) */
    if (nnext.size < needed_size)
        return NULL;

    /* Create new node for next (moved) free block */
    nnode  = (node_t *) ((char *) next + needed_size);
    *nnode = (node_t){
        .size = nnext.size - needed_size,
        .next = nnext.next,
        .page = nnext.page,
        .prev = nnext.prev,
    };

    /* Link new node */
    if (nnode->prev)
        nnode->prev->next = nnode;
    if (nnode->next)
        nnode->next->prev = nnode;

    /* Update header size */
    hptr->size += needed_size;

    return ptr;
}

void *
realloc_use_prev(void *ptr, int size, header_t *hptr)
{
    /* +------------------------+
     * | Free                   | <- prev
     * +------------------------+
     * | Used                   | <- ptr
     * +------------------------+
     * Second block use part of the first, and header is moved.
     */
    header_t *nhptr; /* New header address */
    node_t   *prev;

    nhptr = (header_t *) ((char *) hptr - (size - hptr->size));
    prev  = get_prev_node(hptr->page, ptr);

    /* check if previous free block is just before current block */
    if ((char *) (prev + 1) + prev->size != (char *) hptr)
        return NULL;

    /* Block is to big to realloc here */
    if (prev->size < size - hptr->size)
        return NULL;

    /* Update prev free node size */
    prev->size -= size - hptr->size;

    /* Create (move) header */
    *nhptr = (header_t){
        .size  = size,
        .magic = MAGIC,
        .page  = hptr->page,
    };

    /* Copy data */
    memmove(nhptr + 1, ptr, size);

    return nhptr + 1;
}

void *
__realloc(void *ptr, size_t size)
{
    void     *newptr = NULL;
    header_t *hptr;

    /* Any page initialized */
    if (head == NULL)
    {
        if (size)
            return __malloc(size);
        else
            return NULL;
    }

    /* Allow use realloc as malloc */
    if (ptr == NULL)
        return __malloc(size);

    /* Allow use realloc as free */
    if (size == 0)
        return __free(ptr), NULL;

    /* Header of block to realloc */
    hptr = (header_t *) ptr - 1;

    /* New size is smaller or equal than previous one */
    if (hptr->size >= size)
        return NULL;

    /* Try to allocate just after */
    newptr = realloc_use_next(ptr, size, hptr);
    if (newptr)
        return newptr;

    /* Try to allocate just before */
    newptr = realloc_use_prev(ptr, size, hptr);
    if (newptr)
        return newptr;

    /* A third case can happened, when using previous node
     * and next node at the same. As this operation is hard
     * to implement and hard to happend, a default realloc
     * is done. (Hugo's lazzy approach #3). */

    /* Default: Allocate new block and move data, then free current block */

    newptr = __malloc(size);
    if (!newptr)
        /* out-of-memory */
        return NULL;

    memmove(newptr, ptr, hptr->size);
    __free(ptr);

    return newptr;
}
