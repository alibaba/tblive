#pragma once

/*
 * Add by zhenshan 2015-1-19
 * 注意事项：
 * 1. 事件的host class对象销毁时，事件上绑定的响应会自动失效
 * 2. 绑定依赖base::Bind/Callback，并且由callback自己来维护注册者的生命期（refptr, weakptr, unretained）
 * 3. 需要动态解绑，使用token
 * 4. 事件触发回调默认在触发线程，也可以指定在UI线程回调
 * 5. 注册可以多线程并发
*/


#ifndef BASE_EVENT_H
#define BASE_EVENT_H

#include "base/bind.h"
#include "base/callback.h"
#include "base/synchronization/lock.h"
#include "base/memory/ref_counted.h"

#include <vector>

#if defined(OS_MACOSX)
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wunused-value"
#endif

namespace base {

// 辅助实现UI事件代理
class IEventCallbackDelegate 
{
public:
  virtual void PostTask(int event_type, base::Closure const& callback) = 0;
};

BASE_EXPORT void SetEventCallbackDelegate(IEventCallbackDelegate * pEventDelegate);

BASE_EXPORT bool EventPostTask( int event_type, base::Closure const& callback );


// 事件回调注册线程模式
// 用户自定义的事件类型使用 CALLBACK_DEFAULT 以外的值
enum REGISTER_CALLBACK_THREAD {
  CALLBACK_DEFAULT,
};

template<typename CallbackType>
inline std::pair<CallbackType, int> EventCallback( int event_type, CallbackType const& callback ) {
  return std::make_pair( callback, event_type );
}

// event token for cancel register
class event_token {
public:
	event_token(int id = 0) : id_(id) {}

	bool operator == ( event_token const& rhs ) const {
		return id_ == rhs.id_;
	}

private:
	int id_;
};

namespace internal {
  
// Event object wrapper for host class to export event ability 
// CallbackType: base::Callback<ret_type(arg_type)>
template<typename CallbackType>
class event_base {
public:
  typedef std::pair<CallbackType, int> EventCallbackType;

  // add register
  event_token operator += (CallbackType const& callback) {
    base::AutoLock lock(lock_);

    event_token token = token_generator_.next();
    callback_list_.push_back( event_reg_item(token, callback, CALLBACK_DEFAULT) );
    return token;
  }

  event_token operator += (EventCallbackType const& p) {
    base::AutoLock lock(lock_);

    event_token token = token_generator_.next();
    callback_list_.push_back( event_reg_item(token, p.first, p.second) );
    return token;
  }

  // remove register
  void operator -= ( event_token id ) {
    base::AutoLock lock(lock_);

    auto fit = std::find_if( callback_list_.begin(), callback_list_.end(), [id]( event_reg_item const& item ) {
      return item.token == id;
    } );

    if ( fit != callback_list_.end() ) {
      callback_list_.erase(fit);
    }
  }

  // call event
  // arguments: void
  void operator () () {
    std::vector<event_reg_item> callback_list;
    {
      base::AutoLock lock(lock_);
      callback_list = callback_list_;
    }

    std::for_each( callback_list.begin(), callback_list.end(), [=]( event_reg_item const& item ){
      if ( item.thread_type != CALLBACK_DEFAULT ) {
        EventPostTask( item.thread_type, base::Bind( base::IgnoreResult(item.callback) ) );
      } else {
        item.callback.Run();
      }
    } );
  }

  // arguments: P1
  template<typename P1>
  void operator () ( P1 const& a1 ) {
    std::vector<event_reg_item> callback_list;
    {
      base::AutoLock lock(lock_);
      callback_list = callback_list_;
    }

    std::for_each( callback_list.begin(), callback_list.end(), [=]( event_reg_item const& item ){
      if ( item.thread_type != CALLBACK_DEFAULT ) {
        EventPostTask( item.thread_type, base::Bind( base::IgnoreResult(item.callback), a1 ) );
      } else {
        item.callback.Run(a1);
      }
    } );
  }

  // arguments: P1, P2
  template<typename P1, typename P2>
  void operator () ( P1 const& a1, P2 const& a2 ) {
    std::vector<event_reg_item> callback_list;
    {
      base::AutoLock lock(lock_);
      callback_list = callback_list_;
    }

    std::for_each( callback_list.begin(), callback_list.end(), [=]( event_reg_item const& item ){
      if ( item.thread_type != CALLBACK_DEFAULT ) {
        EventPostTask( item.thread_type, base::Bind( base::IgnoreResult(item.callback), a1, a2 ) );
      } else {
        item.callback.Run(a1, a2);
      }
    } );
  }

