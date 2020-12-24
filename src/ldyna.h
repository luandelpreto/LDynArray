// C Header file:
//              ldyna.h
//
#ifndef __LDYNA_H__
#define __LDYNA_H__ 1

#include <stddef.h>
#include <stdbool.h>

typedef struct _ldyna ldyna;

typedef int (*ldyna_compare)(const void *key1, const void *key2);

typedef struct {
    bool inbulk;  // indicates if an inbulk adding is enabled
} ldyna_inbulk;

typedef enum {
    LDYNA_NONE = 0,
    LDYNA_SORT = 1 << 0,
    // TODO: LDYNA_THREAD_SAFE
} ldyna_flags;

enum {
    LDYNA_SUCCESS=0,
    LDYNA_NOT_FOUND,
    LDYNA_NULLPTR_WARN,
    LDYNA_INBULK_WARN,
    LDYNA_REALLOC_ERR,
};

//---------------------------------
// Public Interface
//---------------------------------

/************************************************************
 * \brief  Create a new dynamic array, given initial callback
 *         functions  and  flags.  If  no list_equal callback
 *         is provided, it  defaults  to  pointer  address of
 *         elements comparison.
 *         NOTE: it is responsabiliey of  the  caller to free
 *         any  dynamic  allocated  memory  upon   exit.  The
 *         memory allocated by this function can be  released
 *         with a call to ldyna_destroy (see below).
 *
 * \param compare  the pointer to compare function
 * \param flags    the initial flags
 *
 * \return  a pointer to a new ldyna if successfull
 * \return  NULL, otherwise
 ************************************************************/
extern ldyna *ldyna_create(size_t esize, ldyna_compare compare, ldyna_flags flags);

/************************************************************
 * \brief  Destroys the given dynamic array.
 *
 * \param list  a pointer to the dynamic array to be destroyed
 *
 * \return LDYNA_SUCCESS       if successful
 * \return LDYNA_NULLPTR_WARN  if list is NULL
 ************************************************************/
extern int ldyna_destroy(ldyna **list);

/************************************************************
 * \brief  Returns the length of the dynamic array.
 *
 * \param list  the dynamic array whose length will be returned
 *
 * \return  the length of the list, that is, the count of elements
 *          in the list
 ************************************************************/
extern size_t ldyna_len(ldyna *list);

/************************************************************
 * \brief  Appends an object to the list.
 *
 * \param list  the list to which the object will be appended
 * \param data  the object to be appended
 * \param inbulk  struct with the flag indicating inbulk add
 *
 * \return LDYNA_SUCCESS       if successful
 * \return LDYNA_NULLPTR_WARN  if list is NULL or data is
 *                                 NULL
 ************************************************************/
extern int ldyna_append(ldyna *list, void *data, ldyna_inbulk inbulk);

/************************************************************
 * \brief  Inserts an object in the list.
 *
 * \param list    the list to which the object will be inserted
 * \param data    the object to be inserted
 * \param idx     the index where the object will  be  inserted.
 *                If this index is out  of range, the insertion
 *                happens as an appending.
 * \param inbulk  struct with the flag indicating inbulk add
 *
 * \return LDYNA_SUCCESS       if successful
 * \return LDYNA_NULLPTR_WARN  if list is NULL or data is
 *                                 NULL
 ************************************************************/
extern int ldyna_insert(ldyna *list, void *data, size_t idx, ldyna_inbulk inbulk);

/************************************************************
 * \brief  Removes an object at index 'idx' from the  dynamic
 *         array. If 'idx' is out of range, removes the  last
 *         element.
 *
 * \param list  the dynamic array
 * \param idx   the index of the array from which the  object
 *              will be removed
 * \param data  returns the removed element as an out parameter
 *
 * \return LDYNA_SUCCESS         if successful
 * \return LDYNA_NULLPTR_WARN    if list is NULL
 ************************************************************/
extern int ldyna_remove(ldyna *list, size_t idx, void *data);

/************************************************************
 * \brief  This function serves two purposes. First, it tells
 *         if an object exists in  the  list. If  it do exist,
 *         it returns its index as an output parameter.
 *
 * \param list  the dynamic array to be searched
 * \param data  the object to be searched
 * \param idx   an output parameter that will contain the index
 *              of the object if it exists in the list
 * \param inbulk  struct with the flag indicating inbulk add
 *
 * \return LDYNA_SUCCESS  if the object represented by 'data' exists in
 *                            the list. In this case, its index is returned
 *                            as the output parameter 'idx'
 * \return LDYNA_NULLPTR_WARN  if list is NULL or data is NULL
 * \return LDYNA_NOT_FOUND     if the object does not exist in the list
 ************************************************************/
extern int ldyna_index_of(ldyna *list, void *data, size_t *idx, ldyna_inbulk inbulk);

/************************************************************
 * \brief  Returns the object at index 'idx' in  the  dynamic
 *         array. The object continues in the  list. if 'idx'
 *         is out of range,  returns the  last element in the
 *         array.
 *
 * \param list  the dyamic array
 * \param idx   the index in which the desired object is
 * \param data  returns the element at index 'idx' as an  out
 *              parameter
 *
 * \return  LDYNA_SUCCESS       if successful
 * \return  LDYNA_NULLPTR_WARN  if list is NULL  or 'idx'
 *                                  out of the list range
 ************************************************************/
extern int ldyna_get(ldyna *list, size_t idx, void *data);

//-----------------------------------------------------------
// When the list was created by  passing  the  flag that will
// tells us that it is a  sorted list,  it will automatically
// sort the entries  when they're added. If there is a  large
// amount of data that's going to be added, it is possible to
// optimize adding by delaying the sort  until everything has
// been added.

/************************************************************
 * \brief  This function suspends auto sorting when  the list
 *         is created  as  a  sorted  list.  While this  bulk
 *         insert is enabled, the list  should  be  used only
 *         for adding items to the list.
 *
 * \param list    the sorted list
 * \param inbulk  struct with the flag indicating inbulk add
 *
 * \return LDYNA_SUCCESS       if successful
 * \return LDYNA_NULLPTR_WARN  if list is NULL
 ************************************************************/
extern int ldyna_start_bulk_add(ldyna *list, ldyna_inbulk *restrict inbulk);

/************************************************************
 * \brief  This function reenables sorting when adding  items
 *         to the sorted  list  (that  was suspended  by  the
 *         function ldyna_start_bulk_add).
 *
 * \param list    the sorted list
 * \param inbulk  struct with the flag indicating inbulk add
 ************************************************************/
extern int ldyna_end_bulk_add(ldyna *list, ldyna_inbulk *restrict inbulk);

/************************************************************
 * \brief  Returns a new copy of the dynamic array. NOTE: this
 *         allocates heap memory and its responsability of the
 *         caller to do cleanup.
 *
 * \param list     the list to be copied
 * \param inbulk   the struct containing the inbulk flag
 *
 * \return a pointer to a new allocated list     if successful
 * \return NULL                                  otherwise
 ************************************************************/
extern ldyna *ldyna_copy(ldyna *list, ldyna_inbulk inbulk);

/************************************************************
 * \brief  Sort the dynamic array. This sets the dynamic array
 *         comparison  (list_equal)  function  to  compare  if
 *         compare is non-NULL. Non-stable sorting.
 *
 * \param list     the dynamic array to be sorted
 * \param compare  the comparison function that defines the sorting
 *
 * \return LDYNA_SUCCESS       if successful
 * \return LDYNA_NULLPTR_WARN  if list is NULL
 ************************************************************/
extern int ldyna_sort(ldyna *list, ldyna_compare compare);

#endif
