#ifndef __SRLZ_STANDIN_H_77BA134C_33D3_469B_B1F1_684B631B09E7__
#define __SRLZ_STANDIN_H_77BA134C_33D3_469B_B1F1_684B631B09E7__


#include "./srlz_base.h"
//#include <boost/smart_ptr.hpp>

NS_SERIALIZATION_LITE_BEGIN

class standin
{
public:
	// inline static const char* static_type() {return "";}
	virtual const std::wstring type() const = 0;
	virtual ~standin(){}
};
//
//template<typename T>
//boost::shared_ptr<T> standin_cast(boost::shared_ptr<standin> const& src)
//{
//	if (src && (src->type() == T::static_type()))
//		return boost::static_pointer_cast<T>(src);
//	else
//		return boost::shared_ptr<T>();
//}


NS_SERIALIZATION_LITE_END


#endif//__SRLZ_STANDIN_H_77BA134C_33D3_469B_B1F1_684B631B09E7__
