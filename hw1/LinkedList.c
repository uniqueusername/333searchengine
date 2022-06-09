/*
 * Copyright ©2022 Hal Perkins.  All rights reserved.  Permission is
 * hereby granted to students registered for University of Washington
 * CSE 333 for use solely during Spring Quarter 2022 for purposes of
 * the course.  No other use, copying, distribution, or modification
 * is permitted without prior written consent. Copyrights for
 * third-party components of this work must be honored.  Instructors
 * interested in reusing these course materials should contact the
 * author.
 */

// ary jha
// aryanjha@uw.edu <1902079>

#include <stdio.h>
#include <stdlib.h>

#include "CSE333.h"
#include "LinkedList.h"
#include "LinkedList_priv.h"


///////////////////////////////////////////////////////////////////////////////
// LinkedList implementation.

LinkedList* LinkedList_Allocate(void) {
  // Allocate the linked list record.
  LinkedList *ll = (LinkedList *) malloc(sizeof(LinkedList));
  Verify333(ll != NULL);

  // STEP 1: initialize the newly allocated record structure.
  ll->num_elements = 0;
  ll->head = NULL;
  ll->tail = NULL;

  // Return our newly minted linked list.
  return ll;
}

void LinkedList_Free(LinkedList *list,
                     LLPayloadFreeFnPtr payload_free_function) {
  Verify333(list != NULL);
  Verify333(payload_free_function != NULL);

  // STEP 2: sweep through the list and free all of the nodes' payloads
  // (using the payload_free_function supplied as an argument) and
  // the nodes themselves.
  while (list->head != NULL) {
    payload_free_function(list->head->payload);
    LinkedListNode *temp = list->head;
    LinkedListNode *next = temp->next;
    list->head = next;
    free(temp);
  }

  // free the LinkedList
  free(list);
}

int LinkedList_NumElements(LinkedList *list) {
  Verify333(list != NULL);
  return list->num_elements;
}

void LinkedList_Push(LinkedList *list, LLPayload_t payload) {
  Verify333(list != NULL);

  // Allocate space for the new node.
  LinkedListNode *ln = (LinkedListNode *) malloc(sizeof(LinkedListNode));
  Verify333(ln != NULL);

  // Set the payload
  ln->payload = payload;

  if (list->num_elements == 0) {
    // Degenerate case; list is currently empty
    Verify333(list->head == NULL);
    Verify333(list->tail == NULL);
    ln->next = ln->prev = NULL;
    list->head = list->tail = ln;
  } else {
    // STEP 3: typical case; list has >=1 elements
    Verify333(list->head != NULL);

    // node operations
    ln->next = list->head;
    ln->prev = NULL;
    list->head->prev = ln;

    // list operations
    list->head = ln;
  }

  list->num_elements++;
}

bool LinkedList_Pop(LinkedList *list, LLPayload_t *payload_ptr) {
  Verify333(payload_ptr != NULL);
  Verify333(list != NULL);

  // STEP 4: implement LinkedList_Pop.  Make sure you test for
  // and empty list and fail.  If the list is non-empty, there
  // are two cases to consider: (a) a list with a single element in it
  // and (b) the general case of a list with >=2 elements in it.
  // Be sure to call free() to deallocate the memory that was
  // previously allocated by LinkedList_Push().
  if (list->num_elements == 0) { return false; }

  Verify333(list->head != NULL);
  LinkedListNode *temp = list->head;
  *payload_ptr = list->head->payload;

  if (list->num_elements == 1) {
    // case a
    list->head = list->tail = NULL;
  } else {
    // case b
    list->head = temp->next;
    list->head->prev = NULL;
  }

  free(temp);
  list->num_elements--;
  return true;  // you may need to change this return value
}

void LinkedList_Append(LinkedList *list, LLPayload_t payload) {
  Verify333(list != NULL);

  // STEP 5: implement LinkedList_Append.  It's kind of like
  // LinkedList_Push, but obviously you need to add to the end
  // instead of the beginning.

  // allocate space for new node
  LinkedListNode *ln = (LinkedListNode *) malloc(sizeof(LinkedListNode));
  Verify333(ln != NULL);

  // set payload
  ln->payload = payload;

  if (list->num_elements == 0) {
    // empty list case
    Verify333(list->head == NULL);
    Verify333(list->tail == NULL);

    ln->next = ln->prev = NULL;
    list->head = list->tail = ln;
  } else {
    // non-empty list case
    Verify333(list->tail != NULL);

    list->tail->next = ln;
    ln->prev = list->tail;
    ln->next = NULL;
    list->tail = ln;
  }

  list->num_elements++;
}

