// Copyright 2013 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef BASE_CALLBACK_REGISTRY_H_
#define BASE_CALLBACK_REGISTRY_H_

#include <list>

#include "base/basictypes.h"
#include "base/callback.h"
#include "base/compiler_specific.h"
#include "base/logging.h"
#include "base/memory/scoped_ptr.h"

// OVERVIEW:
//
// A container for a list of callbacks.  Unlike a normal STL vector or list,
// this container can be modified during iteration without invalidating the
// iterator. It safely handles the case of a callback removing itself
// or another callback from the list while callbacks are being run.
//
// TYPICAL USAGE:
//
// class MyWidget {
//  public:
//   ...
//
//   typedef base::Callback<void(const Foo&)> OnFooCallback;
//
//   scoped_ptr<base::CallbackRegistry<Foo>::Subscription> RegisterCallback(
//       const OnFooCallback& cb) {
//     return callback_registry_.Add(cb);
//   }
//
//  private:
//   void NotifyFoo(const Foo& foo) {
//      callback_registry_.Notify(foo);
//   }
//
//   base::CallbackRegistry<Foo> callback_registry_;
// };
//
//
// class MyWidgetListener {
//  public:
//   MyWidgetListener::MyWidgetListener() {
//     foo_subscription_ = MyWidget::GetCurrent()->RegisterCallback(
//             base::Bind(&MyWidgetListener::OnFoo, this)));
//   }
//
//   MyWidgetListener::~MyWidgetListener() {
//      // Subscription gets deleted automatically and will deregister
//      // the callback in the process.
//   }
//
//  private:
//   void OnFoo(const Foo& foo) {
//     // Do something.
//   }
//
//   scoped_ptr<base::CallbackRegistry<Foo>::Subscription> foo_subscription_;
// };

namespace base {

namespace internal {

template <typename CallbackType>
class CallbackRegistryBase {
 public:
  class Subscription {
   public:
    Subscription(CallbackRegistryBase<CallbackType>* list,
                 typename std::list<CallbackType>::iterator iter)
        : list_(list),
          iter_(iter) {}

    ~Subscription() {
      if (list_->active_iterator_count_)
        (*iter_).Reset();
      else
        list_->callbacks_.erase(iter_);
    }

   private:
    CallbackRegistryBase<CallbackType>* list_;
    typename std::list<CallbackType>::iterator iter_;

    DISALLOW_COPY_AND_ASSIGN(Subscription);
  };

  // Add a callback to the list. The callback will remain registered until the
  // returned Subscription is destroyed, which must occur before the
  // CallbackRegistry is destroyed.
  scoped_ptr<Subscription> Add(const CallbackType& cb) {
    DCHECK(!cb.is_null());
    return scoped_ptr<Subscription>(
        new Subscription(this, callbacks_.insert(callbacks_.end(), cb)));
  }

 protected:
  // An iterator class that can be used to access the list of callbacks.
  class Iterator {
   public:
    explicit Iterator(CallbackRegistryBase<CallbackType>* list)
        : list_(list),
          list_iter_(list_->callbacks_.begin()) {
      ++list_->active_iterator_count_;
    }

    Iterator(const Iterator& iter)
        : list_(iter.list_),
          list_iter_(iter.list_iter_) {
      ++list_->active_iterator_count_;
    }

    ~Iterator() {
      if (list_ && --list_->active_iterator_count_ == 0) {
        list_->Compact();
      }
    }

    CallbackType* GetNext() {
      while ((list_iter_ != list_->callbacks_.end()) && list_iter_->is_null())
        ++list_iter_;

      CallbackType* cb = NULL;
      if (list_iter_ != list_->callbacks_.end()) {
        cb = &(*list_iter_);
        ++list_iter_;
      }
      return cb;
    }

   private:
    CallbackRegistryBase<CallbackType>* list_;
    typename std::list<CallbackType>::iterator list_iter_;
  };

  CallbackRegistryBase()
      : active_iterator_count_(0) {}

  ~CallbackRegistryBase() {
    DCHECK_EQ(0, active_iterator_count_);
    DCHECK_EQ(0U, callbacks_.size());
  }

  // Returns an instance of a CallbackRegistryBase::Iterator which can be used
  // to run callbacks.
  Iterator GetIterator() {
    return Iterator(this);
  }

  // Compact the list: remove any entries which were NULLed out during
  // iteration.
  void Compact() {
    typename std::list<CallbackType>::iterator it = callbacks_.begin();
    while (it != callbacks_.end()) {
      if ((*it).is_null())
        it = callbacks_.erase(it);
      else
        ++it;
    }
  }

 private:
  std::list<CallbackType> callbacks_;
  int active_iterator_count_;

  DISALLOW_COPY_AND_ASSIGN(CallbackRegistryBase);
};

}  // namespace internal

template <typename Details>
class CallbackRegistry
    : public internal::CallbackRegistryBase<Callback<void(const Details&)> > {
 public:
  CallbackRegistry() {}

  // Execute all active callbacks with |details| parameter.
  void Notify(const Details& details) {
    typename internal::CallbackRegistryBase<
        Callback<void(const Details&)> >::Iterator it = this->GetIterator();
    Callback<void(const Details&)>* cb;
    while((cb = it.GetNext()) != NULL) {
      cb->Run(details);
    }
  }

private:
  DISALLOW_COPY_AND_ASSIGN(CallbackRegistry);
};

template <> class CallbackRegistry<void>
    : public internal::CallbackRegistryBase<Closure> {
 public:
  CallbackRegistry() {}

  // Execute all active callbacks.
  void Notify() {
    Iterator it = this->GetIterator();
    Closure* cb;
    while((cb = it.GetNext()) != NULL) {
      cb->Run();
    }
  }

 private:
  DISALLOW_COPY_AND_ASSIGN(CallbackRegistry);
};

}  // namespace base

#endif  // BASE_CALLBACK_REGISTRY_H_
