/*
	This is free and unencumbered software released into the public domain.

	Anyone is free to copy, modify, publish, use, compile, sell, or
	distribute this software, either in source code form or as a compiled
	binary, for any purpose, commercial or non-commercial, and by any
	means.

	In jurisdictions that recognize copyright laws, the author or authors
	of this software dedicate any and all copyright interest in the
	software to the public domain. We make this dedication for the benefit
	of the public at large and to the detriment of our heirs and
	successors. We intend this dedication to be an overt act of
	relinquishment in perpetuity of all present and future rights to this
	software under copyright law.

	THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
	EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
	MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
	IN NO EVENT SHALL THE AUTHORS BE LIABLE FOR ANY CLAIM, DAMAGES OR
	OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE,
	ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR
	OTHER DEALINGS IN THE SOFTWARE.

	For more information, please refer to <http://unlicense.org>
*/

#ifndef LIST_H_39MCA0BP
#define LIST_H_39MCA0BP

#include <types.h>
#include <kernel.h>

typedef struct list_t list_t;

struct list_t
{
	struct list_t *next;
	struct list_t *prev;
};


#define LIST_INIT(name) { &(name), &(name) }

#define LIST(name) \
	list_t name = LIST_INIT(name)

static inline void LIST_HEAD_INIT(list_t *list)
{
	list->next = list;
	list->prev = list;
}

static inline void list_add_at(list_t *new, list_t *prev, list_t *next)
{
	next->prev = new;
	new->next = next;
	new->prev = prev;
	prev->next =  new;
}

static inline void list_add(list_t *new, list_t *target)
{
	list_add_at(new, target, target->next);
}

static inline void list_add_tail(list_t *new, list_t *target)
{
	list_add_at(new, target->prev, target);
}

static inline list_t *list_remove_entry(list_t *target)
{
	// Mind blowing :)
	target->prev->next = target->next;
	target->next->prev = target->prev;

	target->next = NULL;
	target->prev = NULL;

	return target;
}

static inline bool list_is_empty(const list_t *list)
{
	return (list->next == list);
}
/**
 * list_entry - get the struct for this entry
 * @ptr:	the &struct list_head pointer.
 * @type:	the type of the struct this is embedded in.
 * @member:	the name of the list_head within the struct.
 */
#define list_entry(ptr, type, member) \
	CONTAINER_OF(ptr, type, member)

/**
 * list_first_entry - get the first element from a list
 * @ptr:	the list head to take the element from.
 * @type:	the type of the struct this is embedded in.
 * @member:	the name of the list_head within the struct.
 *
 * Note, that list is expected to be not empty.
 */
#define list_first_entry(ptr, type, member) \
	list_entry((ptr)->next, type, member)

/**
 * list_last_entry - get the last element from a list
 * @ptr:	the list head to take the element from.
 * @type:	the type of the struct this is embedded in.
 * @member:	the name of the list_head within the struct.
 *
 * Note, that list is expected to be not empty.
 */
#define list_last_entry(ptr, type, member) \
	list_entry((ptr)->prev, type, member)

/**
 * list_next_entry - get the next element in list
 * @pos:    the type * to cursor
 * @member: the name of the list_head within the struct.
 */
#define list_next_entry(pos, member) \
	list_entry((pos)->member.next, typeof(*(pos)), member)


/**
 * list_for_each    -   iterate over a list
 * @pos:    the &struct list_head to use as a loop cursor.
 * @head:   the head for your list.
 */
#define list_for_each(pos, head) \
	for (pos = (head)->next; pos != (head); pos = pos->next)

/**
   * list_for_each_entry  -   iterate over list of given type
   * @pos:    the type * to use as a loop cursor.
   * @head:   the head for your list.
   * @member: the name of the list_head within the struct.
   */
#define list_for_each_entry(pos, head, member)                          \
	for (pos = list_first_entry(head, typeof(*pos), member);        \
	&pos->member != (head);                                    \
	pos = list_next_entry(pos, member))

#endif /* end of include guard: LIST_H_39MCA0BP */