  // arguments: P1, P2, P3
  template<typename P1, typename P2, typename P3>
  void operator () ( P1 const& a1, P2 const& a2, P3 const& a3 ) {
    std::vector<event_reg_item> callback_list;
    {
      base::AutoLock lock(lock_);
      callback_list = callback_list_;
    }

    std::for_each( callback_list.begin(), callback_list.end(), [=]( event_reg_item const& item ){
      if ( item.thread_type != CALLBACK_DEFAULT ) {
        EventPostTask( item.thread_type, base::Bind( base::IgnoreResult(item.callback), a1, a2, a3 ) );
      } else {
        item.callback.Run(a1, a2, a3);
      }
    } );
  }

  // arguments: P1, P2, P3, P4
  template<typename P1, typename P2, typename P3, typename P4>
  void operator () ( P1 const& a1, P2 const& a2, P3 const& a3, P4 const& a4 ) {
    std::vector<event_reg_item> callback_list;
    {
      base::AutoLock lock(lock_);
      callback_list = callback_list_;
    }

    std::for_each( callback_list.begin(), callback_list.end(), [=]( event_reg_item const& item ){
      if ( item.thread_type != CALLBACK_DEFAULT ) {
        EventPostTask( item.thread_type, base::Bind( base::IgnoreResult(item.callback), a1, a2, a3, a4 ) );
      } else {
        item.callback.Run(a1, a2, a3, a4);
      }
    } );
  }

  // arguments: P1, P2, P3, P4, P5
  template<typename P1, typename P2, typename P3, typename P4, typename P5>
  void operator () ( P1 const& a1, P2 const& a2, P3 const& a3, P4 const& a4, P5 const& a5 ) {
    std::vector<event_reg_item> callback_list;
    {
      base::AutoLock lock(lock_);
      callback_list = callback_list_;
    }

    std::for_each( callback_list.begin(), callback_list.end(), [=]( event_reg_item const& item ){
      if ( item.thread_type != CALLBACK_DEFAULT ) {
        EventPostTask( item.thread_type, base::Bind( base::IgnoreResult(item.callback), a1, a2, a3, a4, a5 ) );
      } else {
        item.callback.Run(a1, a2, a3, a4, a5);
      }
    } );
  }

  // arguments: P1, P2, P3, P4, P5, P6
  template<typename P1, typename P2, typename P3, typename P4, typename P5, typename P6>
  void operator () ( P1 const& a1, P2 const& a2, P3 const& a3, P4 const& a4, P5 const& a5, P6 const& a6 ) {
    std::vector<event_reg_item> callback_list;
    {
      base::AutoLock lock(lock_);
      callback_list = callback_list_;
    }

    std::for_each( callback_list.begin(), callback_list.end(), [=]( event_reg_item const& item ){
      if ( item.thread_type != CALLBACK_DEFAULT ) {
        EventPostTask( item.thread_type, base::Bind( base::IgnoreResult(item.callback), a1, a2, a3, a4, a5, a6 ) );
      } else {
        item.callback.Run(a1, a2, a3, a4, a5, a6);
      }
    } );
  }

  // arguments: P1, P2, P3, P4, P5, P6, P7
  template<typename P1, typename P2, typename P3, typename P4, typename P5, typename P6, typename P7>
  void operator () ( P1 const& a1, P2 const& a2, P3 const& a3, P4 const& a4, P5 const& a5, P6 const& a6, P7 const& a7 ) {
    std::vector<event_reg_item> callback_list;
    {
      base::AutoLock lock(lock_);
      callback_list = callback_list_;
    }

    std::for_each( callback_list.begin(), callback_list.end(), [=]( event_reg_item const& item ){
      if ( item.thread_type != CALLBACK_DEFAULT ) {
        EventPostTask( item.thread_type, base::Bind( base::IgnoreResult(item.callback), a1, a2, a3, a4, a5, a6, a7 ) );
      } else {
        item.callback.Run(a1, a2, a3, a4, a5, a6, a7);
      }
    } );
  }

private:

  // event token generator
  class event_token_generator {
  public:
    event_token_generator() : id_(0) {}

    event_token next() {
      return event_token(id_++);
    }
  private:
    int id_;
  };

  struct event_reg_item
  {
    event_reg_item( event_token const& tok, CallbackType const& cb, int type )
      : token(tok)
      , callback(cb)
      , thread_type(type)
    {}

