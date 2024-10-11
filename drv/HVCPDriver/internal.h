#pragma once

#include <windows.h>
#include <devioctl.h>
#include <ntddser.h>
#include <setupapi.h>
#include <strsafe.h>
#include <intsafe.h>


#include "wudfddi.h"


#include "specstrings.h"


typedef class CMyDriver    *PCMyDriver;
typedef class CMyDevice    *PCMyDevice;
typedef class CMyQueue     *PCMyQueue;
typedef class CRingBuffer  *PCMyRingBuffer;


#ifndef ARRAY_SIZE
#define ARRAY_SIZE(x) (sizeof(x) / sizeof(x[0]))
#endif

#define SAFE_RELEASE(p)     {if ((p)) { (p)->Release(); (p) = NULL; }}


#define WPP_CONTROL_GUIDS                                                   \
    WPP_DEFINE_CONTROL_GUID(                                                \
        MyDriverTraceControl, (C31877A2,C8C8,4c58,B5B8,7D6311D5E343),       \
                                                                            \
        WPP_DEFINE_BIT(MYDRIVER_ALL_INFO)                                   \
        )

#define WPP_FLAG_LEVEL_LOGGER(flag, level)                                  \
    WPP_LEVEL_LOGGER(flag)

#define WPP_FLAG_LEVEL_ENABLED(flag, level)                                 \
    (WPP_LEVEL_ENABLED(flag) &&                                             \
     WPP_CONTROL(WPP_BIT_ ## flag).Level >= level)


//
// This comment block is scanned by the trace preprocessor to define our
// Trace function.
//
// begin_wpp config
// FUNC Trace{FLAG=MYDRIVER_ALL_INFO}(LEVEL, MSG, ...);
// end_wpp
//


#define MYDRIVER_TRACING_ID L"HYH\\HVCPDriver"
#define MYDRIVER_CLASS_ID   { 0xc8ecc087, 0x6b79, 0x4de5, { 0x8f, 0xb4, 0xc0, 0x33, 0x58, 0xa2, 0x96, 0x17 } }

__forceinline
#ifdef _PREFAST_
__declspec(noreturn)
#endif
VOID WdfTestNoReturn(VOID)
{

}

#define WUDF_TEST_DRIVER_ASSERT(p)  \
{                                   \
    if ( !(p) )                     \
    {                               \
        DebugBreak();               \
        WdfTestNoReturn();          \
    }                               \
}

#define MAXULONG    0xffffffff





#include "comsup.h"
#include "driver.h"
#include "device.h"
#include "ringbuffer.h"
#include "queue.h"
#include "serial.h"

