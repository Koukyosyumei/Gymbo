#include <string>

#include "../../libgymbo/tokenizer.h"
#include "gtest/gtest.h"

// Example test fixture
class GymboTokenizerTest : public ::testing::Test {
   protected:
    void SetUp() override {
        // Optional: Set up any common objects or variables needed for your
        // tests
    }

    void TearDown() override {
        // Optional: Clean up any common objects or variables after your tests
    }
};

// Test is_alpha function
TEST_F(GymboTokenizerTest, IsAlpha) {
    EXPECT_TRUE(gymbo::is_alpha('a'));
    EXPECT_TRUE(gymbo::is_alpha('Z'));
    EXPECT_TRUE(gymbo::is_alpha('_'));
    EXPECT_FALSE(gymbo::is_alpha('1'));
    EXPECT_FALSE(gymbo::is_alpha('@'));
}

// Test is_alnum function
TEST_F(GymboTokenizerTest, IsAlnum) {
    EXPECT_TRUE(gymbo::is_alnum('a'));
    EXPECT_TRUE(gymbo::is_alnum('Z'));
    EXPECT_TRUE(gymbo::is_alnum('1'));
    EXPECT_FALSE(gymbo::is_alnum('@'));
}

// Test consume function
TEST_F(GymboTokenizerTest, Consume) {
    gymbo::Token token1, token2;
    token1.kind = gymbo::TOKEN_RESERVED;
    std::string str_if = "if";
    token1.str = const_cast<char *>(str_if.c_str());
    token1.len = 2;
    token1.next = &token2;
    token2.kind = gymbo::TOKEN_IDENT;
    std::string str_x = "x";
    token2.str = const_cast<char *>(str_x.c_str());
    token2.len = 1;
    token2.next = nullptr;

    gymbo::Token *token = &token1;
    gymbo::consume(token, const_cast<char *>(str_if.c_str()));
    EXPECT_EQ(token->kind, gymbo::TOKEN_IDENT);
}

// Test consume_tok function
TEST_F(GymboTokenizerTest, ConsumeTok) {
    gymbo::Token token1, token2;
    token1.kind = gymbo::TOKEN_RESERVED;
    std::string str_if = "if";
    token1.str = const_cast<char *>(str_if.c_str());
    token1.len = 2;
    token1.next = &token2;
    token2.kind = gymbo::TOKEN_IDENT;
    std::string str_x = "x";
    token2.str = const_cast<char *>(str_x.c_str());
    token2.len = 1;
    token2.next = nullptr;

    gymbo::Token *token = &token1;
    gymbo::consume_tok(token, gymbo::TOKEN_RESERVED);
    EXPECT_EQ(token->kind, gymbo::TOKEN_IDENT);
}

// Test consume_ident function
TEST_F(GymboTokenizerTest, ConsumeIdent) {
    gymbo::Token token1, token2;
    token1.kind = gymbo::TOKEN_IDENT;
    std::string str_x = "x";
    token1.str = const_cast<char *>(str_x.c_str());
    token1.len = 1;
    token1.next = &token2;
    token2.kind = gymbo::TOKEN_NUM;
    std::string str_42 = "42";
    token2.str = const_cast<char *>(str_42.c_str());
    token2.len = 2;
    token2.next = nullptr;

    gymbo::Token *token = &token1;
    gymbo::Token *result = gymbo::consume_ident(token);
    EXPECT_EQ(result, &token1);
    EXPECT_EQ(token->kind, gymbo::TOKEN_NUM);
}

TEST_F(GymboTokenizerTest, AtEof) {
    gymbo::Token token;
    token.kind = gymbo::TOKEN_EOF;
    EXPECT_TRUE(gymbo::at_eof(&token));
}

