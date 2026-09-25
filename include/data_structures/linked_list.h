#pragma once

#define LIST_LINK_TO_TYPE(ptr, type, member)                                   \
  ({                                                                           \
    void *__mptr = (void *)(ptr);                                              \
    ((type *)(__mptr - offsetof(type, member)));                               \
  })

struct list_link {
  struct list_link *next;
};

static inline void linked_list_push(struct list_link **list,
                                    struct list_link *link) {
  link->next = *list;
  *list = link;
}

static inline struct list_link *linked_list_pop(struct list_link **list) {
  struct list_link *head = *list;

  if (!head) {
    return nullptr;
  }

  *list = head->next;
  return head;
}
