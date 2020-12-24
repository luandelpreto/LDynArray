// C file:
//       ldyna.c
//
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <assert.h>
#include "ldyna.h"

typedef unsigned char ldyna_Byte;

struct _ldyna {
    size_t allocs;          // actual number of objects in the array
    size_t len;             // total size of the array
    size_t esize;           // size of the element type stored in the list
    ldyna_compare compare;
    ldyna_flags flags;
    ldyna_Byte *array;
};

static const size_t ldyna_block_size = 61;

static void __std_msg(FILE *, const char *restrict, bool);
static int __default_compare(const void *, const void *);
static bool __bsearch_index_insert(const void *, size_t, size_t, const void *, size_t *, bool, int (*)(const void *, const void *));
static void __list_remove(ldyna *, size_t);

#define ldyna_perror(stream, func, msg, isstd)                          \
    fprintf(stream, "[ldyna]:%s:%s:%lu", __FILE__, func, __LINE__+0UL); \
    __std_msg(stream, msg, isstd);

static void __std_msg(FILE *stream, const char *restrict msg, bool isstd)
{
    if (isstd) {
        perror(msg);
    }
    else {
        fprintf(stream, msg);
    }
}

static int __default_compare(const void *key1, const void *key2)
{
    ldyna_Byte *sk1 = (ldyna_Byte *) key1;
    ldyna_Byte *sk2 = (ldyna_Byte *) key2;

    return sk1 - sk2;
}

static bool __bsearch_index_insert(const void *base, size_t nelems, size_t width, const void *key, size_t *idx, bool isinsert, int (*compare)(const void *, const void *))
{
    size_t left = 0;
    size_t right = nelems;
    while (left < right) {
        size_t mid = (left + right) / 2;
        if (compare(key, ((ldyna_Byte *) base) + mid * width) < 0) {
            right = mid;
        }
        else {
            left = mid + 1;
        }
    }
    size_t retidx = right - 1;

    int equal = compare(key, ((ldyna_Byte *) base) + retidx * width);
    if (isinsert) {
        if (equal > 0) { // insert after index
            ++retidx;
        }
        // Stable insertion
        while (retidx < right && !equal) {
            ++retidx;
            equal = compare(key, ((ldyna_Byte *) base) + retidx * width);
        }
    }
    else {
        if (equal) {   // not equal, can't find object
            return false;
        }
    }

    *idx = retidx;
    return true;
}

static void __list_remove(ldyna *list, size_t idx)
{
    list->len--;
    if (idx == list->len) {
        return;
    }

    memmove(list->array + idx * list->esize, list->array + (idx + 1) * list->esize, (list->len - idx) * list->esize);
}

ldyna *ldyna_create(size_t esize, ldyna_compare compare, ldyna_flags flags)
{
    assert(esize);

    ldyna *list = malloc(sizeof(*list));
    if (!list) {
        ldyna_perror(stderr, __func__, "malloc failed", true);
        return NULL;
    }
    // TODO: check size_t overflow
    list->array = malloc(sizeof(*list->array) * ldyna_block_size * esize);
    if (!list->array) {
        ldyna_perror(stderr, __func__, "malloc failed", true);
        return NULL;
    }

    list->allocs = ldyna_block_size;
    list->len = 0;
    list->esize = esize;
    list->flags = flags;

    if (compare) {
        list->compare = compare;
    }
    else {
        list->compare = __default_compare;
    }

    return list;
}

int ldyna_destroy(ldyna **list)
{
    if (!*list || !(*list)->array) {
        return LDYNA_NULLPTR_WARN;
    }

    free((*list)->array);
    (*list)->array = NULL;
    free(*list);
    list = NULL;
    return LDYNA_SUCCESS;
}

size_t ldyna_len(ldyna *list)
{
    if (!list) {
        return 0;
    }
    return list->len;
}

int ldyna_append(ldyna *list, void *data, ldyna_inbulk inbulk)
{
    return ldyna_insert(list, data, list->len, inbulk);
}

