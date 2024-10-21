/***************************************************************
 * Name:      HVCP.cpp
 * Purpose:   HVCP库实现
 * Author:    HYH (hyhsystem.cn)
 * Created:   2024-10-20
 * Copyright: HYH (hyhsystem.cn)
 * License:   MIT
 **************************************************************/
#include "HVCP.h"
#include "stdio.h"
#include "stdlib.h"
#include "windows.h"
#include <setupapi.h>
#include <cfgmgr32.h>

void HVCP_Enum(void(*OnEnum)(const char* device,void *usr),void *usr)
{
    if (OnEnum == NULL)
    {
        return;
    }
    SP_DEVINFO_DATA device_info_data = { 0 };
    device_info_data.cbSize = sizeof(device_info_data);
    HDEVINFO    device_info = SetupDiGetClassDevs(NULL, 0, 0,DIGCF_PRESENT | DIGCF_ALLCLASSES);
    if (device_info == INVALID_HANDLE_VALUE)
    {
        return;
    }

    for (int i = 0; SetupDiEnumDeviceInfo(device_info, i, &device_info_data); i++)
    {
        {
            //查找硬件Id
            DEVINST dev_inst=device_info_data.DevInst;
            char hwid[256]= {0};
            ULONG hwid_len=sizeof(hwid);
            if(CM_Get_DevNode_Registry_PropertyA(dev_inst,CM_DRP_HARDWAREID,NULL,(PVOID)hwid,(PULONG)&hwid_len,0)!=CR_SUCCESS)
            {
                continue;
            }
            hwid[sizeof(hwid)-1]='\0';
            //判断是不HVCP设备
            if(strcmp(hwid,"UMDF\\HVCPDriver")!=0)
            {
                continue;
            }
        }

        {
            //查找串口号
            char value[8];
            DWORD type=REG_NONE;
            DWORD size = sizeof(value);
            HKEY device_key = SetupDiOpenDevRegKey(device_info, &device_info_data,DICS_FLAG_GLOBAL, 0,DIREG_DEV, KEY_QUERY_VALUE);
            if (device_key == INVALID_HANDLE_VALUE)
            {
                continue;
            }
            if (RegQueryValueExA(device_key, "PortName", NULL, &type, (LPBYTE)value,&size) != ERROR_SUCCESS || type != REG_SZ)
            {
                RegCloseKey(device_key);
                continue;
            }
            RegCloseKey(device_key);
            value[sizeof(value) - 1] = '\0';
            OnEnum(value,usr);
        }
    }
    SetupDiDestroyDeviceInfoList(device_info);
}



