#pragma once
#include "base/prg.h"

//Error Code type;
//
//  Values are 32 bit values laid out as follows:
//
//   3 3 2 2 2 2 2 2 2 2 2 2 1 1 1 1 1 1 1 1 1 1
//   1 0 9 8 7 6 5 4 3 2 1 0 9 8 7 6 5 4 3 2 1 0 9 8 7 6 5 4 3 2 1 0
//  +---+-+-+-----------------------+-------------------------------+
//  |Sev|C|R|     Facility          |               Code            |
//  +---+-+-+-----------------------+-------------------------------+
//
//  where
//
//      Sev - is the severity code
//
//          00 - Success
//          01 - Informational
//          10 - Warning
//          11 - Error
//
//      C - is the Customer code flag
//
//      R - is a reserved bit
//
//      Facility - is the facility code
//
//      Code - is the facility's status code
//
//
// Define the facility codes
//


//
// Define the severity codes
//

// PrgDB Facility Code: 301   

#define PRGDB_HRESULT_FACILITY_START 301

#define PRGDB_HRESULT_CODE_START 0

//
// MessageId: E_PRGDB_NODBNAME
//
// MessageText:
//
// 该name没有对应的路径和key，有可能没有做InitConnect的调用
//
const HRESULT E_PRGDB_NODBNAME = PRGMAKEERROR_HRESULT(PRGDB_HRESULT_FACILITY_START, PRGDB_HRESULT_CODE_START);

//
// MessageId: E_PRGDB_OPENDBFAIL
//
// MessageText:
//
// 打开该db文件失败
//
const HRESULT E_PRGDB_OPENDBFAIL = PRGMAKEERROR_HRESULT(PRGDB_HRESULT_FACILITY_START, PRGDB_HRESULT_CODE_START + 1);

//
// MessageId: E_PRGDB_INVALIDKEY
//
// MessageText:
//
// 打开该db文件的秘钥无效
//
const HRESULT E_PRGDB_INVALIDKEY = PRGMAKEERROR_HRESULT(PRGDB_HRESULT_FACILITY_START, PRGDB_HRESULT_CODE_START + 2);

//
// MessageId: E_PRGDB_CLOSEERROR
//
// MessageText:
//
// 关闭数据库失败
//
const HRESULT E_PRGDB_CLOSEERROR = PRGMAKEERROR_HRESULT(PRGDB_HRESULT_FACILITY_START, PRGDB_HRESULT_CODE_START + 3);

//
// MessageId: E_PRGDB_SQLITENULL
//
// MessageText:
//
// 数据库未打开下，无法操作数据库
//
const HRESULT E_PRGDB_SQLITENULL = PRGMAKEERROR_HRESULT(PRGDB_HRESULT_FACILITY_START, PRGDB_HRESULT_CODE_START + 4);

//
// MessageId: E_PRGDB_SQLERROR
//
// MessageText:
//
// sql语法错误
//
const HRESULT E_PRGDB_SQLERROR = PRGMAKEERROR_HRESULT(PRGDB_HRESULT_FACILITY_START, PRGDB_HRESULT_CODE_START + 5);

//
// MessageId: E_PRGDB_SQLSTATEMENTNULL
//
// MessageText:
//
// sql Statement为空
//
const HRESULT E_PRGDB_SQLSTATEMENTNULL = PRGMAKEERROR_HRESULT(PRGDB_HRESULT_FACILITY_START, PRGDB_HRESULT_CODE_START + 6);

//
// MessageId: S_PRGDB_COMMANDBINDERROR
//
// MessageText:
//
// sql command bind错误
//
const HRESULT S_PRGDB_COMMANDBINDERROR  = PRGMAKEERROR_HRESULT(PRGDB_HRESULT_FACILITY_START, PRGDB_HRESULT_CODE_START + 7);

//
// MessageId: S_PRGDB_FILEDNAMENOEXIST
//
// MessageText:
//
// 不存在这个列名
//
const HRESULT S_PRGDB_FILEDNAMENOEXIST  = PRGMAKEERROR_HRESULT(PRGDB_HRESULT_FACILITY_START, PRGDB_HRESULT_CODE_START + 8);

//
// MessageId: S_PRGDB_FILEDINDEXNOEXIST
//
// MessageText:
//
// 不存在这个列索引
//
const HRESULT S_PRGDB_FILEDINDEXNOEXIST  = PRGMAKEERROR_HRESULT(PRGDB_HRESULT_FACILITY_START, PRGDB_HRESULT_CODE_START + 9);
