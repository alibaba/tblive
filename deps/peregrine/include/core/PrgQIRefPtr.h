//------------------------------------------------------------------------------
//
//    版权所有(C) 阿里巴巴（中国）网络技术有限公司 保留所有权利
//
//    创建者:   徐明
//    创建日期: 2015-1-29
//    功能描述: 带有QueryInterface的智能指针
//
//
//------------------------------------------------------------------------------

#ifndef __PrgQIRefPtr_H_6063515E_090A_446C_B30C_039EE75D0E49_INCLUDED__
#define __PrgQIRefPtr_H_6063515E_090A_446C_B30C_039EE75D0E49_INCLUDED__

#include "base/namespace.h"
#include "base/memory/ref_counted.h"
#include "core/IPrgCOMRefCounted.h"

BEGIN_NAMESPACE(prg)
template <class T>
class CPrgQIRefPtr:public scoped_refptr<T>
{
public:
	CPrgQIRefPtr():scoped_refptr<T>()
	{

	}

	CPrgQIRefPtr(T *pIRef):scoped_refptr<T>(pIRef)
	{

	}

	CPrgQIRefPtr(const CPrgQIRefPtr<T> &spIRef):scoped_refptr<T>(spIRef.get())
	{
	}

	template<typename U>
	CPrgQIRefPtr(U *pIRef)
	{
		if (pIRef)
		{
            pIRef->QueryInterface(i_uuidof(T), (void**)&this->ptr_);
		}
	}

	template<typename U>
	CPrgQIRefPtr(const CPrgQIRefPtr<U> &spIRef)
	{
		if (spIRef)
		{
            spIRef->QueryInterface(i_uuidof(T), (void**)&this->ptr_);
		}
	}

	CPrgQIRefPtr<T> &operator =(T *pIRef)
	{
		if (pIRef)
			pIRef->AddRef();
		T* old_ptr = this->ptr_;
		this->ptr_ = pIRef;
		if (old_ptr)
			old_ptr->Release();
		return *this;
	}

	CPrgQIRefPtr<T> &operator =(const CPrgQIRefPtr<T> &spIRef)
	{
		T* old_ptr = this->ptr_;
		this->ptr_ = spIRef.get();
		if (this->ptr_)
		{
			this->ptr_->AddRef();
		}
		if (old_ptr)
			old_ptr->Release();
		return *this;
	}

	template<typename U>
	CPrgQIRefPtr<T> &operator =(const CPrgQIRefPtr<U> &spIRef)
	{
		T* old_ptr = this->ptr_;
		U *t = spIRef.get();
		if (t)
		{
			t->QueryInterface(i_uuidof(T), (void**)&this->ptr_);
		}
		if (old_ptr)
			old_ptr->Release();
		return *this;
	}
};
END_NAMESPACE()

#endif // #ifndef __PrgQIRefPtr_H_6063515E_090A_446C_B30C_039EE75D0E49_INCLUDED__
