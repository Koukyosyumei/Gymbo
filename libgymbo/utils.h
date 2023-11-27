#pragma once
#include <bitset>
#include <cstdint>
#include <iostream>
#include <string>
#include <cstring>

inline uint32_t FloatToWord(float val)
{
  uint32_t word;
  std::memcpy(&word, &val, sizeof(val));
  return word;
}
 
inline float wordToFloat(uint32_t word)
{
  float val;
  std::memcpy(&val, &word, sizeof(word));
  return val;
}

inline int wordToInt(uint32_t word) { return static_cast<int>(word); }

/**
 * Converts a uint32_t word to a signed int.
 *
 * @param word The uint32_t word to convert.
 * @return The signed int equivalent of the word.
 */
inline int wordToSignedInt(uint32_t word) {
  if (word & 0x80000000) {
    return -static_cast<int>(wordToSignedInt(~word + 1));
  } else {
    return static_cast<int>(word);
  }
}

/**
 * Converts a uint32_t word to a bool.
 *
 * @param word The uint32_t word to convert.
 * @return The bool equivalent of the word.
 */
inline bool wordToBool(uint32_t word) { return word != 0; }

/**
 * Converts a bool to a uint32_t word.
 *
 * @param value The bool to convert.
 * @return The uint32_t word equivalent of the bool.
 */
inline uint32_t boolToWord(bool value) { return value ? 1 : 0; }

/**
 * Returns the twos complement of a uint32_t word.
 *
 * @param i The uint32_t word to get the twos complement of.
 * @return The twos complement of the word.
 */
inline uint32_t twosComplement(uint32_t i) { return 1 + ~i; }

/**
 * Checks if a uint32_t word is negative.
 *
 * @param word The uint32_t word to check.
 * @return True if the word is negative, false otherwise.
 */
inline bool isNegative(uint32_t word) { return (word & 0x80000000) != 0; }

/**
 * Generates a name for a variable based on its index.
 *
 * @param i The index of the variable.
 * @return A name for the variable.
 */
inline std::string valName(int i) { return "val_" + std::to_string(i); }

template <typename T> class LLNode {
public:
  T data;
  LLNode *next;
  LLNode *prev;

  // Default constructor
  LLNode(T data) : data(data), next(NULL), prev(NULL) {}
};

/**
 * A linked list class to implement a linked list.
 *
 * @tparam T The type of data to store in the linked list.
 */
template <typename T> class Linkedlist {
public:
  LLNode<T> *ghost;
  LLNode<T> *head;
  LLNode<T> *tail;

  Linkedlist() : ghost(NULL), head(NULL), tail(NULL) {}

  /**
   * Returns the length of the linked list.
   *
   * @return The length of the linked list.
   */
  uint32_t len() {
    LLNode<T> *tmp = head;
    uint32_t cnt = 0;
    while (tmp != NULL) {
      cnt++;
      tmp = tmp->next;
    }
    return cnt;
  }

  /**
   * Pushes a new element onto the back of the linked list.
   *
   * @param data The element to push onto the linked list.
   */
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

  /**
   * Returns the element at the back of the linked list.
   *
   * @return The element at the back of the linked list.
   */
  T *back() {
    if (tail == NULL) {
      fprintf(stderr, "Warning... tail of linked-list is null\n");
    }
    return &(tail->data);
  }

  /**
   * Pops the element at the back of the linked list.
   */
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
