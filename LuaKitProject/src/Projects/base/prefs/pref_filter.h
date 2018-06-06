// Copyright 2013 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef BASE_PREFS_PREF_FILTER_H_
#define BASE_PREFS_PREF_FILTER_H_

#include <string>

#include "base/prefs/base_prefs_export.h"

namespace base {
class DictionaryValue;
class Value;
}  // namespace base

// Filters preferences as they are loaded from disk or updated at runtime.
// Currently supported only by JsonPrefStore.
class BASE_PREFS_EXPORT PrefFilter {
 public:
  virtual ~PrefFilter() {}

  // Receives notification when the pref store data has been loaded but before
  // Observers are notified.
  // Changes made by a PrefFilter during FilterOnLoad do not result in
  // notifications to |PrefStore::Observer|s.
  virtual void FilterOnLoad(base::DictionaryValue* pref_store_contents) = 0;

  // Receives notification when a pref store value is changed, before Observers
  // are notified.
  virtual void FilterUpdate(const std::string& path,
                            const base::Value* value) = 0;
};

#endif  // BASE_PREFS_PREF_FILTER_H_
