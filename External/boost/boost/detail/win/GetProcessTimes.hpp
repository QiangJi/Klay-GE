//  GetProcessTimes.hpp  --------------------------------------------------------------//

//  Copyright 2010 Vicente J. Botet Escriba

//  Distributed under the Boost Software License, Version 1.0.
//  See http://www.boost.org/LICENSE_1_0.txt


#ifndef BOOST_DETAIL_WIN_GETPROCESSTIMES_HPP
#define BOOST_DETAIL_WIN_GETPROCESSTIMES_HPP

#include <boost/detail/win/time.hpp>

namespace boost {
namespace detail {
namespace win32 {
// Windows CE and the new WinRT do not define GetProcessTimes
#if !defined(UNDER_CE) && !(defined(WINAPI_FAMILY) && (WINAPI_FAMILY == WINAPI_FAMILY_APP))
#if defined( BOOST_USE_WINDOWS_H )
    using ::GetProcessTimes;
#else
    extern "C" __declspec(dllimport) BOOL_ WINAPI
        GetProcessTimes(
            HANDLE_ hProcess,
            LPFILETIME_ lpCreationTime,
            LPFILETIME_ lpExitTime,
            LPFILETIME_ lpKernelTime,
            LPFILETIME_ lpUserTime
        );
#endif
#endif // Windows CE || WinRT
}
}
}

#endif // BOOST_DETAIL_WIN_GETPROCESSTIMES_HPP
