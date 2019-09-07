// Copyright 2013 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <string>
#include <vector>

#include "base/ini_parser.h"
#include "testing/gtest/include/gtest/gtest.h"

namespace base {

namespace {

struct TestTriplet {
  TestTriplet(const std::string& section,
          const std::string& key,
          const std::string& value)
      : section(section),
        key(key),
        value(value) {
  }

  std::string section;
  std::string key;
  std::string value;
};

class TestINIParser : public INIParser {
 public:
  explicit TestINIParser(
      const std::vector<TestTriplet>& expected_triplets)
      : expected_triplets_(expected_triplets),
        pair_i_(0) {
  }
  virtual ~TestINIParser() {}

  size_t pair_i() {
    return pair_i_;
  }

 private:
  virtual void HandleTriplet(const std::string& section, const std::string& key,
                             const std::string& value) OVERRIDE {
    EXPECT_EQ(expected_triplets_[pair_i_].section, section);
    EXPECT_EQ(expected_triplets_[pair_i_].key, key);
    EXPECT_EQ(expected_triplets_[pair_i_].value, value);
    ++pair_i_;
  }

  std::vector<TestTriplet> expected_triplets_;
  size_t pair_i_;
};

TEST(INIParserTest, BasicValid) {
  std::vector<TestTriplet> expected_triplets;
  expected_triplets.push_back(TestTriplet("section1", "key1", "value1"));
  expected_triplets.push_back(TestTriplet("section1", "key2", "value2"));
  expected_triplets.push_back(TestTriplet("section1", "key3", "value3"));
  expected_triplets.push_back(TestTriplet("section2", "key4", "value4"));
  expected_triplets.push_back(TestTriplet("section2", "key5",
                                          "value=with=equals"));
  expected_triplets.push_back(TestTriplet("section2", "key6", "value6"));
  TestINIParser test_parser(expected_triplets);

  test_parser.Parse(
      "[section1]\n"
      "key1=value1\n"
      "key2=value2\r\n"  // Testing DOS "\r\n" line endings.
      "key3=value3\n"
      "[section2\n"      // Testing omitted closing bracket.
      "key4=value4\r"    // Testing "\r" line endings.
      "key5=value=with=equals\n"
      "key6=value6");    // Testing omitted final line ending.
}

TEST(INIParserTest, IgnoreBlankLinesAndComments) {
  std::vector<TestTriplet> expected_triplets;
  expected_triplets.push_back(TestTriplet("section1", "key1", "value1"));
  expected_triplets.push_back(TestTriplet("section1", "key2", "value2"));
  expected_triplets.push_back(TestTriplet("section1", "key3", "value3"));
  expected_triplets.push_back(TestTriplet("section2", "key4", "value4"));
  expected_triplets.push_back(TestTriplet("section2", "key5", "value5"));
  expected_triplets.push_back(TestTriplet("section2", "key6", "value6"));
  TestINIParser test_parser(expected_triplets);

  test_parser.Parse(
      "\n"
      "[section1]\n"
      "key1=value1\n"
      "\n"
      "\n"
      "key2=value2\n"
      "key3=value3\n"
      "\n"
      ";Comment1"
      "\n"
      "[section2]\n"
      "key4=value4\n"
      "#Comment2\n"
      "key5=value5\n"
      "\n"
      "key6=value6\n");
}

TEST(INIParserTest, DictionaryValueINIParser) {
  DictionaryValueINIParser test_parser;

  test_parser.Parse(
      "[section1]\n"
      "key1=value1\n"
      "key.2=value2\n"
      "key3=va.lue3\n"
      "[se.ction2]\n"
      "key.4=value4\n"
      "key5=value5\n");

  const DictionaryValue& root = test_parser.root();
  std::string value;
  EXPECT_TRUE(root.GetString("section1.key1", &value));
  EXPECT_EQ("value1", value);
  EXPECT_FALSE(root.GetString("section1.key.2", &value));
  EXPECT_TRUE(root.GetString("section1.key3", &value));
  EXPECT_EQ("va.lue3", value);
  EXPECT_FALSE(root.GetString("se.ction2.key.4", &value));
  EXPECT_FALSE(root.GetString("se.ction2.key5", &value));
}

}  // namespace

}  // namespace base
