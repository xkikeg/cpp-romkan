#include <gtest/gtest.h>

#include <romkan.h>

TEST(roma_to_kana, hepburn)
{
  EXPECT_EQ(u8"あいうえお", roma_to_kana(u8"aiueo"));
}
