set CXXFLAGS="-I%ANDROID_NDK%/platforms/android-9/arch-arm/usr/include -I%ANDROID_NDK%/sources/cxx-stl/gnu-libstdc++/4.6/include -I%ANDROID_NDK%/sources/cxx-stl/gnu-libstdc++/4.6/libs/armeabi/include"
set TOOLSET=gcc-android_armeabi
bjam --toolset=%TOOLSET% --user-config=./user-config-android-armeabi.jam cxxflags=%CXXFLAGS% threadapi=pthread --stagedir=./lib_android_armeabi --builddir=./ target-os=linux --without-atomic --without-date_time --without-program_options --without-regex --disable-filesystem2 link=static runtime-link=static threading=multi stage

set CXXFLAGS="-I%ANDROID_NDK%/platforms/android-9/arch-arm/usr/include -I%ANDROID_NDK%/sources/cxx-stl/gnu-libstdc++/4.6/include -I%ANDROID_NDK%/sources/cxx-stl/gnu-libstdc++/4.6/libs/armeabi-v7a/include"
set TOOLSET=gcc-android_armeabi_v7a
bjam --toolset=%TOOLSET% --user-config=./user-config-android-armeabi-v7a.jam cxxflags=%CXXFLAGS% threadapi=pthread --stagedir=./lib_android_armeabi-v7a --builddir=./ target-os=linux --without-atomic --without-date_time --without-program_options --without-regex --disable-filesystem2 link=static runtime-link=static threading=multi stage

set CXXFLAGS="-I%ANDROID_NDK%/platforms/android-9/arch-x86/usr/include -I%ANDROID_NDK%/sources/cxx-stl/gnu-libstdc++/4.6/include -I%ANDROID_NDK%/sources/cxx-stl/gnu-libstdc++/4.6/libs/x86/include"
set TOOLSET=gcc-android_x86
bjam --toolset=%TOOLSET% --user-config=./user-config-android-x86.jam cxxflags=%CXXFLAGS% threadapi=pthread --stagedir=./lib_android_x86 --builddir=./ target-os=linux --without-atomic --without-date_time --without-program_options --without-regex --disable-filesystem2 link=static runtime-link=static threading=multi stage
