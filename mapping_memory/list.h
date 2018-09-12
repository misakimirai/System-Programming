/**
 * Mapping Memory Lab
 * CS 241 - Fall 2017
 */
#pragma once
#include <stdlib.h>

/**
 * Use this macro to remove some struct from an internally contained
 * linked list.
 *
 * -`structure` is the structure that needs to be unlinked
 * -`pprev_member` is name of the left linked list pointer within the
 *  structure
 * -`next_member` is the name of the right linked list pointer within the
 *  structure.
 *
 * For example, here is how you'd remove the stack segment from the virtual
 * address space of the current process.
 * (see virtual_memory.h).
 *
 * ```
 * list_unlink(current->stack, area_prev, area_next);
 * ```
 */
#define list_unlink(structure, pprev_member, next_member)                      \
    {                                                                          \
        typeof(structure) $current = (structure);                              \
        if ($current->next_member)                                             \
            $current->next_member->pprev_member = $current->pprev_member;      \
        if ($current->pprev_member)                                            \
            *$current->pprev_member = $current->next_member;                   \
        $current->next_member = NULL;                                          \
        $current->pprev_member = NULL;                                         \
    }

/**
 * Use this macro to add some struct to a nonempty linked list. This macro
 * will never change the head of the linked list. In order to push a node to
 * the head of a linked list, use `list_link_before`.
 *
 * -`new` is the element that needs to be added to the linked list
 * -`prev` is the non-NULL struct that precedes `new` in the linked list
 * -`pprev_member` refers to the name of the struct member that corresponds to
 *  the reverse pointer pointer in the linked list
 * -`next_member` refers to the name of the struct member that corresponds to
 *  the forward pointer in the linked list
 *
 * For example, here is how you'd insert a `vm_area` struct right after another
 * `vm_area` in an existing, nonempty linked list
 *
 * ```
 * struct vm_area *to_be_inserted;
 * struct vm_area *existing_list_node;
 *
 * list_link(to_be_inserted, existing_list_node, area_prev, area_next);
 * ```
 */
#define list_link_after(new, prev, pprev_member, next_member)                  \
    {                                                                          \
        typeof(new) $new = (new), $prev = (prev);                              \
        $new->pprev_member = &$prev->next_member;                              \
        $new->next_member = $prev->next_member;                                \
        if ($prev->next_member) {                                              \
            $prev->next_member->pprev_member = &$new->next_member;             \
        }                                                                      \
        $prev->next_member = new;                                              \
    }

/**
 * Use this macro to add some struct to a possibly empty linked list. This
 * macro will insert a new linked list node before the specified target node.
 * If the target node is an l-value reference to an empty embedded linked
 * list pointer, then the item inserted will be the new head of the linked
 * list.
 *
 * -`new` is the element that needs to be added to the linked list
 * -`next_prev` is the `pprev_member` of the target node, before
 *  which the `new` node will be inserted into the linked list, or a pointer to
 *  the embedded link list head pointer; if `next_prev` points to the
 *  embedded linked list head pointer, then `new` becomes the new list head
 * -`pprev_member` refers to the name of the struct member that corresponds to
 *  the reverse pointer pointer in the linked list
 * -`next_member` refers to the name of the struct member that corresponds to
 *  the forward pointer in the linked list
 *
 * For example, here is how you'd append a `vm_area` struct to the front of the
 * `process->areas` linked list (see virtual_memory.h).
 *
 * ```
 * struct vm_area *my_vma;
 * struct process *my_proc;
 * list_link_before(my_vma, &my_proc->area_list, area_prev, area_next);
 * ```
 *
 * and here is how you'd insert a `vm_area` struct right before a
 * `vm_area` in an existing, nonempty linked list
 *
 * ```
 * struct vm_area *to_be_inserted;
 * struct vm_area *next_list_node;
 *
 * list_link(to_be_inserted, next_list_node->area_prev, area_prev, area_next);
 * ```
 */
