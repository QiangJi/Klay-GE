#if defined(__ANDROID__)
#include "pyconfig.android.h"
#elif defined(_WIN32) || defined(__WIN32__) || defined(WIN32)
#include "pyconfig.win.h"
#else
#include "../pyconfig.h"
#endif
