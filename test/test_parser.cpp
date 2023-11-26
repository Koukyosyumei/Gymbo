#include "gtest/gtest.h"
#include "../libgymbo/parser.h"

// Test primary function
TEST(GymboParserTest, Primary) {
  gymbo::Token* token= new gymbo::Token();
  token->kind = gymbo::TOKEN_NUM;
  token->val = 42;
  token->len = 2;
  token->next = nullptr;

  char user_input[] = "42";

  gymbo::Node *result = gymbo::primary(token, user_input);

  // Check if the result is a numeric node with the correct value
  ASSERT_EQ(result->kind, gymbo::ND_NUM);
  ASSERT_EQ(result->val, 42);

  // Clean up
  delete token;
  delete result;
}
