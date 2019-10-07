// Copyright 2016 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef NET_QUIC_PLATFORM_IMPL_QUIC_EXPORT_IMPL_H_
#define NET_QUIC_PLATFORM_IMPL_QUIC_EXPORT_IMPL_H_

#include "net/net_export.h" // Patch [LARPOUX]

#define QUIC_EXPORT NET_EXPORT
#define QUIC_EXPORT_PRIVATE NET_EXPORT_PRIVATE

#endif  // NET_QUIC_PLATFORM_IMPL_QUIC_EXPORT_IMPL_H_
