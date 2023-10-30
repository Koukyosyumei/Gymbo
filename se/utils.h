#pragma once
#include <bitset>
#include <cstdint>
#include <iostream>
#include <string>

// Import statements are not directly translatable to C++.
// Include the necessary libraries directly.

inline int wordToInt(uint32_t word) { return static_cast<int>(word); }

inline int wordToSignedInt(uint32_t word) {
  if (word & 0x80000000) {
    return -static_cast<int>(wordToSignedInt(~word + 1));
  } else {
    return static_cast<int>(word);
  }
}

inline bool wordToBool(uint32_t word) { return word != 0; }

inline uint32_t boolToWord(bool value) { return value ? 1 : 0; }

inline uint32_t twosComplement(uint32_t i) { return 1 + ~i; }

inline bool isNegative(uint32_t word) { return (word & 0x80000000) != 0; }

inline std::string valName(int i) { return "val_" + std::to_string(i); }

template <typename T> class LLNode {
public:
  T data;
  LLNode *next;
  LLNode *prev;

  // Default constructor
  LLNode(T data) : data(data), next(NULL), prev(NULL) {}
};

// Linked list class to
// implement a linked list.
template <typename T> class Linkedlist {
public:
  LLNode<T> *ghost;
  LLNode<T> *head;
  LLNode<T> *tail;

  // Default constructor
  Linkedlist() {
    ghost = NULL;
    head = NULL;
    tail = NULL;
  }

  uint32_t len() {
    LLNode<T> *tmp = head;
    uint32_t cnt = 0;
    while (tmp != NULL) {
      cnt++;
      tmp = tmp->next;
    }
    return cnt;
  }

  void push(T data) {
    // Create the new Node.
    LLNode<T> *newNode = new LLNode<T>(data);

    // Assign to head
    if (head == NULL) {
      head = newNode;
      tail = head;
      return;
    }

    tail->next = newNode;
    newNode->prev = tail;
    tail = newNode;
  }

  T *back() {
    if (tail == NULL) {
      fprintf(stderr, "Warning... tail of linked-list is null\n");
    }
    return &(tail->data);
  }

  void pop() {
    if (tail == NULL) {
      fprintf(stderr, "Warning... tail is NULL when trying to pop\n");
      return;
    }

    LLNode<T> *current_tail = tail;
    tail = tail->prev;
    if (tail != NULL) {
      tail->next = NULL;
    }

    if (current_tail == head) {
      head = NULL;
    }

    if (ghost == NULL) {
      ghost = current_tail;
    } else {
      ghost->next = current_tail;
      current_tail->prev = ghost;
      ghost = tail;
    }
  }
};
