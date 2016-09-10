//------------------------------------------------------------------------------
//
//    版权所有(C) 阿里巴巴（中国）网络技术有限公司 保留所有权利
//
//    创建者:   徐明
//    创建日期: 2015-01-29
//    功能描述: COM类接口定义
//
//
//------------------------------------------------------------------------------

#ifndef __ICOMREFCOUNTED_H_A7FD4DC8_2445_49C6_AE6F_2B6CF296A471_INCLUDED__
#define __ICOMREFCOUNTED_H_A7FD4DC8_2445_49C6_AE6F_2B6CF296A471_INCLUDED__

#include "base/namespace.h"
#include "base/prg.h"
#include "string"
#include "base/memory/iref_counted.h"

BEGIN_NAMESPACE(prg)

template<typename T>
class CPrgCOMUUID
{
public:
  static std::string GetUUID();
};

class IPrgCOMRefCounted : public base::IRefCounted
{
public:
	virtual HRESULT QueryInterface(const std::string &uuid, void **ppObject) = 0;
};

END_NAMESPACE()


// Macros for prg CLSID and IID

#define DEFINE_CLSID(T, CLSID) \
  class T##UUID \
  { \
  }; \
  namespace prg { \
  template<> \
  class CPrgCOMUUID<T##UUID> \
  { \
  public: \
    static std::string GetUUID() \
    { \
      return CLSID; \
    } \
  }; } // namespace prg

#define DEFINE_IID(T, IID) \
  namespace prg { \
  template<> \
  class CPrgCOMUUID<T> \
  { \
  public: \
    static std::string GetUUID() \
    { \
      return IID; \
    } \
  }; } // namespace prg

#define i_uuidof(T) \
  prg::CPrgCOMUUID<T>::GetUUID()

#define c_uuidof(T) \
  prg::CPrgCOMUUID<T##UUID>::GetUUID()

DEFINE_IID(prg::IPrgCOMRefCounted, "{8D0890E0-9AFB-4426-A830-48A27EB4475A}")

#endif  // __ICOMREFCOUNTED_H_A7FD4DC8_2445_49C6_AE6F_2B6CF296A471_INCLUDED__
