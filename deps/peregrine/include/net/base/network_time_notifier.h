// Copyright (c) 2013 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef NET_BASE_NETWORK_TIME_NOTIFIER_H_
#define NET_BASE_NETWORK_TIME_NOTIFIER_H_

#include <vector>

#include "base/callback.h"
#include "base/memory/ref_counted.h"
#include "base/memory/scoped_ptr.h"
#include "base/memory/weak_ptr.h"
#include "base/threading/thread_checker.h"
#include "base/time/tick_clock.h"
#include "base/time/time.h"
#include "net/base/net_export.h"

namespace net {

// A class that receives updates for and maintains network time. Network time
// sources can pass updates through UpdateNetworkTime, and network time
// consumers can register as observers. This class is not thread-safe.
class NET_EXPORT NetworkTimeNotifier {
 public:
  // Callback for observers to receive network time updates.
  // The parameters are:
  // const base::Time& network_time - the new network time.
  // const base::TimeTicks& network_time_ticks - the ticks time that corresponds
  //     with |network_time|.
  // const base::TimeDelta& network_time_uncertainty - the uncertainty
  //     associated with the new network time.
  typedef base::Callback<void(const base::Time&,
                              const base::TimeTicks&,
                              const base::TimeDelta&)> ObserverCallback;

  // Takes ownership of |tick_clock|.
  explicit NetworkTimeNotifier(scoped_ptr<base::TickClock> tick_clock);
  ~NetworkTimeNotifier();

  // Calculates corresponding time ticks according to the given parameters and
  // notifies observers. The provided |network_time| is precise at the given
  // |resolution| and represent the time between now and up to |latency| +
  // (now - |post_time|) ago.
  void UpdateNetworkTime(const base::Time& network_time,
                         const base::TimeDelta& resolution,
                         const base::TimeDelta& latency,
                         const base::TimeTicks& post_time);

  // |observer_callback| will invoked every time the network time is updated, or
  // if a network time is already available when AddObserver is called.
  void AddObserver(const ObserverCallback& observer_callback);

 private:
  base::ThreadChecker thread_checker_;

  // For querying current time ticks.
  scoped_ptr<base::TickClock> tick_clock_;

  // The network time based on last call to UpdateNetworkTime().
  base::Time network_time_;

  // The estimated local time from |tick_clock| that corresponds with
  // |network_time|. Assumes the actual network time measurement was performed
  // midway through the latency time, and does not account for suspect/resume
  // events since the network time was measured.
  // See UpdateNetworkTime(..) implementation for details.
  base::TimeTicks network_time_ticks_;

  // Uncertainty of |network_time_| based on added inaccuracies/resolution.
  // See UpdateNetworkTime(..) implementation for details.
  base::TimeDelta network_time_uncertainty_;

  // List of network time update observers.
  // A vector of callbacks is used, rather than an ObserverList, so that the
  // lifetime of the observer can be bound to the callback.
  std::vector<ObserverCallback> observers_;

  DISALLOW_COPY_AND_ASSIGN(NetworkTimeNotifier);
};

}  // namespace net

#endif  // NET_BASE_NETWORK_TIME_NOTIFIER_H_
