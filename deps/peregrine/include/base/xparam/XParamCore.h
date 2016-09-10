#ifndef __XPARAM_H_825479BD_F46A_4b42_9D1B_DC0B0442BF5D__
#define __XPARAM_H_825479BD_F46A_4b42_9D1B_DC0B0442BF5D__

#include <stddef.h>
#include <base/prg.h>
#include <base/memory/ref_counted.h>
#include <base/memory/iref_counted.h>
#include <base/files/file_path.h>

#ifndef __IUnknown_INTERFACE_DEFINED__
struct IUnknown;
#endif
#ifndef __IDispatch_INTERFACE_DEFINED__
struct IDispatch;
#endif

namespace xparam
{

enum {ZERO_TERMINATOR = ((size_t)-1)};

enum X_VALUE_TYPE
{
	xvt_empty = 0,
	xvt_beginof_basic = 0x10,
	xvt_char, 
	xvt_uchar, 
	xvt_short,
	xvt_ushort,
	xvt_long,
	xvt_ulong,
	xvt_longlong,
	xvt_ulonglong,
	xvt_int,
	xvt_uint,
	xvt_float,
	xvt_double,
	xvt_longdouble,
	xvt_bool,
	xvt_endof_basic = 0x2f,

	xvt_beginof_pointer = 0x30,
	xvt_pvoid,
	xvt_sp,
	xvt_iunknown,
	xvt_idispatch,
	xvt_irefcount,
	xvt_endof_pointer = 0x4f,

	xvt_string,
	xvt_blob,
};

enum X_VALUE_NODE
{
	xvn_text,
	xvn_cdata,
	xvn_attr
};

enum X_PARSE_ERRORS
{
	xpe_no_error				= 0,	// 正确，没有错误
	xpe_unknown					= 1,	// 未知错误
	xpe_callback_cancel,				// 解析过程被回调函数取消
	xpe_null_root,						// 没有根节点，通常是因为没有调用BeginParse，或者在解析过程中调用了DetachRoot
	xpe_need_initialize,				// 调用Parsing前没有调用BeginParse
	xec_unescape_failed,				// 反转义失败，通常是读到了错误的转义编码
	xec_unexpected_char,				// 读到非法字符
	xec_unmatched_tag,					// 标签不匹配
	xec_too_much_doc_element,			// 文档元素太多，xml规范规定每个xml文档只能有一个文档元素
	xec_decode_failed,					// 解码失败
	xec_cannot_switch_coding,			// 无法切换编码
	xec_mssing_tail,					// 找不到文档末尾的结束标签
	xec_read_file_failed,				// 读取文件错误
	xec_write_file_failed,				// 写文件错误
	xec_not_supported					// 不支持的操作
};

class XParam;
typedef scoped_refptr<XParam> XParamP;
class XInRes; // inner resources
typedef scoped_refptr<XInRes> XInResP;
class XParamParser;
typedef scoped_refptr<XParamParser> XParamParserP;
class XParamTraversal;
typedef scoped_refptr<XParamTraversal> XParamTraversalP;


class XParam
{
public:

	virtual ~XParam(){}
	virtual XParamP Clone() const = 0;
	virtual XParamP CreateXParam() const = 0;

	// value type methods
	virtual X_VALUE_TYPE GetValueType() const = 0;
	virtual bool IsEmptyType() const = 0;
	virtual bool IsBasicType() const = 0;
	virtual bool IsStringCompatibleType() const = 0;
	virtual bool IsBlobType() const = 0;
	virtual bool IsPointerType() const = 0;

	// value implementation methods
	virtual X_VALUE_NODE GetValueNode() const = 0;
	virtual void SetValueNode(X_VALUE_NODE xvn)= 0;

	// name methods
	virtual bool GetName(wchar_t const*& s, size_t& len, XInResP& inres) const = 0;
	virtual void SetName(wchar_t const* s, size_t len = ZERO_TERMINATOR) = 0;

	// get basic value methods
	virtual bool GetValue(char& val) const = 0;
	virtual bool GetValue(unsigned char& val) const = 0;
	virtual bool GetValue(short& val) const = 0;
	virtual bool GetValue(unsigned short& val) const = 0;
	virtual bool GetValue(long& val) const = 0;
	virtual bool GetValue(unsigned long& val) const = 0;
	virtual bool GetValue(long long& val) const = 0;
	virtual bool GetValue(unsigned long long& val) const = 0;
	virtual bool GetValue(int& val) const = 0;
	virtual bool GetValue(unsigned int& val) const = 0;
	virtual bool GetValue(float& val) const = 0;
	virtual bool GetValue(double& val) const = 0;
	virtual bool GetValue(long double& val) const = 0;
	virtual bool GetValue(bool& val) const = 0;

	// get pointer value methods
	virtual bool GetValue(void*& val) const = 0;
//	virtual bool GetValue(boost::shared_ptr<void>& val) const = 0;

	// get string value methods
	virtual bool GetValue(wchar_t const*& s, size_t& len, XInResP& inres) const = 0;
    virtual bool GetValue(char const*& s, size_t& len, XInResP& inres) const = 0;

	// get blob value methods
	virtual bool GetValue(void const*& p, size_t& size, XInResP& inres) const = 0;


	// set null value methods
	virtual void SetValue() = 0;

