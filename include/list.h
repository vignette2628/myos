#ifndef __LIST_H__


#define __LIST_H__

struct list_head
{
    struct list_head *next, *prev;
};

#define INIT_LIST_HEAD(ptr)     \
do { \
    (ptr)->next = (ptr); \
    (ptr)->prev = (ptr); \
} while (0)


#define list_add(new_node, prev_node, next_node) do {\
    (next_node)->prev = (new_node);      \
    (new_node)->next = (next_node);      \
    (new_node)->prev = (prev_node);      \
    (prev_node)->next = (new_node);      \
} while (0)


#define list_del(entry) do {  \
    (entry)->next->prev = (entry)->prev;    \
    (entry)->prev->next = (entry)->next;    \
} while (0)


static inline void list_add_tail(struct list_head *new, struct list_head *head)
{
	list_add(new, head->prev, head);
}

#define list_for_each(pos, head, member) \
	for (pos = list_entry((head)->next, typeof(*pos), member); \
			&pos->member != (head); \
		pos = list_entry(pos->member.next, typeof(*pos), member))


#define list_entry(ptr, type, member)   \
    ((type *)((unsigned long)(ptr) - (unsigned long)(&((type *)0)->member)))


static inline int list_empty(const struct list_head *head)
{
	return head->next == head;
}

#endif
