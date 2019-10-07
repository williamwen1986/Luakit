// Copyright (c) 2012 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <string>

#include "base/prefs/json_pref_store.h"
#include "base/prefs/mock_pref_change_callback.h"
#include "base/prefs/pref_change_registrar.h"
#include "base/prefs/pref_registry_simple.h"
#include "base/prefs/pref_value_store.h"
#include "base/prefs/testing_pref_service.h"
#include "base/prefs/testing_pref_store.h"
#include "base/values.h"
#include "testing/gmock/include/gmock/gmock.h"
#include "testing/gtest/include/gtest/gtest.h"

using testing::_;
using testing::Mock;

const char kPrefName[] = "pref.name";

TEST(PrefServiceTest, NoObserverFire) {
  TestingPrefServiceSimple prefs;

  const char pref_name[] = "homepage";
  prefs.registry()->RegisterStringPref(pref_name, std::string());

  const char new_pref_value[] = "http://www.google.com/";
  MockPrefChangeCallback obs(&prefs);
  PrefChangeRegistrar registrar;
  registrar.Init(&prefs);
  registrar.Add(pref_name, obs.GetCallback());

  // This should fire the checks in MockPrefChangeCallback::OnPreferenceChanged.
  const base::StringValue expected_value(new_pref_value);
  obs.Expect(pref_name, &expected_value);
  prefs.SetString(pref_name, new_pref_value);
  Mock::VerifyAndClearExpectations(&obs);

  // Setting the pref to the same value should not set the pref value a second
  // time.
  EXPECT_CALL(obs, OnPreferenceChanged(_)).Times(0);
  prefs.SetString(pref_name, new_pref_value);
  Mock::VerifyAndClearExpectations(&obs);

  // Clearing the pref should cause the pref to fire.
  const base::StringValue expected_default_value((std::string()));
  obs.Expect(pref_name, &expected_default_value);
  prefs.ClearPref(pref_name);
  Mock::VerifyAndClearExpectations(&obs);

  // Clearing the pref again should not cause the pref to fire.
  EXPECT_CALL(obs, OnPreferenceChanged(_)).Times(0);
  prefs.ClearPref(pref_name);
  Mock::VerifyAndClearExpectations(&obs);
}

TEST(PrefServiceTest, HasPrefPath) {
  TestingPrefServiceSimple prefs;

  const char path[] = "fake.path";

  // Shouldn't initially have a path.
  EXPECT_FALSE(prefs.HasPrefPath(path));

  // Register the path. This doesn't set a value, so the path still shouldn't
  // exist.
  prefs.registry()->RegisterStringPref(path, std::string());
  EXPECT_FALSE(prefs.HasPrefPath(path));

  // Set a value and make sure we have a path.
  prefs.SetString(path, "blah");
  EXPECT_TRUE(prefs.HasPrefPath(path));
}

TEST(PrefServiceTest, Observers) {
  const char pref_name[] = "homepage";

  TestingPrefServiceSimple prefs;
  prefs.SetUserPref(pref_name,
                    base::Value::CreateStringValue("http://www.cnn.com"));
  prefs.registry()->RegisterStringPref(pref_name, std::string());

  const char new_pref_value[] = "http://www.google.com/";
  const base::StringValue expected_new_pref_value(new_pref_value);
  MockPrefChangeCallback obs(&prefs);
  PrefChangeRegistrar registrar;
  registrar.Init(&prefs);
  registrar.Add(pref_name, obs.GetCallback());

  PrefChangeRegistrar registrar_two;
  registrar_two.Init(&prefs);

  // This should fire the checks in MockPrefChangeCallback::OnPreferenceChanged.
  obs.Expect(pref_name, &expected_new_pref_value);
  prefs.SetString(pref_name, new_pref_value);
  Mock::VerifyAndClearExpectations(&obs);

  // Now try adding a second pref observer.
  const char new_pref_value2[] = "http://www.youtube.com/";
  const base::StringValue expected_new_pref_value2(new_pref_value2);
  MockPrefChangeCallback obs2(&prefs);
  obs.Expect(pref_name, &expected_new_pref_value2);
  obs2.Expect(pref_name, &expected_new_pref_value2);
  registrar_two.Add(pref_name, obs2.GetCallback());
  // This should fire the checks in obs and obs2.
  prefs.SetString(pref_name, new_pref_value2);
  Mock::VerifyAndClearExpectations(&obs);
  Mock::VerifyAndClearExpectations(&obs2);

  // Set a recommended value.
  const base::StringValue recommended_pref_value("http://www.gmail.com/");
  obs.Expect(pref_name, &expected_new_pref_value2);
  obs2.Expect(pref_name, &expected_new_pref_value2);
  // This should fire the checks in obs and obs2 but with an unchanged value
  // as the recommended value is being overridden by the user-set value.
  prefs.SetRecommendedPref(pref_name, recommended_pref_value.DeepCopy());
  Mock::VerifyAndClearExpectations(&obs);
  Mock::VerifyAndClearExpectations(&obs2);

  // Make sure obs2 still works after removing obs.
  registrar.Remove(pref_name);
  EXPECT_CALL(obs, OnPreferenceChanged(_)).Times(0);
  obs2.Expect(pref_name, &expected_new_pref_value);
  // This should only fire the observer in obs2.
  prefs.SetString(pref_name, new_pref_value);
  Mock::VerifyAndClearExpectations(&obs);
  Mock::VerifyAndClearExpectations(&obs2);
}

