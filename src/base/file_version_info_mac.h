// Copyright (c) 2011 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef BASE_FILE_VERSION_INFO_MAC_H_
#define BASE_FILE_VERSION_INFO_MAC_H_

#include <string>

#include "base/file_version_info.h"
#include "base/mac/scoped_nsobject.h"

#ifdef __OBJC__
@class NSBundle;
#else
class NSBundle;
#endif

class FileVersionInfoMac : public FileVersionInfo {
 public:
  explicit FileVersionInfoMac(NSBundle *bundle);
  virtual ~FileVersionInfoMac();

  // Accessors to the different version properties.
  // Returns an empty string if the property is not found.
  virtual base::string16 company_name() OVERRIDE;
  virtual base::string16 company_short_name() OVERRIDE;
  virtual base::string16 product_name() OVERRIDE;
  virtual base::string16 product_short_name() OVERRIDE;
  virtual base::string16 internal_name() OVERRIDE;
  virtual base::string16 product_version() OVERRIDE;
  virtual base::string16 private_build() OVERRIDE;
  virtual base::string16 special_build() OVERRIDE;
  virtual base::string16 comments() OVERRIDE;
  virtual base::string16 original_filename() OVERRIDE;
  virtual base::string16 file_description() OVERRIDE;
  virtual base::string16 file_version() OVERRIDE;
  virtual base::string16 legal_copyright() OVERRIDE;
  virtual base::string16 legal_trademarks() OVERRIDE;
  virtual base::string16 last_change() OVERRIDE;
  virtual bool is_official_build() OVERRIDE;

 private:
  // Returns a base::string16 value for a property name.
  // Returns the empty string if the property does not exist.
  base::string16 GetString16Value(CFStringRef name);

  base::scoped_nsobject<NSBundle> bundle_;

  DISALLOW_COPY_AND_ASSIGN(FileVersionInfoMac);
};

#endif  // BASE_FILE_VERSION_INFO_MAC_H_
