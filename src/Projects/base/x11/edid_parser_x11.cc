// Copyright (c) 2013 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "base/x11/edid_parser_x11.h"

#include <X11/extensions/Xrandr.h>
#include <X11/Xatom.h>
#include <X11/Xlib.h>

#include "base/hash.h"
#include "base/message_loop/message_loop.h"
#include "base/strings/string_util.h"
#include "base/sys_byteorder.h"

namespace {

// Returns 64-bit persistent ID for the specified manufacturer's ID and
// product_code_hash, and the index of the output it is connected to.
// |output_index| is used to distinguish the displays of the same type. For
// example, swapping two identical display between two outputs will not be
// treated as swap. The 'serial number' field in EDID isn't used here because
// it is not guaranteed to have unique number and it may have the same fixed
// value (like 0).
int64 GetID(uint16 manufacturer_id,
            uint32 product_code_hash,
            uint8 output_index) {
  return ((static_cast<int64>(manufacturer_id) << 40) |
          (static_cast<int64>(product_code_hash) << 8) | output_index);
}

bool IsRandRAvailable() {
  int randr_version_major = 0;
  int randr_version_minor = 0;
  static bool is_randr_available = XRRQueryVersion(
      base::MessagePumpX11::GetDefaultXDisplay(),
      &randr_version_major, &randr_version_minor);
  return is_randr_available;
}

}  // namespace

namespace base {

bool GetEDIDProperty(XID output, unsigned long* nitems, unsigned char** prop) {
  if (!IsRandRAvailable())
    return false;

  Display* display = base::MessagePumpX11::GetDefaultXDisplay();

  static Atom edid_property = XInternAtom(
      base::MessagePumpX11::GetDefaultXDisplay(),
      RR_PROPERTY_RANDR_EDID, false);

  bool has_edid_property = false;
  int num_properties = 0;
  Atom* properties = XRRListOutputProperties(display, output, &num_properties);
  for (int i = 0; i < num_properties; ++i) {
    if (properties[i] == edid_property) {
      has_edid_property = true;
      break;
    }
  }
  XFree(properties);
  if (!has_edid_property)
    return false;

  Atom actual_type;
  int actual_format;
  unsigned long bytes_after;
  XRRGetOutputProperty(display,
                       output,
                       edid_property,
                       0,                // offset
                       128,              // length
                       false,            // _delete
                       false,            // pending
                       AnyPropertyType,  // req_type
                       &actual_type,
                       &actual_format,
                       nitems,
                       &bytes_after,
                       prop);
  DCHECK_EQ(XA_INTEGER, actual_type);
  DCHECK_EQ(8, actual_format);
  return true;
}

bool GetDisplayId(XID output_id, size_t output_index, int64* display_id_out) {
  unsigned long nitems = 0;
  unsigned char* prop = NULL;
  if (!GetEDIDProperty(output_id, &nitems, &prop))
    return false;

  bool result =
      GetDisplayIdFromEDID(prop, nitems, output_index, display_id_out);
  XFree(prop);
  return result;
}

bool GetDisplayIdFromEDID(const unsigned char* prop,
                          unsigned long nitems,
                          size_t output_index,
                          int64* display_id_out) {
  uint16 manufacturer_id = 0;
  std::string product_name;

  // ParseOutputDeviceData fails if it doesn't have product_name.
  ParseOutputDeviceData(prop, nitems, &manufacturer_id, &product_name);

  // Generates product specific value from product_name instead of product code.
  // See crbug.com/240341
  uint32 product_code_hash = product_name.empty() ?
      0 : base::Hash(product_name);
  if (manufacturer_id != 0) {
    // An ID based on display's index will be assigned later if this call
    // fails.
    *display_id_out = GetID(
        manufacturer_id, product_code_hash, output_index);
    return true;
  }
  return false;
}

bool ParseOutputDeviceData(const unsigned char* prop,
                           unsigned long nitems,
                           uint16* manufacturer_id,
                           std::string* human_readable_name) {
  // See http://en.wikipedia.org/wiki/Extended_display_identification_data
  // for the details of EDID data format.  We use the following data:
  //   bytes 8-9: manufacturer EISA ID, in big-endian
  //   bytes 54-125: four descriptors (18-bytes each) which may contain
  //     the display name.
  const unsigned int kManufacturerOffset = 8;
  const unsigned int kManufacturerLength = 2;
  const unsigned int kDescriptorOffset = 54;
  const unsigned int kNumDescriptors = 4;
  const unsigned int kDescriptorLength = 18;
  // The specifier types.
  const unsigned char kMonitorNameDescriptor = 0xfc;

  if (manufacturer_id) {
    if (nitems < kManufacturerOffset + kManufacturerLength) {
      LOG(ERROR) << "too short EDID data: manifacturer id";
      return false;
    }

    *manufacturer_id =
        *reinterpret_cast<const uint16*>(prop + kManufacturerOffset);
#if defined(ARCH_CPU_LITTLE_ENDIAN)
    *manufacturer_id = base::ByteSwap(*manufacturer_id);
#endif
  }

  if (!human_readable_name)
    return true;

  human_readable_name->clear();
  for (unsigned int i = 0; i < kNumDescriptors; ++i) {
    if (nitems < kDescriptorOffset + (i + 1) * kDescriptorLength)
      break;

    const unsigned char* desc_buf =
        prop + kDescriptorOffset + i * kDescriptorLength;
    // If the descriptor contains the display name, it has the following
    // structure:
    //   bytes 0-2, 4: \0
    //   byte 3: descriptor type, defined above.
    //   bytes 5-17: text data, ending with \r, padding with spaces
    // we should check bytes 0-2 and 4, since it may have other values in
    // case that the descriptor contains other type of data.
    if (desc_buf[0] == 0 && desc_buf[1] == 0 && desc_buf[2] == 0 &&
        desc_buf[4] == 0) {
      if (desc_buf[3] == kMonitorNameDescriptor) {
        std::string found_name(
            reinterpret_cast<const char*>(desc_buf + 5), kDescriptorLength - 5);
        TrimWhitespaceASCII(found_name, TRIM_TRAILING, human_readable_name);
        break;
      }
    }
  }

  // Verify if the |human_readable_name| consists of printable characters only.
  for (size_t i = 0; i < human_readable_name->size(); ++i) {
    char c = (*human_readable_name)[i];
    if (!isascii(c) || !isprint(c)) {
      human_readable_name->clear();
      LOG(ERROR) << "invalid EDID: human unreadable char in name";
      return false;
    }
  }

  return true;
}

}  // namespace base