#define list_link_before(new, next_pprev, pprev_member, next_member)           \
    {                                                                          \
        typeof(next_pprev) $next_pprev = (next_pprev);                         \
        typeof(new) $new = (new);                                              \
        $new->pprev_member = $next_pprev;                                      \
        $new->next_member = *$next_pprev;                                      \
        if (*$next_pprev) {                                                    \
            (*$next_pprev)->pprev_member = &new->next_member;                  \
        }                                                                      \
        *$next_pprev = $new;                                                   \
    }

/**
 * List iterator macro. Below is an example used to iterate through every VMA
 * in a `struct process` and print out each virtual address range.
 *
 * ```
 * list_for_each(proc->area_list, area_next, elem){
 *   printf("%08x-%08x\n", elem->start_addr, elem->end_addr);
 * }
 * ```
 *
 * Note that it is not legal to remove the current list node from the linked
 * list while using this iterator, unless `varname` is properly moved after
 * doing so.
 */
#define list_for_each(list_start, next_member, varname)                        \
    for (typeof(list_start) varname = (list_start); varname;                   \
         varname = varname->next_member)

/**
 * This gets the actual linked list node that precedes a struct contained in
 * some internal linked list, provided that there exists a preceding struct.
 * Note that the `pprev_member` does not actually point to the previous struct,
 * but rather the previous struct's next pointer. So this macro uses some
 * hackery to get the real previous struct..
 *
 * -`list_node` is the node whose predecessor you want to obtain
 * -`list_head` is the first element in the linked list
 *
 * Returns an undefined value if there is no previous element.
 *
 * Usage examples (`vma1` ends up being NULL, while `vma2` ends up being
 * `current->area_list`):
 *
 * ```
 *  struct vm_area *vma0 = current->area_list; // assume not NULL
 *  struct vm_area *vma1 =
 *    list_prev(vma0, area_prev, area_next);
 *  struct vm_area *vma2 =
 *    list_prev(vma0->next, area_prev, area_next);
 * ```
 *
 * Note the use of GCC specific statement expressions to improve readability.
 * https://gcc.gnu.org/onlinedocs/gcc/Statement-Exprs.html
 */
#define list_prev(list_node, pprev_member, next_member)                        \
    ({                                                                         \
        typeof(list_node) $current = (list_node);                              \
        typeof(list_node) $output = NULL;                                      \
        if ($current->pprev_member) {                                          \
            size_t $off = ((void *)&$current->next_member - (void *)$current); \
            $output = (((void *)($current->pprev_member)) - $off);             \
        }                                                                      \
        $output;                                                               \
    })

/**
 * This is like `list_for_each`, but goes backward.
 *
 * -`list_start` is a pointer to the starting struct from which to
 *  reverse-iterate
 * -`list_head` is the linked list head pointer, which is
 *  NOT a struct of the same type as `list_tail` but rather a pointer to a
 *  struct pointer. WARNING: do not pass in an expression with side effects
 *  into this parameter
 * -`pprev_member` is the name of the struct member that refers to the previous
 *  struct in a linked list (or specifically to the previous struct's "next"
 *  pointer)
 * -`next_member` is the name of the struct member that refers to the next
 *  struct in a linked list
 * -`varname` is the name of the intended iterator variable
 *
 * Below is an example used to reverse-iterate through every VMA in a
 * `struct process` starting from the stack and printing out each virtual
 * address range.
 *
 * ```
 * list_rev_for_each(proc->stack_segment, proc->area_list, area_prev,
 *                   area_next, elem)
 * {
 *   printf("%08x-%08x\n", elem->start_addr, elem->end_addr);
 * }
 * ```
 */
#define list_rev_for_each(list_start, list_head, pprev_member, next_member,    \
                          varname)                                             \
    for (typeof(list_start) varname = (list_start), $end = (list_head);        \
         varname;                                                              \
         varname = varname == $end ? NULL : list_prev(varname, pprev_member,   \
                                                      next_member))