void LinkedList_Sort(LinkedList *list, bool ascending,
                     LLPayloadComparatorFnPtr comparator_function) {
  Verify333(list != NULL);
  if (list->num_elements < 2) {
    // No sorting needed.
    return;
  }

  // We'll implement bubblesort! Nnice and easy, and nice and slow :)
  int swapped;
  do {
    LinkedListNode *curnode;

    swapped = 0;
    curnode = list->head;
    while (curnode->next != NULL) {
      int compare_result = comparator_function(curnode->payload,
                                               curnode->next->payload);
      if (ascending) {
        compare_result *= -1;
      }
      if (compare_result < 0) {
        // Bubble-swap the payloads.
        LLPayload_t tmp;
        tmp = curnode->payload;
        curnode->payload = curnode->next->payload;
        curnode->next->payload = tmp;
        swapped = 1;
      }
      curnode = curnode->next;
    }
  } while (swapped);
}


///////////////////////////////////////////////////////////////////////////////
// LLIterator implementation.

LLIterator* LLIterator_Allocate(LinkedList *list) {
  Verify333(list != NULL);

  // OK, let's manufacture an iterator.
  LLIterator *li = (LLIterator *) malloc(sizeof(LLIterator));
  Verify333(li != NULL);

  // Set up the iterator.
  li->list = list;
  li->node = list->head;

  return li;
}

void LLIterator_Free(LLIterator *iter) {
  Verify333(iter != NULL);
  free(iter);
}

bool LLIterator_IsValid(LLIterator *iter) {
  Verify333(iter != NULL);
  Verify333(iter->list != NULL);

  return (iter->node != NULL);
}

bool LLIterator_Next(LLIterator *iter) {
  Verify333(iter != NULL);
  Verify333(iter->list != NULL);
  Verify333(iter->node != NULL);

  // STEP 6: try to advance iterator to the next node and return true if
  // you succeed, false otherwise
  // Note that if the iterator is already at the last node,
  // you should move the iterator past the end of the list
  iter->node = iter->node->next;
  return LLIterator_IsValid(iter);
}

void LLIterator_Get(LLIterator *iter, LLPayload_t *payload) {
  Verify333(iter != NULL);
  Verify333(iter->list != NULL);
  Verify333(iter->node != NULL);

  *payload = iter->node->payload;
}

bool LLIterator_Remove(LLIterator *iter,
                       LLPayloadFreeFnPtr payload_free_function) {
  Verify333(iter != NULL);
  Verify333(iter->list != NULL);
  Verify333(iter->node != NULL);

  // STEP 7: implement LLIterator_Remove.  This is the most
  // complex function you'll build.  There are several cases
  // to consider:
  // - degenerate case: the list becomes empty after deleting.
  // - degenerate case: iter points at head
  // - degenerate case: iter points at tail
  // - fully general case: iter points in the middle of a list,
  //                       and you have to "splice".
  //
  // Be sure to call the payload_free_function to free the payload
  // the iterator is pointing to, and also free any LinkedList
  // data structure element as appropriate.
  LinkedList* list = iter->list;
  LinkedListNode* temp = iter->node;
  LLPayload_t payload = iter->node->payload;

  if (list->num_elements == 1 || temp->prev == NULL) {
    // one element / points to head case
    // action is the same; iterator will handle
    // it if we iterate into null
    LinkedList_Pop(list, &payload);
    iter->node = list->head;
  } else if (temp->next == NULL) {
    // points to tail case
    LinkedList_Slice(list, &payload);
    iter->node = list->head;
  } else {
    // splice and free the node
    temp->next->prev = temp->prev;
    temp->prev->next = temp->next;
    iter->node = temp->next;
    free(temp);
    list->num_elements--;
  }

  payload_free_function(payload);
  return list->num_elements != 0;
}


///////////////////////////////////////////////////////////////////////////////
// Helper functions

bool LinkedList_Slice(LinkedList *list, LLPayload_t *payload_ptr) {
  Verify333(payload_ptr != NULL);
  Verify333(list != NULL);

  // STEP 8: implement LinkedList_Slice.i
  if (list->num_elements == 0) { return false; }

  Verify333(list->tail != NULL);
  *payload_ptr = list->tail->payload;

  if (list->num_elements == 1) {
    return LinkedList_Pop(list, payload_ptr);
  } else {
    LinkedListNode *temp = list->tail;
    list->tail = temp->prev;
    list->tail->next = NULL;
    free(temp);
  }

  list->num_elements--;
  return true;  // you may need to change this return value
}

void LLIterator_Rewind(LLIterator *iter) {
  iter->node = iter->list->head;
}
