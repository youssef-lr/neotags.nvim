#include "neotags.h"
#include <assert.h>
#include <stdlib.h>
#include <string.h>

static struct Node * getnode_at_index(struct linked_list *list, int64_t index);
static void remove_node(struct linked_list *list, struct Node *node);

/* Note that contrary to its name, in this case `can_free_data` refers to the
 * string component of the struct string *, not the struct itself. The struct is
 * always free'd. The intention is to keep things adaptable. This macro could
 * change for different datatypes, and the field itself need not be boolean. */

#define FREE_DATA(LL_, NODE_)               \
    do {                                    \
            if ((LL_)->can_free_data)       \
                    free((NODE_)->data->s); \
            free((NODE_)->data);            \
    } while (0)


struct linked_list *
new_list(uint8_t can_free_data)
{
        struct linked_list *list = xmalloc(sizeof *list);
        list->head = list->tail = NULL;
        list->size = 0;
        list->can_free_data = can_free_data;
        return list;
}


void
ll_add(struct linked_list *list, LLTYPE data)
{
        struct Node *node = xmalloc(sizeof *node);

        if (list->head != NULL)
                list->head->prev = node;
        if (list->tail == NULL)
                list->tail = node;

        node->data = data;
        node->prev = NULL;
        node->next = list->head;
        list->head = node;
        ++list->size;
}


void
ll_append(struct linked_list *list, LLTYPE data)
{
        struct Node *node = xmalloc(sizeof *node);

        if (list->tail != NULL)
                list->tail->next = node;
        if (list->head == NULL)
                list->head = node;

        node->data = data;
        node->prev = list->tail;
        node->next = NULL;
        list->tail = node;
        ++list->size;
}


LLTYPE
_ll_popat(struct linked_list *list, long index, enum ll_pop_type type)
{
        struct Node *node = getnode_at_index(list, index);

        switch (type) {
        case DEL_ONLY:
                remove_node(list, node);
                return NULL;
        case RET_ONLY:
                return node->data;
        case BOTH:
                ; LLTYPE data = node->data;
                remove_node(list, node);
                return data;
        default:
                errx(1, "Unreachable!\n");
        }
}


bool
ll_find_str(struct linked_list *list, char *str)
{
        struct Node *current = list->head;
        bool ret = false;

        while (current != NULL) {
                /* if (streq(str+1, current->data->s+1)) { */
                if (streq(str, current->data->s)) {
                        ret = true;
                        break;
                }
                current = current->next;
        }
        return ret;
}


void
destroy_list(struct linked_list *list)
{
        if (list->size == 1) {
                struct Node *current;
                if (list->tail != NULL)
                        current = list->tail;
                else
                        current = list->head;
                FREE_DATA(list, current);
                free(current);
        } else if (list->size > 1) {
                struct Node *current = list->head;
                do {
                        struct Node *tmp = current;
                        current = current->next;
                        FREE_DATA(list, tmp);
                        free(tmp);
                } while (current != NULL);
        }

        free(list);
}


static struct Node *
getnode_at_index(struct linked_list *list, int64_t index)
{
        assert(list->size > 0);

        if (index == 0)
                return list->head;
        if (index == -1)
                return list->tail;

        if (index < 0)
                index += list->size;
        assert(index >= 0 && index < list->size);

        struct Node *current;

        /* If index is less than the size of the list divided by 2 (ie is in the
         * first half of the list) start the search from the head, otherwise
         * start from the tail. */
        if (index < ((list->size - 1) / 2)) {
                long x = 0;
                current = list->head;

                while (x++ != index)
                        current = current->next;
        } else {
                long x = list->size - 1;
                current = list->tail;

                while (x-- != index)
                        current = current->prev;
        }

        return current;
}


static void
remove_node(struct linked_list *list, struct Node *node)
{
        if (list->size == 1) {
                list->head = list->tail = NULL;
        } else if (node == list->head) {
                list->head = node->next;
                list->head->prev = NULL;
        } else if (node == list->tail) {
                list->tail = node->prev;
                list->tail->next = NULL;
        } else {
                node->prev->next = node->next;
                node->next->prev = node->prev;
        }
        --list->size;
        free(node);
}