// Copyright (c) 2013 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef BASE_X11_EDID_PARSER_X11_H_
#define BASE_X11_EDID_PARSER_X11_H_

#include <string>

#include "base/base_export.h"
#include "base/basictypes.h"

typedef unsigned long XID;

// EDID (Extended Display Identification Data) is a format for monitor
// metadata. This provides a parser for the data and an interface to get it
// from XRandR.

namespace base {

// Get the EDID data from the |output| and stores to |prop|. |nitem| will store
// the number of characters |prop| will have. It doesn't take the ownership of
// |prop|, so caller must release it by XFree().
// Returns true if EDID property is successfully obtained. Otherwise returns
// false and does not touch |prop| and |nitems|.
BASE_EXPORT bool GetEDIDProperty(XID output,
                                 unsigned long* nitems,
                                 unsigned char** prop);

// Gets the EDID data from |output| and generates the display id through
// |GetDisplayIdFromEDID|.
BASE_EXPORT bool GetDisplayId(XID output, size_t index,
                              int64* display_id_out);

// Generates the display id for the pair of |prop| with |nitems| length and
// |index|, and store in |display_id_out|. Returns true if the display id is
// successfully generated, or false otherwise.
BASE_EXPORT bool GetDisplayIdFromEDID(const unsigned char* prop,
                                      unsigned long nitems,
                                      size_t index,
                                      int64* display_id_out);

// Parses |prop| as EDID data and stores extracted data into |manufacturer_id|
// and |human_readable_name| and returns true. NULL can be passed for unwanted
// output parameters. Some devices (especially internal displays) may not have
// the field for |human_readable_name|, and it will return true in that case.
BASE_EXPORT bool ParseOutputDeviceData(const unsigned char* prop,
                                       unsigned long nitems,
                                       uint16* manufacturer_id,
                                       std::string* human_readable_name);

}  // namespace base

#endif  // BASE_X11_EDID_PARSER_X11_H_
