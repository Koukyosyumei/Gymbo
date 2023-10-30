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
  LLNode<T> *ghost;
  LLNode<T> *head;
  LLNode<T> *tail;

public:
  // Default constructor
  Linkedlist() {
    ghost = NULL;
    head = NULL;
    tail = NULL;
  }
  void push(T data) {
    // Create the new Node.
    LLNode<T> *newNode = new LLNode<T>(data);

    // Assign to head
    if (head == NULL) {
      head = newNode;
      tail = newNode;
      return;
    }

    tail->next = newNode;
    newNode->prev = tail;
    tail = newNode;
  }

  T *back() {
    if (tail == NULL) {
      fprintf(stderr, "tail of linked-list is null\n");
    }
    return &(tail->data);
  }

  void pop() {
    if (tail == NULL) {
      return;
    }
    if (ghost == NULL) {
      ghost = tail;
    } else {
      ghost->next = tail;
      ghost = tail;
    }
    tail = tail->prev;
    tail->next = NULL;
  }
};