	// set basic value methods
	virtual void SetValue(char val) = 0;
	virtual void SetValue(unsigned char val) = 0;
	virtual void SetValue(short val) = 0;
	virtual void SetValue(unsigned short val) = 0;
	virtual void SetValue(long val) = 0;
	virtual void SetValue(unsigned long val) = 0;
	virtual void SetValue(long long val) = 0;
	virtual void SetValue(unsigned long long val) = 0;
	virtual void SetValue(int val) = 0;
	virtual void SetValue(unsigned int val) = 0;
	virtual void SetValue(float val) = 0;
	virtual void SetValue(double val) = 0;
	virtual void SetValue(long double val) = 0;
	virtual void SetValue(bool val) = 0;

	// set pointer value methods
	virtual void SetValue(void* val) = 0;
//	virtual void SetValue(boost::shared_ptr<void> const& val) = 0;

	// set string value methods
	virtual void SetValue(wchar_t const* s, size_t len = ZERO_TERMINATOR) = 0;
    virtual void SetValue(char const* s, size_t len = ZERO_TERMINATOR) = 0;

	// set blob value methods
	virtual void SetValue(void const* p, size_t size) = 0;

	virtual bool GetValue( base::IRefCounted*& val ) const = 0;
	virtual void SetValue( base::IRefCounted* val ) = 0;

#ifdef WIN32
    virtual bool GetValue(IUnknown*& val) const = 0;
    virtual bool GetValue(IDispatch*& val) const = 0;
    virtual void SetValue(IUnknown* val) = 0;
    virtual void SetValue(IDispatch* val) = 0;
#endif

	// parent methods
	virtual XParamP GetParent() const = 0;

	// Child methods
	virtual void PushBackChild(XParamP const& child) = 0;
	virtual void PushFrontChild(XParamP const& child) = 0;
	virtual void InsertBefore(XParamP const& before, XParamP const& child) = 0;
	virtual void InsertAfter(XParamP const& after, XParamP const& child) = 0;

	virtual void DropChild(XParamP const& child) = 0;
	virtual void DropSelf() = 0;
	virtual void ClearChildren() = 0;

	virtual XParamP GetDocument() const = 0;
	virtual XParamP GetFirstChild() const = 0;
	virtual XParamP GetFirstChild(wchar_t const* s, size_t len = ZERO_TERMINATOR, size_t offset = 0) const = 0;
	virtual XParamP GetLastChild() const = 0;
	virtual XParamP GetLastChild(wchar_t const* s, size_t len = ZERO_TERMINATOR, size_t offset = 0) const = 0;
	virtual size_t GetChildrenCount() const = 0;

	// sibling methods
	virtual XParamP GetPrevSibling() const = 0;
	virtual XParamP GetPrevNamesake() const = 0;
	virtual XParamP GetNextSibling() const = 0;
	virtual XParamP GetNextNamesake() const = 0;

	// index methods
	virtual bool HasIndex() const = 0;
	virtual void BuildIndex() = 0;
	virtual void ClearIndex() = 0;
  
  virtual void AddRef() const = 0;
  virtual void Release() const = 0;
};

//////////////////////////////////////////////////////////////////////////
class XInRes
{
public:
	virtual ~XInRes(){};
  virtual void AddRef() const = 0;
  virtual void Release() const = 0;
};

//////////////////////////////////////////////////////////////////////////

class XParamParser
{
public:
	virtual ~XParamParser(){}

	virtual bool ParseString(wchar_t const* s, size_t len) = 0;

	virtual bool ParseData(const void* data, size_t size) = 0;

	virtual bool ParseFile(const base::FilePath &filePath) = 0;

	virtual size_t Row() const = 0;
	virtual size_t Col() const = 0;
	virtual size_t Offset() const = 0;
	virtual unsigned int ErrorCode() const = 0;
	virtual XParamP Root() const = 0;
	virtual XParamP DetachRoot() = 0;
  virtual void AddRef() const = 0;
  virtual void Release() const = 0;
};

//////////////////////////////////////////////////////////////////////////
class XParamTraversal
{
public:
	typedef bool (*CallbackType)(XParamP const& node, void* context);
	virtual ~XParamTraversal(){}

	virtual void Traverse(
		XParamP const& node, 
		CallbackType callback, 
		void* context
		) const = 0;

	virtual wchar_t const* ToString(
		XParamP const& node, 
		size_t* dlen, 
		wchar_t const* indent,
		wchar_t const* crlf,
		XInResP& inres
		) const = 0;

	virtual bool ToFile(
		XParamP const& node, 
		const base::FilePath &filePath,
		const wchar_t* indent, 
		const wchar_t* crlf, 
		const wchar_t* encoding
		) const = 0;
  virtual void AddRef() const = 0;
  virtual void Release() const = 0;
};
  
//////////////////////////////////////////////////////////////////////////

// create xparam function decoration. add by hongwei.huhw
BASE_EXPORT XParamP CreateXParam(wchar_t const* s = NULL, size_t len = ZERO_TERMINATOR);
BASE_EXPORT XParamParserP CreateXParamParser();
BASE_EXPORT XParamTraversalP CreateXParamTraversal();

}

#endif//__XPARAM_H_825479BD_F46A_4b42_9D1B_DC0B0442BF5D__

