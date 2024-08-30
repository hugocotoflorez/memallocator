# Mem Allocator

## About

This memory allocation interface can be used to
request a free chunk of memory and use it as wanted.
Once memory is no longer required it is needes to return it
throw the corresponding function call. Usage are specified below.

> [!NOTE]
> This memory managment interface is the new version of
> *hallocator*, my previous malloc library. Now memory is
> limitless (At least can grow dynamically), calloc is implemented
> and some other features that are hiden from user.

> [!CAUTION]
> Note for users. If you noticed a strange behaviour while
> using the interface, and you think it is caused by the
> implementation, please open an issue or contact me
> (contact info in my profile). Thanks for your patience.

## Name
malloc, free, calloc, realloc - Allocate and free dynamic memory.

## Synopsis
- `void *malloc(size_t size);`
- `void free(void *_Nullable ptr);`
- `void *calloc(size_t nmemb, size_t size);`
- `void *realloc(void *_Nullable ptr, size_t size);`


## Description
- `malloc()`:
   The malloc() function allocates size bytes and returns a pointer
   to  the  allocated  memory.   The memory is not initialized. Size must be
   grater than 0 (Non-gnu approach).

- `free()`:
   The  free()  function  frees the memory space pointed to by ptr,
   which must have been returned by a previous call to malloc()  or
   related functions.  Otherwise, or if ptr has already been freed,
   undefined behavior occurs.  If ptr is NULL, no operation is performed.

- `calloc()`:
   The calloc() function allocates memory for an array of nmemb elements of size
   bytes each and returns a pointer to the allocated
   memory.  The memory is set to zero. Size must be grater than 0 (Non-gnu approach).


- `realloc()`:
   The  realloc()  function  changes  the  size of the memory block
   pointed to by ptr to size bytes.  The  contents  of  the  memory
   will  be  unchanged in the range from the start of the region up
   to the minimum of the old and new sizes.  If  the  new  size  is
   larger  than the old size, the added memory will not be initialized.

   If ptr is NULL, then the call is equivalent to malloc(size), for
   all values of size.

   If size is equal to zero, and ptr is not NULL, then the call  is
   equivalent  to  free(ptr).

   Unless ptr is NULL, it must have been  returned  by  an  earlier
   call to malloc or related functions.  If the area pointed to was
   moved, a free(ptr) is done.

## Return value

The  malloc(), calloc() and realloc() functions
return a pointer to the  allocated  memory,  which  is  suitably
aligned  for any type that fits into the requested size or less.
On error, these functions return NULL and set errno.  Attempting
to allocate more than PTRDIFF_MAX bytes is considered an  error,
as an object that large could cause later pointer subtraction to
overflow.

The free() function returns no value, and preserves errno.

The realloc() function return NULL if ptr is
not  NULL and the requested size is zero; this is not considered
an error.
Otherwise,  the  returned  pointer may be the same as ptr if the
allocation was not moved (e.g., there was room to expand the allocation in-place),
or different from ptr if the allocation  was
moved  to  a new address.  If these functions fail, the original
block is left untouched; it is not freed or moved.

## Errors

calloc(), malloc() and realloc() can fail  with
the following error:

`ENOMEM` Out   of  memory.

Have to check errno when interface returns NULL,
no errors make the program exit.

## Attributes

Functions are not thread-safe, lock have to be managed externly.
Sorry.

## Standards

malloc(), free(), calloc(), realloc()

      C11, POSIX.1-2008.

