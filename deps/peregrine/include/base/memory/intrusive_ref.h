//------------------------------------------------------------------------------
//
//    版权所有(C) 阿里巴巴（中国）网络技术有限公司 保留所有权利
//
//    创建者:   寺原
//    创建日期: 2015-12-30
//    功能描述:
//        scopt_ptr<CIntrusiveStrong> sp; 
//        CIntrusiveWeakRef weak(sp.get());
//
//
//------------------------------------------------------------------------------

#ifndef __IntrusiveRef_HELPER_H_75895926_20F7_40B4_A1F4_718D2F4AFFA1_INCLUDED__
#define __IntrusiveRef_HELPER_H_75895926_20F7_40B4_A1F4_718D2F4AFFA1_INCLUDED__

#include <base\atomic_ref_count.h>


class CIntrusiveWeakRef;
class CIntrusiveStrong
{   // 支持 侵入式指针.
    friend CIntrusiveWeakRef;
    typedef base::AtomicRefCount TypeCount;
    enum {XOR_MAGIC_NUM = 0x19791130}; // 魔术数字，用来交验合法性
    volatile base::AtomicRefCount ref_count;
    volatile base::AtomicRefCount m_orVerrifyNum; // 用来做有效性的校验.
public:
    CIntrusiveStrong():ref_count(0),
        m_orVerrifyNum(XOR_MAGIC_NUM^TypeCount(this)) { }
    virtual ~CIntrusiveStrong(){ m_orVerrifyNum = ref_count = 0; }
    bool HasOneRef()
    {
        return base::AtomicRefCountIsOne(&ref_count);
    }
    TypeCount AddRef()
    {
        return base::AtomicRefCountIncN(&ref_count, 1);
    }
    TypeCount Release()
    {
        const TypeCount nRef = base::AtomicRefCountDecN(&ref_count, 1);
        if ( 0 == nRef )
        {   // 多线程进入的时候, 只会有一个等于 0 的.
            m_orVerrifyNum = ref_count = 0;
            delete this; return 0;
        }
        return nRef > 0 ? nRef : 0;
    }
};

class CIntrusiveWeakRef
{
    CIntrusiveStrong* m_pStrongRef;
    static const long XOR_MAGIC_NUM = CIntrusiveStrong::XOR_MAGIC_NUM;
    typedef CIntrusiveStrong::TypeCount TypeCount;
public:
    virtual ~CIntrusiveWeakRef(){ m_pStrongRef = NULL; }
    CIntrusiveWeakRef(CIntrusiveStrong* pStrongRef):m_pStrongRef(pStrongRef){}
    template< class I > bool GetStrongRef( I **ppStrongRef )
    {
        if ( !m_pStrongRef || !ppStrongRef )
        {
            return false;
        }

        do
        {
            TypeCount counter = m_pStrongRef->ref_count;
            if ( counter <= 0 || m_pStrongRef->m_orVerrifyNum != (XOR_MAGIC_NUM^TypeCount(m_pStrongRef)) )
            {   // 引用计数小于零，或者是对象已经明确被销毁。
                return false;
            }

            if (m_pStrongRef->ref_count == counter)
            {
                base::AtomicRefCountIncN(&m_pStrongRef->ref_count, 1);
                if ( m_pStrongRef->m_orVerrifyNum == (XOR_MAGIC_NUM^TypeCount(m_pStrongRef)) )
                {   // 校验有效性成功. 
                    I* pDestObject = NULL;
                    try {
                        pDestObject = dynamic_cast<I*>(m_pStrongRef);
                    } catch( ... ){ pDestObject = nullptr; return false; }

                    *ppStrongRef = pDestObject;
                    return pDestObject ? true : false;
                } return false; 
            }
        } while ( true ); return false;
    }
};
#endif // #ifndef __IntrusiveRef_HELPER_H_75895926_20F7_40B4_A1F4_718D2F4AFFA1_INCLUDED__
