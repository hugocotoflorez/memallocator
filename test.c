#include <stdio.h>
#include <unistd.h>

#define SHORTER
#include "malloc.h"

static void
print_mem_map()
{
    page_t   *pptr;
    node_t   *nptr;
    node_t   *best_fit = NULL;
    header_t *hptr;
    size_t    real_size;


    for (pptr = __debug_get_head(); pptr != NULL; pptr = pptr->next)
    {
        puts("+---------------------+");
        printf("|PAGE %1d (size: %4lu)  | %p [page_t]\n", pptr->page_id,
               pptr->size, pptr);
        printf("|prev: %14p | PAGE\n", pptr->prev);
        printf("|next: %14p |\n", pptr->next);
        puts("+---------------------+");

        nptr = (node_t *) ((char *) pptr + sizeof(page_t));

        do
        {
            printf("|size: %14lu | %p [node_t]\n", nptr->size, nptr);
            printf("|prev: %14p | FREE\n", nptr->prev);
            printf("|next: %14p |\n", nptr->next);
            printf("|page: %14p |\n", nptr->page);

#ifndef SHORTER
            puts("| - - - - - - - - - - |");
            if (nptr->size > 0)
            {
                puts("|free                 |");
                puts("+---------------------+");
            }
#else
            puts("+---------------------+");
#endif

            hptr = (void *) nptr + nptr->size + sizeof(node_t);

            while ((void *) hptr != (void *) nptr->next &&
                   (void *) hptr < (void *) __debug_get_head() + SIZE)
            {
                if (hptr->magic == MAGIC)
                {
                    printf("|size: %14lu | %p [header_t]\n", hptr->size, hptr);
                    printf("|magic: %13x | USED\n", hptr->magic);
                    printf("|page: %14p |\n", hptr->page);
#ifndef SHORTER
                    puts("| - - - - - - - - - - |");
                    puts("|used                 |");
#endif
                    puts("+---------------------+");
                    hptr = (void *) hptr + hptr->size + sizeof(header_t);
                }
                else
                {
                    printf("|size: %14lu | %p [node_t]\n",
                           ((node_t *) hptr)->size, hptr);
                    printf("|prev: %14p | FREE\n", ((node_t *) hptr)->prev);
                    printf("|next: %14p |\n", ((node_t *) hptr)->next);
                    printf("|page: %14p |\n", hptr->page);
#ifndef SHORTER
                    puts("| - - - - - - - - - - |");
                    puts("|free                 |");
#endif
                    puts("+---------------------+");
                    hptr = (void *) hptr + hptr->size + sizeof(node_t);
                }
            }
            nptr = nptr->next;

        } while (nptr != NULL);
    }
}

int
main(int argc, char *argv[])
{
    void *ptrs['z' - 'a' + 1] = { 0 };
    char  action;
    char  c;
    int   n;
    void *r;

    ptrs[0] = __malloc(100); // a
    ptrs[1] = __malloc(100); // b
    ptrs[2] = __malloc(400); // c

    while (1)
    {
        puts("\e[H\e[2J");

        print_mem_map();

        for (int i = 0; i < 'z' - 'a' + 1; i++)
            if (ptrs[i] != NULL)
                printf("[ptr] %c = %p\n", 'a' + i, ptrs[i]);

        puts("[m]: Malloc  id[a-z] size[0,...]    | [q]: Quit");
        puts("[r]: Realloc id[a-z] size[0,...]    | [c]: Clear");
        puts("[f]: Free    id[a-z]                |");
        printf("[>]: ");
        scanf(" %c", &action);
        switch (action)
        {
            case 'm':
                scanf(" %c %d", &c, &n);
                if (ptrs[c - 'a'] != NULL)
                    puts("Variable already used!");
                else
                    ptrs[c - 'a'] = __malloc(n);
                break;

            case 'r':
                scanf(" %c %d", &c, &n);
                ptrs[c - 'a'] =
                (r = __realloc(ptrs[c - 'a'], n)) ? r : ptrs[c - 'a'];
                break;

            case 'f':
                scanf(" %c", &c);
                __free(ptrs[c - 'a']);
                ptrs[c - 'a'] = NULL;
                break;

            case 'c':
                for (int i = 0; i < 'z' - 'a' + 1; i++)
                    if (ptrs[i] != NULL)
                    {
                        __free(ptrs[i]);
                        ptrs[i] = NULL;
                    }
                break;

            case 'q':
                return 0;

            default:
                fseek(stdin, 0, SEEK_END);
                break;
        }
    }

    return 0;
}
