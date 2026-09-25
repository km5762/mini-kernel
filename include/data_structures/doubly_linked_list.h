#pragma once

struct list_double_link {
  struct list_double_link *next;
  struct list_double_link *previous;
};

static inline void doubly_linked_list_push(struct list_double_link **list,
                                           struct list_double_link *link) {
  link->previous = nullptr;
  link->next = *list;
  if (*list) {
    (*list)->previous = link;
  }
  *list = link;
}

static inline struct list_double_link *
doubly_linked_list_pop(struct list_double_link **list) {
  struct list_double_link *head = *list;

  if (!head) {
    return nullptr;
  }

  *list = head->next;
  return head;
}

static inline struct list_double_link *
doubly_linked_list_unlink(struct list_double_link **list,
                          struct list_double_link *link) {
  if (link->previous) {
    link->previous->next = link->next;
  } else {
    *list = link->next;
  }

  if (link->next) {
    link->next->previous = link->previous;
  }

  return link;
}
