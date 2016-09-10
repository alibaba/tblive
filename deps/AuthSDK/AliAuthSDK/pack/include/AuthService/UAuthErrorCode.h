#pragma once

namespace uauth
{
    ///////////////////////// Login //////////////////////////
    enum AuthErrorCode
    {
        /**
        * 初始化数据失败
        */
        AuthErrorCode_InitError = -1,
        /**
        * 连接服务器失败
        */
        AuthErrorCode_ConnError = -2,
        /**
        * 服务器通信超时
        */
        AuthErrorCode_Timeout = -4,
        /**
        * 服务器回包无法解析
        */
        AuthErrorCode_RspError = -5,
        /**
        * 未知错误
        */
        AuthErrorCode_UnkownError = -6,


        AUTH_PWD_ERROR           = 0x1,
        AUTH_TOKEN_ERROR         = 0x2,
        AUTH_TOKEN_EXPIRED       = 0x3,
        AUTH_NOT_ACTIVE          = 0x4,
        AUTH_NOT_EXIST           = 0x5,
        AUTH_LIMITED             = 0x6,
        AUTH_CTU_LIMITED         = 0x7,
        AUTH_NEED_CHECKCODE      = 0x8,
        AUTH_NEED_2NDAUTH        = 0x9,
        AUTH_2NDAUTH_ERROR       = 0xa,
        AUTH_CHILD_EXPIRED       = 0xb,
        AUTH_CHILD_STOPED        = 0xc,
        AUTH_CHILD_PAUSED        = 0xd,
        AUTH_INVALID_SERVER      = 0xe,
        AUTH_INVALID_USER        = 0xf,
        AUTH_UIC_LIMITED         = 0x10,
        AUTH_CHECKCODE_ERROR     = 0x11,
        AUTH_TRUSTTOKEN_EXPIRED  = 0x12,
        AUTH_SSOLOGIN_ERROR      = 0x13,
        AUTH_SESSION_ERROR       = 0x80,
        AUTH_SESSION_EXPIRED     = 0x81,
        AUTH_VERSION_ERROR		= 0x82,
        AUTH_INVALID_PARAM       = 0xfc,
        AUTH_SYS_ERROR           = 0xfd,
        AUTH_UNKNOW_ERROR        = 0xfe,
    };
}