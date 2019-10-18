// Copyright (c) 2011 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// This file describes a central switchboard for notifications that might
// happen in various parts of the application, and allows users to register
// observers for various classes of events that they're interested in.

#ifndef CONTENT_PUBLIC_BROWSER_NOTIFICATION_SERVICE_H_
#define CONTENT_PUBLIC_BROWSER_NOTIFICATION_SERVICE_H_
#pragma once

#include "notification_details.h"
#include "notification_source.h"

namespace content {

class NotificationService {
 public:
  // Returns the NotificationService object for the current thread, or NULL if
  // none.
  static NotificationService* current();

  static NotificationService* Create();

  virtual ~NotificationService() {}

  // Synchronously posts a notification to all interested observers.
  // Source is a reference to a NotificationSource object representing
  // the object originating the notification (can be
  // NotificationService::AllSources(), in which case
  // only observers interested in all sources will be notified).
  // Details is a reference to an object containing additional data about
  // the notification.  If no additional data is needed, NoDetails() is used.
  // There is no particular order in which the observers will be notified.
  virtual void Notify(int type,
                      const NotificationSource& source,
                      const NotificationDetails& details) = 0;

  // Returns a NotificationSource that represents all notification sources
  // (for the purpose of registering an observer for events from all sources).
  static Source<void> AllSources() { return Source<void>(NULL); }

  // Returns the same value as AllSources(). This function has semantic
  // differences to the programmer: We have checked that this AllSources()
  // usage is safe in the face of multiple profiles. Objects that were
  // singletons now will always have multiple instances, one per browser
  // context.
  //
  // Some usage is safe, where the Source is checked to see if it's a member of
  // a container before use. But, we want the number of AllSources() calls to
  // drop to almost nothing, because most usages are not multiprofile safe and
  // were done because it was easier to listen to everything.
  static Source<void> AllBrowserContextsAndSources() {
    return Source<void>(NULL);
  }

  // Returns a NotificationDetails object that represents a lack of details
  // associated with a notification.  (This is effectively a null pointer.)
  static Details<void> NoDetails() { return Details<void>(NULL); }
};

}  // namespace content

#endif  // CONTENT_PUBLIC_BROWSER_NOTIFICATION_SERVICE_H_
