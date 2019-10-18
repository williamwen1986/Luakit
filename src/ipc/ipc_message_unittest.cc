// Copyright (c) 2012 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "ipc/ipc_message.h"

#include <string.h>

#include "base/memory/scoped_ptr.h"
#include "base/values.h"
#include "ipc/ipc_message_utils.h"
#include "testing/gtest/include/gtest/gtest.h"

namespace {

TEST(IPCMessageTest, ListValue) {
  base::ListValue input;
  input.Set(0, new base::FundamentalValue(42.42));
  input.Set(1, new base::StringValue("forty"));
  input.Set(2, base::Value::CreateNullValue());

  IPC::Message msg(1, 2, IPC::Message::PRIORITY_NORMAL);
  IPC::WriteParam(&msg, input);

  base::ListValue output;
  PickleIterator iter(msg);
  EXPECT_TRUE(IPC::ReadParam(&msg, &iter, &output));

  EXPECT_TRUE(input.Equals(&output));

  // Also test the corrupt case.
  IPC::Message bad_msg(1, 2, IPC::Message::PRIORITY_NORMAL);
  bad_msg.WriteInt(99);
  iter = PickleIterator(bad_msg);
  EXPECT_FALSE(IPC::ReadParam(&bad_msg, &iter, &output));
}

TEST(IPCMessageTest, DictionaryValue) {
  base::DictionaryValue input;
  input.Set("null", base::Value::CreateNullValue());
  input.Set("bool", new base::FundamentalValue(true));
  input.Set("int", new base::FundamentalValue(42));
  input.SetWithoutPathExpansion("int.with.dot", new base::FundamentalValue(43));

  scoped_ptr<base::DictionaryValue> subdict(new base::DictionaryValue());
  subdict->Set("str", new base::StringValue("forty two"));
  subdict->Set("bool", new base::FundamentalValue(false));

  scoped_ptr<base::ListValue> sublist(new base::ListValue());
  sublist->Set(0, new base::FundamentalValue(42.42));
  sublist->Set(1, new base::StringValue("forty"));
  sublist->Set(2, new base::StringValue("two"));
  subdict->Set("list", sublist.release());

  input.Set("dict", subdict.release());

  IPC::Message msg(1, 2, IPC::Message::PRIORITY_NORMAL);
  IPC::WriteParam(&msg, input);

  base::DictionaryValue output;
  PickleIterator iter(msg);
  EXPECT_TRUE(IPC::ReadParam(&msg, &iter, &output));

  EXPECT_TRUE(input.Equals(&output));

  // Also test the corrupt case.
  IPC::Message bad_msg(1, 2, IPC::Message::PRIORITY_NORMAL);
  bad_msg.WriteInt(99);
  iter = PickleIterator(bad_msg);
  EXPECT_FALSE(IPC::ReadParam(&bad_msg, &iter, &output));
}

}  // namespace
