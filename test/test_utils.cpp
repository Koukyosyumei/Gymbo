#include "../libgymbo/utils.h"  // Replace with the actual header file name
#include "gtest/gtest.h"

// Test Linkedlist push function
TEST(LinkedlistTest, Push) {
    Linkedlist<int> list;
    list.push(1);
    list.push(2);
    list.push(3);

    // Check the length of the linked list
    EXPECT_EQ(list.len(), 3);

    // Check the elements
    EXPECT_EQ(*list.back(), 3);
}

// Test Linkedlist pop function
TEST(LinkedlistTest, Pop) {
    Linkedlist<int> list;
    list.push(1);
    list.push(2);
    list.push(3);

    // Check the length before popping
    EXPECT_EQ(list.len(), 3);

    // Pop an element
    list.pop();

    // Check the length after popping
    EXPECT_EQ(list.len(), 2);

    // Check the elements
    EXPECT_EQ(*list.back(), 2);
}

// Test Linkedlist back function
TEST(LinkedlistTest, Back) {
    Linkedlist<int> list;
    list.push(1);
    list.push(2);
    list.push(3);

    // Check the element at the back
    EXPECT_EQ(*list.back(), 3);
}

// Test Linkedlist len function
TEST(LinkedlistTest, Len) {
    Linkedlist<int> list;
    list.push(1);
    list.push(2);
    list.push(3);

    // Check the length of the linked list
    EXPECT_EQ(list.len(), 3);
}