int ldyna_insert(ldyna *list, void *data, size_t idx, ldyna_inbulk inbulk)
{
    if (!list || !list->array || !data) {
        return LDYNA_NULLPTR_WARN;
    }
    if (inbulk.inbulk) {
        return LDYNA_INBULK_WARN;
    }

    // Need to grow the dynamic array
    if (list->allocs == list->len) {
        list->allocs += ldyna_block_size;
        // TODO: check for size_t overflow
        ldyna_Byte *tmp = realloc(list->array, sizeof(*tmp) * list->allocs * list->esize);
        if (!tmp) {
            ldyna_perror(stderr, __func__, "realloc failed", true);
            return LDYNA_REALLOC_ERR;
        }
        list->array = tmp;
    }

    if (list->flags & LDYNA_SORT) {
        __bsearch_index_insert(list->array, list->len, list->esize, data, &idx, true, list->compare);
    }

    if (idx > list->len) {
        idx = list->len;
    }
    else if (idx < list->len) {
        memmove(list->array + (idx+1) * list->esize, list->array + idx * list->esize, (list->len - idx) * list->esize);
    }

    memcpy(list->array + idx * list->esize, data, list->esize);
    list->len++;
    return LDYNA_SUCCESS;
}

int ldyna_remove(ldyna *list, size_t idx, void *data)
{
    assert(list->len);

    if (!list || !list->array) {
        return LDYNA_NULLPTR_WARN;
    }
    if (idx >= list->len) {
        idx = list->len - 1;
    }

    memcpy(data, list->array + idx * list->esize, list->esize);
    __list_remove(list, idx);
    return LDYNA_SUCCESS;
}

int ldyna_index_of(ldyna *list, void *data, size_t *idx, ldyna_inbulk inbulk)
{
    if (!list || !list->array || !data) {
        return LDYNA_NULLPTR_WARN;
    }
    if (inbulk.inbulk) {
        return LDYNA_INBULK_WARN;
    }

    // Sorted list
    if (list->flags & LDYNA_SORT) {
        if (__bsearch_index_insert(list->array, list->len, list->esize, data, idx, false, list->compare)) {
            return LDYNA_SUCCESS;
        }
        return LDYNA_NOT_FOUND;
    }

    // Non-sorted list
    bool found = false;
    size_t foundidx = list->len+1;
    for (ldyna_Byte *ptr = list->array; ptr != list->array + list->len * list->esize; ptr += list->esize) {
        if (!list->compare(data, ptr)) {
            found = true;
            foundidx = (ptr - list->array) / list->esize;
            break;
        }
    }
    if (found && idx) {
        *idx = foundidx;
    }
    else {
        return LDYNA_NOT_FOUND;
    }

    return LDYNA_SUCCESS;;
}

int ldyna_get(ldyna *list, size_t idx, void *data)
{
    assert(list->len);

    if (!list || !list->array || idx >= list->len) {
        return LDYNA_NULLPTR_WARN;
    }
    if (idx >= list->len) {
        idx = list->len - 1;
    }

    memcpy(data, list->array + idx * list->esize, list->esize);
    return LDYNA_SUCCESS;;
}

int ldyna_start_bulk_add(ldyna *list, ldyna_inbulk *restrict inbulk)
{
    if (!list) {
        return LDYNA_NULLPTR_WARN;
    }
    inbulk->inbulk = true;
    return LDYNA_SUCCESS;
}

int ldyna_end_bulk_add(ldyna *list, ldyna_inbulk *restrict inbulk)
{
    if (!list) {
        return LDYNA_NULLPTR_WARN;
    }
    inbulk->inbulk = false;
    return ldyna_sort(list, NULL);
}

ldyna *ldyna_copy(ldyna *list, ldyna_inbulk inbulk)
{
    if (!list || !list->array) {
        return NULL;
    }
    if (inbulk.inbulk) {
        return NULL;
    }

    ldyna *newarray = malloc(sizeof *newarray);
    if (!newarray) {
        ldyna_perror(stderr, __func__, "malloc failed", true);
        return NULL;
    }
    *newarray = *list;

    newarray->array = malloc(sizeof(*newarray->array) * newarray->allocs * newarray->esize);
    if (!newarray->array) {
        free(newarray);
        ldyna_perror(stderr, __func__, "malloc failed", true);
        return NULL;
    }
    memcpy(newarray->array, list->array, list->len * list->esize);
    return newarray;
}

int ldyna_sort(ldyna *list, ldyna_compare compare)
{
    if (!list || !list->array) {
        return LDYNA_NULLPTR_WARN;
    }

    if (list->flags & LDYNA_SORT) {
        if (!compare) {
            compare = list->compare;
        }
        else {
            list->compare = compare;
        }
    }

    qsort(list->array, list->len, list->esize, compare);
    return LDYNA_SUCCESS;
}