// Make sure that if a preference changes type, so the wrong type is stored in
// the user pref file, it uses the correct fallback value instead.
TEST(PrefServiceTest, GetValueChangedType) {
  const int kTestValue = 10;
  TestingPrefServiceSimple prefs;
  prefs.registry()->RegisterIntegerPref(kPrefName, kTestValue);

  // Check falling back to a recommended value.
  prefs.SetUserPref(kPrefName,
                    base::Value::CreateStringValue("not an integer"));
  const PrefService::Preference* pref = prefs.FindPreference(kPrefName);
  ASSERT_TRUE(pref);
  const base::Value* value = pref->GetValue();
  ASSERT_TRUE(value);
  EXPECT_EQ(base::Value::TYPE_INTEGER, value->GetType());
  int actual_int_value = -1;
  EXPECT_TRUE(value->GetAsInteger(&actual_int_value));
  EXPECT_EQ(kTestValue, actual_int_value);
}

TEST(PrefServiceTest, GetValueAndGetRecommendedValue) {
  const int kDefaultValue = 5;
  const int kUserValue = 10;
  const int kRecommendedValue = 15;
  TestingPrefServiceSimple prefs;
  prefs.registry()->RegisterIntegerPref(kPrefName, kDefaultValue);

  // Create pref with a default value only.
  const PrefService::Preference* pref = prefs.FindPreference(kPrefName);
  ASSERT_TRUE(pref);

  // Check that GetValue() returns the default value.
  const base::Value* value = pref->GetValue();
  ASSERT_TRUE(value);
  EXPECT_EQ(base::Value::TYPE_INTEGER, value->GetType());
  int actual_int_value = -1;
  EXPECT_TRUE(value->GetAsInteger(&actual_int_value));
  EXPECT_EQ(kDefaultValue, actual_int_value);

  // Check that GetRecommendedValue() returns no value.
  value = pref->GetRecommendedValue();
  ASSERT_FALSE(value);

  // Set a user-set value.
  prefs.SetUserPref(kPrefName, base::Value::CreateIntegerValue(kUserValue));

  // Check that GetValue() returns the user-set value.
  value = pref->GetValue();
  ASSERT_TRUE(value);
  EXPECT_EQ(base::Value::TYPE_INTEGER, value->GetType());
  actual_int_value = -1;
  EXPECT_TRUE(value->GetAsInteger(&actual_int_value));
  EXPECT_EQ(kUserValue, actual_int_value);

  // Check that GetRecommendedValue() returns no value.
  value = pref->GetRecommendedValue();
  ASSERT_FALSE(value);

  // Set a recommended value.
  prefs.SetRecommendedPref(kPrefName,
                           base::Value::CreateIntegerValue(kRecommendedValue));

  // Check that GetValue() returns the user-set value.
  value = pref->GetValue();
  ASSERT_TRUE(value);
  EXPECT_EQ(base::Value::TYPE_INTEGER, value->GetType());
  actual_int_value = -1;
  EXPECT_TRUE(value->GetAsInteger(&actual_int_value));
  EXPECT_EQ(kUserValue, actual_int_value);

  // Check that GetRecommendedValue() returns the recommended value.
  value = pref->GetRecommendedValue();
  ASSERT_TRUE(value);
  EXPECT_EQ(base::Value::TYPE_INTEGER, value->GetType());
  actual_int_value = -1;
  EXPECT_TRUE(value->GetAsInteger(&actual_int_value));
  EXPECT_EQ(kRecommendedValue, actual_int_value);

  // Remove the user-set value.
  prefs.RemoveUserPref(kPrefName);

  // Check that GetValue() returns the recommended value.
  value = pref->GetValue();
  ASSERT_TRUE(value);
  EXPECT_EQ(base::Value::TYPE_INTEGER, value->GetType());
  actual_int_value = -1;
  EXPECT_TRUE(value->GetAsInteger(&actual_int_value));
  EXPECT_EQ(kRecommendedValue, actual_int_value);

  // Check that GetRecommendedValue() returns the recommended value.
  value = pref->GetRecommendedValue();
  ASSERT_TRUE(value);
  EXPECT_EQ(base::Value::TYPE_INTEGER, value->GetType());
  actual_int_value = -1;
  EXPECT_TRUE(value->GetAsInteger(&actual_int_value));
  EXPECT_EQ(kRecommendedValue, actual_int_value);
}