    event_token token;
    CallbackType callback;
    int thread_type;
  };

private:
  base::Lock lock_;
  event_token_generator token_generator_;
  std::vector<event_reg_item> callback_list_;
};

// wrapper C++11 lambda to base::Closure
template<typename Func>
inline base::Closure LambdaClosure( Func const& func )
{
  struct Inner : public base::RefCountedThreadSafe<Inner>
  {
    Inner( Func const& func ) : func_(func) {}
    void Invoke() { func_(); }
    Func func_;
  };

  scoped_refptr<Inner> spInner( new Inner(func) );
  return base::Bind( &Inner::Invoke, spInner );
}

template<typename ArgType, typename Func>
inline base::Callback<void(ArgType)> LambdaArgCallback( Func const& func )
{
  struct Inner : public base::RefCountedThreadSafe<Inner>
  {
	  Inner( Func const& func ) : func_(func) {}
	  void Invoke(ArgType arg) { func_(arg); }
	  Func func_;
  };
		
  scoped_refptr<Inner> spInner( new Inner(func) );
  return base::Bind( &Inner::Invoke, spInner );
}

template<typename ArgType1, typename ArgType2, typename Func>
inline base::Callback<void(ArgType1, ArgType2)> LambdaArgCallback( Func const& func )
{
    struct Inner : public base::RefCountedThreadSafe<Inner>
    {
        Inner( Func const& func ) : func_(func) {}
        void Invoke(ArgType1 arg1, ArgType2 arg2) { func_(arg1, arg2); }
        Func func_;
    };
    
    scoped_refptr<Inner> spInner( new Inner(func) );
    return base::Bind( &Inner::Invoke, spInner );
}

template<typename ArgType1, typename ArgType2, typename ArgType3, typename Func>
inline base::Callback<void(ArgType1, ArgType2, ArgType3)> LambdaArgCallback( Func const& func )
{
	struct Inner : public base::RefCountedThreadSafe<Inner>
	{
		Inner( Func const& func ) : func_(func) {}
		void Invoke(ArgType1 arg1, ArgType2 arg2, ArgType3 arg3) { func_(arg1, arg2, arg3); }
		Func func_;
	};

	scoped_refptr<Inner> spInner( new Inner(func) );
	return base::Bind( &Inner::Invoke, spInner );
}

template<typename ArgType1, typename ArgType2, typename ArgType3, typename ArgType4, typename Func>
inline base::Callback<void(ArgType1, ArgType2, ArgType3, ArgType4)> LambdaArgCallback( Func const& func )
{
	struct Inner : public base::RefCountedThreadSafe<Inner>
	{
		Inner( Func const& func ) : func_(func) {}
		void Invoke(ArgType1 arg1, ArgType2 arg2, ArgType3 arg3, ArgType4 arg4) { func_(arg1, arg2, arg3, arg4); }
		Func func_;
	};

	scoped_refptr<Inner> spInner( new Inner(func) );
	return base::Bind( &Inner::Invoke, spInner );
}

template<typename T, typename Func>
inline base::Closure LambdaRefCallback( T* pThis, Func const& func )
{
	struct Inner : public base::RefCountedThreadSafe<Inner>
	{
		Inner( Func const& func, T* pThis) : func_(func), spThis(pThis) {}
		void Invoke() { func_(); }
		Func func_;
		scoped_refptr<T> spThis;
	};

	scoped_refptr<Inner> spInner( new Inner(func, pThis) );
	return base::Bind( &Inner::Invoke, spInner );
}

template<typename ArgType, typename T, typename Func>
inline base::Callback<void(ArgType)> LambdaRefCallback( T* pThis, Func const& func )
{
	struct Inner : public base::RefCountedThreadSafe<Inner>
	{
		Inner( Func const& func, T* pThis ) : func_(func), spThis(pThis) {}
		void Invoke(ArgType arg) { func_(arg); }
		Func func_;
		scoped_refptr<T> spThis;
	};

	scoped_refptr<Inner> spInner( new Inner(func, pThis) );
	return base::Bind( &Inner::Invoke, spInner );
}

template<typename ArgType1, typename ArgType2, typename T, typename Func>
inline base::Callback<void(ArgType1, ArgType2)> LambdaRefCallback( T* pThis, Func const& func )
{
	struct Inner : public base::RefCountedThreadSafe<Inner>
	{
		Inner( Func const& func, T* pThis ) : func_(func), spThis(pThis) {}
		void Invoke(ArgType1 arg1, ArgType2 arg2) { func_(arg1, arg2); }
		Func func_;
		scoped_refptr<T> spThis;
	};

	scoped_refptr<Inner> spInner( new Inner(func, pThis) );
	return base::Bind( &Inner::Invoke, spInner );
}

template<typename ArgType1, typename ArgType2, typename ArgType3, typename T, typename Func>
inline base::Callback<void(ArgType1, ArgType2)> LambdaRefCallback( T* pThis, Func const& func )
{
	struct Inner : public base::RefCountedThreadSafe<Inner>
	{
		Inner( Func const& func, T* pThis ) : func_(func), spThis(pThis) {}
		void Invoke(ArgType1 arg1, ArgType2 arg2, ArgType2 arg3) { func_(arg1, arg2, arg3); }
		Func func_;
		scoped_refptr<T> spThis;
	};

	scoped_refptr<Inner> spInner( new Inner(func, pThis) );
	return base::Bind( &Inner::Invoke, spInner );
}

}// namespace internal


// Event type with signature: base::event<void(std::string)>
template<typename Sig>
class event;

template <typename R>
class event<R(void)>
  : public internal::event_base< base::Callback<R(void)> > {};

template <typename R, typename A1>
class event<R(A1)>
  : public internal::event_base< base::Callback<R(A1)> > {};

template <typename R, typename A1, typename A2>
class event<R(A1, A2)>
  : public internal::event_base< base::Callback<R(A1, A2)> > {};
  
template <typename R, typename A1, typename A2, typename A3>
class event<R(A1, A2, A3)>
  : public internal::event_base< base::Callback<R(A1, A2, A3)> > {};

template <typename R, typename A1, typename A2, typename A3, typename A4>
class event<R(A1, A2, A3, A4)>
  : public internal::event_base< base::Callback<R(A1, A2, A3, A4)> > {};
  
template <typename R, typename A1, typename A2, typename A3, typename A4, typename A5>
class event<R(A1, A2, A3, A4, A5)>
  : public internal::event_base< base::Callback<R(A1, A2, A3, A4, A5)> > {};
  
template <typename R, typename A1, typename A2, typename A3, typename A4, typename A5, typename A6>
class event<R(A1, A2, A3, A4, A5, A6)>
  : public internal::event_base< base::Callback<R(A1, A2, A3, A4, A5, A6)> > {};

  
template <typename R, typename A1, typename A2, typename A3, typename A4, typename A5, typename A6, typename A7>
class event<R(A1, A2, A3, A4, A5, A6, A7)>
  : public internal::event_base< base::Callback<R(A1, A2, A3, A4, A5, A6, A7)> > {};
  
  
// 从event到base::Closure的辅助方法，用于event的异步调用
// 注意: 
// T 类型必须实现了Release & AddRef，用于保护event对象的生命期，一般就用this指针

template<typename T, typename R>
base::Closure make_event_closure( base::event<R()> * ev, T * pThis )
{
  scoped_refptr<T> self = make_scoped_refptr(pThis);
  auto lambda = [=]() 
  { 
    self; // 让lambda持有引用计数
    (*ev)(); 
  };
  return internal::LambdaClosure( lambda );
}

template<typename T, typename R, typename A1>
base::Closure make_event_closure( base::event<R(A1)> * ev, T * pThis, A1 const& a1 )
{
  scoped_refptr<T> self = make_scoped_refptr(pThis);
  auto lambda = [=]() { self; (*ev)(a1); };
  return internal::LambdaClosure( lambda );
}

template<typename T, typename R, typename A1, typename A2>
base::Closure make_event_closure( base::event<R(A1,A2)> * ev, T * pThis, A1 const& a1, A2 const& a2 )
{
  scoped_refptr<T> self = make_scoped_refptr(pThis);
  auto lambda = [=]() { self; (*ev)(a1, a2); };
  return internal::LambdaClosure( lambda );
}

template<typename T, typename R, typename A1, typename A2, typename A3>
base::Closure make_event_closure( base::event<R(A1,A2,A3)> * ev, T * pThis, A1 const& a1, A2 const& a2, A3 const& a3 )
{
  scoped_refptr<T> self = make_scoped_refptr(pThis);
  auto lambda = [=]() { self; (*ev)(a1, a2, a3); };
  return internal::LambdaClosure( lambda );
}

template<typename T, typename R, typename A1, typename A2, typename A3, typename A4>
base::Closure make_event_closure( base::event<R(A1,A2,A3,A4)> * ev, T * pThis, A1 const& a1, A2 const& a2, A3 const& a3, A4 const& a4 )
{
  scoped_refptr<T> self = make_scoped_refptr(pThis);
  auto lambda = [=]() { self; (*ev)(a1, a2, a3, a4); };
  return internal::LambdaClosure( lambda );
}

template<typename T, typename R, typename A1, typename A2, typename A3, typename A4, typename A5>
base::Closure make_event_closure( base::event<R(A1,A2,A3,A4,A5)> * ev, T * pThis, A1 const& a1, A2 const& a2, A3 const& a3
  , A4 const& a4, A5 const& a5 )
{
  scoped_refptr<T> self = make_scoped_refptr(pThis);
  auto lambda = [=]() { self; (*ev)(a1, a2, a3, a4, a5); };
  return internal::LambdaClosure( lambda );
}

template<typename T, typename R, typename A1, typename A2, typename A3, typename A4, typename A5, typename A6>
base::Closure make_event_closure( base::event<R(A1,A2,A3,A4,A5,A6)> * ev, T * pThis, A1 const& a1, A2 const& a2, A3 const& a3 
  , A4 const& a4, A5 const& a5, A6 const& a6 )
{
  scoped_refptr<T> self = make_scoped_refptr(pThis);
  auto lambda = [=]() { self; (*ev)(a1, a2, a3, a4, a5, a6); };
  return internal::LambdaClosure( lambda );
}

template<typename T, typename R, typename A1, typename A2, typename A3, typename A4, typename A5, typename A6, typename A7>
base::Closure make_event_closure( base::event<R(A1,A2,A3,A4,A5,A6,A7)> * ev, T * pThis, A1 const& a1, A2 const& a2, A3 const& a3 
  , A4 const& a4, A5 const& a5, A6 const& a6, A7 const& a7 )
{
  scoped_refptr<T> self = make_scoped_refptr(pThis);
  auto lambda = [=]() { self; (*ev)(a1, a2, a3, a4, a5, a6, a7); };
  return internal::LambdaClosure( lambda );
}

template<typename Func>
inline base::Closure make_lambda_closure(Func const& fun)
{
	return internal::LambdaClosure(fun);
};
	
template<typename ArgType, typename Func>
inline base::Callback<void(ArgType)> make_lambda_arg_callback(Func const& fun)
{
	return internal::LambdaArgCallback<ArgType>(fun);
};
    
template<typename ArgType1, typename ArgType2, typename Func>
inline base::Callback<void(ArgType1, ArgType2)> make_lambda_arg_callback(Func const& fun)
{
    return internal::LambdaArgCallback<ArgType1, ArgType2>(fun);
};

template<typename ArgType1, typename ArgType2, typename ArgType3, typename Func>
inline base::Callback<void(ArgType1, ArgType2, ArgType3)> make_lambda_arg_callback(Func const& fun)
{
	return internal::LambdaArgCallback<ArgType1, ArgType2, ArgType3>(fun);
};

template<typename ArgType1, typename ArgType2, typename ArgType3, typename ArgType4, typename Func>
inline base::Callback<void(ArgType1, ArgType2, ArgType3, ArgType4)> make_lambda_arg_callback(Func const& fun)
{
	return internal::LambdaArgCallback<ArgType1, ArgType2, ArgType3, ArgType4>(fun);
};

template<typename T, typename Func>
inline base::Closure make_lambda_ref_callback(T* pThis, Func const& fun)
{
	return internal::LambdaRefCallback(pThis, fun);
};

template<typename ArgType, typename T, typename Func>
inline base::Callback<void(ArgType)> make_lambda_ref_callback(T* pThis, Func const& fun)
{
	return internal::LambdaRefCallback<ArgType>(pThis, fun);
};

template<typename ArgType1, typename ArgType2, typename T, typename Func>
inline base::Callback<void(ArgType1, ArgType2)> make_lambda_ref_callback(T* pThis, Func const& fun)
{
	return internal::LambdaRefCallback<ArgType1, ArgType2>(pThis, fun);
};

template<typename ArgType1, typename ArgType2, typename ArgType3, typename T, typename Func>
inline base::Callback<void(ArgType1, ArgType2, ArgType3)> make_lambda_ref_callback(T* pThis, Func const& fun)
{
	return internal::LambdaRefCallback<ArgType1, ArgType2, ArgType3>(pThis, fun);
};

}// namespace base

#if defined(OS_MACOSX)
#pragma clang diagnostic pop
#endif

#endif// BASE_EVENT_H
