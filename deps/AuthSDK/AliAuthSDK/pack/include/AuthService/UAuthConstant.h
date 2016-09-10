#pragma once

namespace uauth
{
    ///////////////////////// Login //////////////////////////
    enum LoginType
    {
        Type_RealPass = 0, //‘≠ º√‹¬Î
        Type_Token,
    };

    enum HTTP_CLIENT_RPOXY_TYPE {
        HTTP_CLIENT_RPOXY_NONE = 0,
        HTTP_CLIENT_RPOXY_SOCKS5,
        HTTP_CLIENT_RPOXY_HTTP,
        HTTP_CLIENT_RPOXY_IE,
    };
}