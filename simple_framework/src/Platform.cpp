/*
 * This confidential and proprietary software may be used only as
 * authorised by a licensing agreement from ARM Limited
 * (C) COPYRIGHT 2012 ARM Limited
 * ALL RIGHTS RESERVED
 * The entire notice above must be reproduced on all authorised
 * copies and copies may only be made to the extent permitted
 * by a licensing agreement from ARM Limited.
 */

#include "Platform.h"

#include <cstdio>
#include <cstdarg>

namespace MaliSDK
{
    Platform* Platform::getInstance(void)
    {
    #if defined(_WIN32)
        return WindowsPlatform::getInstance();
    #elif defined(__arm__) && defined(__linux__)
        //return LinuxOnARMPlatform::getInstance();
        return HWCPlatform::getInstance();
    #elif defined(__linux__)
        return DesktopLinuxPlatform::getInstance();
    #endif
    }

    void Platform::log(const char* format, ...)
    {
        va_list ap;
        va_start (ap, format);
        vfprintf (stdout, format, ap);
        fprintf (stdout, "\n");
        va_end (ap);
        fflush(stdout);
    }
}