class PrefServiceSetValueTest : public testing::Test {
 protected:
  static const char kName[];
  static const char kValue[];

  PrefServiceSetValueTest() : observer_(&prefs_) {}

  TestingPrefServiceSimple prefs_;
  MockPrefChangeCallback observer_;
};

const char PrefServiceSetValueTest::kName[] = "name";
const char PrefServiceSetValueTest::kValue[] = "value";

TEST_F(PrefServiceSetValueTest, SetStringValue) {
  const char default_string[] = "default";
  const base::StringValue default_value(default_string);
  prefs_.registry()->RegisterStringPref(kName, default_string);

  PrefChangeRegistrar registrar;
  registrar.Init(&prefs_);
  registrar.Add(kName, observer_.GetCallback());

  // Changing the controlling store from default to user triggers notification.
  observer_.Expect(kName, &default_value);
  prefs_.Set(kName, default_value);
  Mock::VerifyAndClearExpectations(&observer_);

  EXPECT_CALL(observer_, OnPreferenceChanged(_)).Times(0);
  prefs_.Set(kName, default_value);
  Mock::VerifyAndClearExpectations(&observer_);

  base::StringValue new_value(kValue);
  observer_.Expect(kName, &new_value);
  prefs_.Set(kName, new_value);
  Mock::VerifyAndClearExpectations(&observer_);
}

TEST_F(PrefServiceSetValueTest, SetDictionaryValue) {
  prefs_.registry()->RegisterDictionaryPref(kName);
  PrefChangeRegistrar registrar;
  registrar.Init(&prefs_);
  registrar.Add(kName, observer_.GetCallback());

  EXPECT_CALL(observer_, OnPreferenceChanged(_)).Times(0);
  prefs_.RemoveUserPref(kName);
  Mock::VerifyAndClearExpectations(&observer_);

  base::DictionaryValue new_value;
  new_value.SetString(kName, kValue);
  observer_.Expect(kName, &new_value);
  prefs_.Set(kName, new_value);
  Mock::VerifyAndClearExpectations(&observer_);

  EXPECT_CALL(observer_, OnPreferenceChanged(_)).Times(0);
  prefs_.Set(kName, new_value);
  Mock::VerifyAndClearExpectations(&observer_);

  base::DictionaryValue empty;
  observer_.Expect(kName, &empty);
  prefs_.Set(kName, empty);
  Mock::VerifyAndClearExpectations(&observer_);
}

TEST_F(PrefServiceSetValueTest, SetListValue) {
  prefs_.registry()->RegisterListPref(kName);
  PrefChangeRegistrar registrar;
  registrar.Init(&prefs_);
  registrar.Add(kName, observer_.GetCallback());

  EXPECT_CALL(observer_, OnPreferenceChanged(_)).Times(0);
  prefs_.RemoveUserPref(kName);
  Mock::VerifyAndClearExpectations(&observer_);

  base::ListValue new_value;
  new_value.Append(base::Value::CreateStringValue(kValue));
  observer_.Expect(kName, &new_value);
  prefs_.Set(kName, new_value);
  Mock::VerifyAndClearExpectations(&observer_);

  EXPECT_CALL(observer_, OnPreferenceChanged(_)).Times(0);
  prefs_.Set(kName, new_value);
  Mock::VerifyAndClearExpectations(&observer_);

  base::ListValue empty;
  observer_.Expect(kName, &empty);
  prefs_.Set(kName, empty);
  Mock::VerifyAndClearExpectations(&observer_);
}
