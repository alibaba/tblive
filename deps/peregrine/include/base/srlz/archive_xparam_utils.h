#ifndef __ARCHIVE_XPARAM_UTILS_H_AD81B9F0_043C_46de_861A_A5C30F60D657__
#define __ARCHIVE_XPARAM_UTILS_H_AD81B9F0_043C_46de_861A_A5C30F60D657__

#include <base/xparam/XParam.h>
#include <base/files/file_path.h>
#include "./archive_xparam_basic.h"

NS_SERIALIZATION_LITE_BEGIN

class XParamSLHelper
{
public:
	XParamSLHelper() {}

	template <typename T>
	static bool FromXParam(xparam::XParamP const& p, T const& t)
	{
		if (!p)
			return false;
		iarchive_xparam(p) >> const_cast<T&>(t);
		return true;
	}

	template <typename T>
	static xparam::XParamP ToXParam(const std::wstring& strParamName, T const& t)
	{
		if (strParamName.length() == 0)
        {
            return xparam::XParamP();
        }
		xparam::XParamP root = xparam::CreateXParam();
		oarchive_xparam(L"", root) << t;
		root->GetDocument()->SetName(strParamName.c_str(), xparam::ZERO_TERMINATOR);
		return root->GetDocument();
	}

	template <typename T>
	static bool FromString(const std::wstring& s, T const& t)
	{
		xparam::XParamParserP parser = xparam::CreateXParamParser();
		if (!parser->ParseString(s.c_str(), s.length()))
			return false;
		return FromXParam(parser->Root()->GetDocument(), t);
	}

	template <typename T>
	static bool ToString(
		std::wstring& s, 
		const std::wstring& strParamName, 
		T const& t, 
		wchar_t const* indent = L"\t", 
		wchar_t const* crlf = L"\r\n"
		)
	{
		xparam::XParamP p = ToXParam(strParamName, t);
		if (!p)
			return false;
		xparam::XParamTraversalP traversal = xparam::CreateXParamTraversal();
		xparam::XInResP inres;
		size_t dlen;
		wchar_t const* dst = traversal->ToString(p, &dlen, indent, crlf, inres);
		s.assign(dst, dlen);
		return true;
	}

	template <typename T>
	static bool FromFile(
		const base::FilePath &filePath, 
		T const& t
	)
	{
		xparam::XParamParserP parser = xparam::CreateXParamParser();
		if (!parser->ParseFile(filePath))
			return false;
		return FromXParam(parser->Root()->GetDocument(), t);
	}

	template <typename T>
	static bool ToFile(
		const base::FilePath &filePath,
		const std::wstring& strParamName, 
		T const& t,
		const std::wstring& strEncoding = L"utf-8",
		const std::wstring& strIndent = L"\t", 
		const std::wstring& strCrlf = L"\r\n"
		)
	{
		xparam::XParamP p = ToXParam(strParamName, t);
		if (!p)
			return false;
		xparam::XParamTraversalP traversal = xparam::CreateXParamTraversal();
		return traversal->ToFile(p, filePath, strIndent.c_str(), strCrlf.c_str(), strEncoding.c_str());
	}
};

NS_SERIALIZATION_LITE_END

#endif//__ARCHIVE_XPARAM_UTILS_H_AD81B9F0_043C_46de_861A_A5C30F60D657__
