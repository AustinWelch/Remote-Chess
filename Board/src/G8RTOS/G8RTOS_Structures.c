#include "G8RTOS_Structures.h"

void tcbll_Insert(TCBLL* list, TCB* toInsert) {
    TCB* left;
    TCB* right;

    if (list->size != 0) {
        left = list->head->previous;
        right = list->head;

        left->next = toInsert;
        right->previous = toInsert;
    } else {
        // Set up circular list if the first item
        left = toInsert;
        right = toInsert;

        list->head = toInsert;
    }

    toInsert->previous = left;
    toInsert->next = right;

    list->size++;
}

void tcbll_Remove(TCBLL* list, TCB* toRemove) {
    // Check if only item in linked list
    if (list->size > 1) {
        // Change head if we're removing the head and multiple items in list
        if (list->head == toRemove)
            list->head = toRemove->next;

        TCB* left = toRemove->previous;
        TCB* right = toRemove->next;

        left->next = right;
        right->previous = left;
    } else {
        // Make the head NULL if now empty
        list->head = NULL;
    }

    toRemove->next = NULL;
    toRemove->previous = NULL;

    list->size--;
}