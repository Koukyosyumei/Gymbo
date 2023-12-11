/**
 * @file utils.h
 * @brief Utility funcitons and classes.
 * @author Hideaki Takahashi
 */

#pragma once
#include <bitset>
#include <cmath>
#include <cstdint>
#include <cstring>
#include <string>
#include <vector>

namespace gymbo {

/**
 * @brief Checks if a float is an integer.
 *
 * The `is_integer` function checks if a given float value is an integer.
 *
 * @param x The float value to check.
 * @return `true` if the float is an integer, otherwise `false`.
 */
inline bool is_integer(float x) { return std::floor(x) == x; }

/**
 * @brief Converts a float value to a 32-bit word representation.
 *
 * The `FloatToWord` function converts a float value to its 32-bit word
 * representation using memcpy.
 *
 * @param val The float value to convert.
 * @return The 32-bit word representation of the float.
 */
inline uint32_t FloatToWord(float val) {
    uint32_t word;
    std::memcpy(&word, &val, sizeof(val));
    return word;
}

/**
 * @brief Converts a 32-bit word representation to a float value.
 *
 * The `wordToFloat` function converts a 32-bit word representation to its
 * corresponding float value using memcpy.
 *
 * @param word The 32-bit word representation.
 * @return The float value.
 */
inline float wordToFloat(uint32_t word) {
    float val;
    std::memcpy(&val, &word, sizeof(word));
    return val;
}

/**
 * @brief Converts a 32-bit word representation to an integer value.
 *
 * The `wordToInt` function converts a 32-bit word representation to its
 * corresponding integer value using static_cast.
 *
 * @param word The 32-bit word representation.
 * @return The integer value.
 */
inline int wordToInt(uint32_t word) { return static_cast<int>(word); }

/**
 * @brief Converts a uint32_t word to a signed int.
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
 * @brief Converts a uint32_t word to a bool.
 *
 * @param word The uint32_t word to convert.
 * @return The bool equivalent of the word.
 */
inline bool wordToBool(uint32_t word) { return word != 0; }

/**
 * @brief Converts a bool to a uint32_t word.
 *
 * @param value The bool to convert.
 * @return The uint32_t word equivalent of the bool.
 */
inline uint32_t boolToWord(bool value) { return value ? 1 : 0; }

/**
 * @brief Returns the twos complement of a uint32_t word.
 *
 * @param i The uint32_t word to get the twos complement of.
 * @return The twos complement of the word.
 */
inline uint32_t twosComplement(uint32_t i) { return 1 + ~i; }

/**
 * @brief Checks if a uint32_t word is negative.
 *
 * @param word The uint32_t word to check.
 * @return True if the word is negative, false otherwise.
 */
inline bool isNegative(uint32_t word) { return (word & 0x80000000) != 0; }

/**
 * @brief Generates a name for a variable based on its index.
 *
 * @param i The index of the variable.
 * @return A name for the variable.
 */
inline std::string valName(int i) { return "val_" + std::to_string(i); }

/**
 * @brief Node for a Doubly Linked List
 *
 * The `LLNode` class represents a node in a doubly linked list. It contains
 * data of type `T`, and pointers to the next and previous nodes in the list.
 *
 * @tparam T The type of data to store in the node.
 */
template <typename T>
class LLNode {
   public:
    T data;        ///< The data stored in the node.
    LLNode *next;  ///< Pointer to the next node in the linked list.
    LLNode *prev;  ///< Pointer to the previous node in the linked list.

    /**
     * @brief Default constructor for LLNode.
     *
     * @param data The data to store in the node.
     */
    LLNode(T data) : data(data), next(NULL), prev(NULL) {}
};

/**
 * @brief Doubly Linked List Implementation
 *
 * The `Linkedlist` class provides a simple implementation of a doubly linked
 * list. It supports basic operations such as pushing elements onto the back,
 * getting the length, accessing the element at the back, and popping the
 * element at the back.
 *
 * @tparam T The type of data to store in the linked list.
 */
template <typename T>
class Linkedlist {
   public:
    LLNode<T>
        *ghost;  ///< Ghost node for maintaining previous tails during pops.
    LLNode<T> *head;  ///< Pointer to the head of the linked list.
    LLNode<T> *tail;  ///< Pointer to the tail of the linked list.

    /**
     * @brief Default constructor for Linkedlist.
     */
    Linkedlist() : ghost(NULL), head(NULL), tail(NULL) {}

    /**
     * @brief Get the length of the linked list.
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
     * @brief Pushes a new element onto the back of the linked list.
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
     * @brief Returns the element at the back of the linked list.
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
     * @brief Pops the element at the back of the linked list.
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

/**
 * @brief Compute the Cartesian product of a vector of vectors of integers.
 *
 * This function takes a vector of vectors of integers and computes their
 * Cartesian product. The result is a vector of vectors, where each inner vector
 * represents a combination of elements from the input vectors.
 *
 * @param vectors A vector of vectors of integers for which the Cartesian
 * product is computed.
 * @return The Cartesian product as a vector of vectors of integers.
 */
inline std::vector<std::vector<int>> cartesianProduct(
    const std::vector<std::vector<int>> &vectors) {
    std::vector<std::vector<int>> result;

    if (vectors.size() == 0) {
        return result;
    }

    for (int v : vectors[0]) {
        std::vector<int> tmp = {v};
        result.emplace_back(tmp);
    }

    for (int i = 1; i < vectors.size(); ++i) {
        std::vector<std::vector<int>> tempResult;
        for (const auto &element1 : result) {
            for (const auto &element2 : vectors[i]) {
                std::vector<int> tempElement = element1;
                tempElement.push_back(element2);
                tempResult.push_back(tempElement);
            }
        }
        result = tempResult;
    }

    return result;
}
};  // namespace gymbo
