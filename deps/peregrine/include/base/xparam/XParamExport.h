//------------------------------------------------------------------------------
//
//    版权所有(C) 浙江天猫技术有限公司 保留所有权利
//
//    创建者:   哈尔
//    创建日期: 2015-2-4
//    功能描述: XParam的导出函数
//
//------------------------------------------------------------------------------

#ifndef __XPARAMEXPORT_H_6F88B155_85FE_44C9_AE38_4E546788303F_INCLUDED__
#define __XPARAMEXPORT_H_6F88B155_85FE_44C9_AE38_4E546788303F_INCLUDED__

#ifdef XParam_EXPORTS
#define XParam_API Prg_API_Export
#else
#define XParam_API Prg_API_Import
#endif

#endif // #__XPARAMEXPORT_H_6F88B155_85FE_44C9_AE38_4E546788303F_INCLUDED__
