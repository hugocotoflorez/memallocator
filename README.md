# Mem Allocator

## About

This memory allocation interface allows an user to
request a free chunk of memory and use it as user wants.
Once memory is no longer required user have to return it
throw the corresponding function. Usage are specified below.

> [!NOTE]
> This memory managment interface is the new version of
> *hallocator*, my previous malloc library. Now memory
> limitless (hardware dependent), calloc is implemented
> and other features that are hiden to user.

> [!CAUTION]
> Note for users. If you noticed a strange behaviour while
> using the interface, and you thing is caused by the
> implementation, please open an issue or contact me
> (contact info in my profile). Thanks for your patience.

## Name
malloc,  free, calloc, realloc -
allocate and free dynamic memory

## Synopsis
- void *malloc(size_t size);
- void free(void *_Nullable ptr);
- void *calloc(size_t nmemb, size_t size);
- void *realloc(void *_Nullable ptr, size_t size);


## Description
- `malloc()`

   The malloc() function allocates size bytes and returns a pointer
   to  the  allocated  memory.   The memory is not initialized.  If
   size is 0, then malloc() returns a unique pointer value that can
   later be successfully passed to free().  (See  "Nonportable  be‐
   havior" for portability issues.)

- `free()`

   The  free()  function  frees the memory space pointed to by ptr,
   which must have been returned by a previous call to malloc()  or
   related functions.  Otherwise, or if ptr has already been freed,
   undefined behavior occurs.  If ptr is NULL, no operation is per‐
   formed.

- `calloc()`

   The calloc() function allocates memory for an array of nmemb el‐
   ements of size bytes each and returns a pointer to the allocated
   memory.  The memory is set to zero.  If nmemb or size is 0, then
   calloc()  returns  a unique pointer value that can later be suc‐
   cessfully passed to free().

   If the multiplication of nmemb and size would result in  integer
   overflow, then calloc() returns an error.  By contrast, an inte‐
   ger overflow would not be detected in the following call to mal‐
   loc(), with the result that an incorrectly sized block of memory
   would be allocated:

       malloc(nmemb * size);

- `realloc()`

   The  realloc()  function  changes  the  size of the memory block
   pointed to by ptr to size bytes.  The  contents  of  the  memory
   will  be  unchanged in the range from the start of the region up
   to the minimum of the old and new sizes.  If  the  new  size  is
   larger  than the old size, the added memory will not be initial‐
   ized.

   If ptr is NULL, then the call is equivalent to malloc(size), for
   all values of size.

   If size is equal to zero, and ptr is not NULL, then the call  is
   equivalent  to  free(ptr)  (but  see  "Nonportable behavior" for
   portability issues).

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
an error.  (See "Nonportable behavior" for portability  issues.)
Otherwise,  the  returned  pointer may be the same as ptr if the
allocation was not moved (e.g., there was room to expand the al‐
location in-place), or different from ptr if the allocation  was
moved  to  a new address.  If these functions fail, the original
block is left untouched; it is not freed or moved.

## Errors

calloc(), malloc() and realloc() can fail  with
the following error:

`ENOMEM` Out   of  memory.   Possibly,  the  application  hit  the
      RLIMIT_AS or RLIMIT_DATA limit described in getrlimit(2).
      Another reason could be that the number of mappings  cre‐
      ated  by  the caller process exceeded the limit specified
      by /proc/sys/vm/max_map_count.

## Attributes

Functions are not thread-safe, lock have to be managed externly.
Sorry.

## Standards
malloc()

free()

calloc()

realloc()

      C11, POSIX.1-2008.

