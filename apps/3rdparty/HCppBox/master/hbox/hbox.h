/***************************************************************
 * Name:      hbox.h
 * Purpose:   hbox对外接口
 * Author:    HYH (hyhsystem.cn)
 * Created:   2024-01-18
 * Copyright: HYH (hyhsystem.cn)
 * License:   MIT
 **************************************************************/
#ifndef HBOX_H_INCLUDED
#define HBOX_H_INCLUDED


/*
由C语言编写的组件
*/
#include "hcompiler.h"
#include "hdefaults.h"
#include "h3rdparty.h" //第三方库
#include "heventloop.h"
#include "heventslots.h"
#include "heventchain.h"
#include "hwatchdog.h"
#include "hmemoryheap.h"
#include "hobject.h"
#include "hringbuf.h"
#include "hunicode.h"
#include "hstacklesscoroutine.h"
#include "hmodbus.h"
#include "hbase64.h"
#include "hsimulator.h"
#include "hgui.h"

/*
由C++编写的组件
*/
#include "hcpprt.h"
#include "hchain.h"
#include "hloop.h"
#include "hslots.h"
#include "hfifo.h"
#include "hsoftdog.h"

#endif // HBOX_H_INCLUDED
