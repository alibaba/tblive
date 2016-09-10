//------------------------------------------------------------------------------
//
//    版权所有(C) 阿里巴巴（中国）网络技术有限公司 保留所有权利
//
//    创建者:   徐明
//    创建日期: 2015-1-29
//    功能描述: 
//
//
//------------------------------------------------------------------------------

#ifndef __PrgDBExport_H_6063515E_090A_446C_B30C_039EE75D0E49_INCLUDED__
#define __PrgDBExport_H_6063515E_090A_446C_B30C_039EE75D0E49_INCLUDED__

#ifdef PrgDB_EXPORTS
#define PrgDB_API Prg_API_Export
#else
#define PrgDB_API Prg_API_Import
#endif

#endif // #ifndef __PrgDBExport_H_6063515E_090A_446C_B30C_039EE75D0E49_INCLUDED__
