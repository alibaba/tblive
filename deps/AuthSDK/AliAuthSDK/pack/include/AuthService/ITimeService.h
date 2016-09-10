#pragma once

#include <memory>
#include <vector>

#include <core/PrgQIRefPtr.h>
#include <core/IPrgCOMRefCounted.h>
#include <base/memory/iref_counted.h>

namespace uauth{

class ITimeService : public prg::IPrgCOMRefCounted
{
public:
    //************************************************************************
    // Description:获取本地服务器时间
    // Returns:    int64 服务器时间
    // Parameter:  bool bMsec 是否为毫秒
    //************************************************************************
    virtual int64 GetLocalServerTime(bool bMsec = false) = 0;

    //************************************************************************
    // Description:更新本地服务器时间
    // Returns:    void
    // Parameter:  const int64 time
    //************************************************************************
    virtual void UpdateLocalServerTime(const int64 time) = 0;
};
}

DEFINE_IID(uauth::ITimeService, "{5EFB1312-B6F8-4718-A766-C78BEF5BEC50}")
DEFINE_CLSID(TimeService, "{D99F2615-694A-4202-BC20-199CB6E5A3FA}")

