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

static HMODULE newdev_dll=NULL;
typedef BOOL WINAPI (*DiInstallDeviceProto)(HWND hwndParent,HDEVINFO DeviceInfoSet,PSP_DEVINFO_DATA DeviceInfoData,PSP_DRVINFO_DATA DriverInfoData,DWORD Flags,PBOOL NeedReboot);
static DiInstallDeviceProto DiInstallDeviceFn=NULL;


BOOL WINAPI DllMain(HANDLE hInstance, ULONG Command, LPVOID Reserved)
{
    switch(Command)
    {
    case DLL_PROCESS_ATTACH:
    {
        newdev_dll=LoadLibraryA("newdev.dll");
        if(newdev_dll!=NULL)
        {
            DiInstallDeviceFn=(DiInstallDeviceProto)GetProcAddress(newdev_dll,"DiInstallDevice");
        }
    }
    break;
    case DLL_PROCESS_DETACH:
    {
        if(newdev_dll!=NULL)
        {
            FreeLibrary(newdev_dll);
        }
    }
    break;
    }
    return TRUE;
}

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

typedef struct
{
    const char *com;
    bool IsHVCPPort;
} HVCP_IsHVCPPort_Para_t;
static bool HVCP_IsHVCPPort(const char * device)
{
    bool ret=false;
    if(device!=NULL)
    {
        const char * com=NULL;
        if((com=strstr(device,"COM"))!=NULL)
        {
            HVCP_IsHVCPPort_Para_t para;
            para.IsHVCPPort=ret;
            para.com=com;
            HVCP_Enum([](const char* device,void *usr)
            {
                if(usr==NULL)
                {
                    return;
                }
                HVCP_IsHVCPPort_Para_t &para=*(HVCP_IsHVCPPort_Para_t *)usr;
                if(strcmp(device,para.com)==0)
                {
                    para.IsHVCPPort=true;
                }
            },&para);
            ret=para.IsHVCPPort;
        }
    }
    return ret;
}

int HVCP_Exists(const char *device)
{
    return HVCP_IsHVCPPort(device)?0:-1;
}

size_t HVCP_Write(const char *device,const void *buffer,size_t length)
{
    size_t ret=0;
    if(!HVCP_IsHVCPPort(device))
    {
        return ret;
    }
    if(strlen(device) > 8)
    {
        return ret;
    }
    char com_path[32]="\\\\.\\";
    if(device[0]=='\\')
    {
        strcpy(com_path,device);
    }
    else
    {
        strcat(com_path,device);
    }
    HANDLE handle=CreateFileA(com_path,GENERIC_READ|GENERIC_WRITE,FILE_SHARE_READ|FILE_SHARE_WRITE,NULL,OPEN_EXISTING,FILE_ATTRIBUTE_NORMAL,NULL);
    if(handle==INVALID_HANDLE_VALUE)
    {
        return ret;
    }
    {
        DWORD len=0;
        WriteFile(handle,buffer,length,&len,NULL);
        ret=len;
    }
    CloseHandle(handle);
    return ret;
}


size_t HVCP_Read(const char *device,void *buffer,size_t length)
{
    size_t ret=0;
    if(!HVCP_IsHVCPPort(device))
    {
        return ret;
    }
    if(strlen(device) > 8)
    {
        return ret;
    }
    char com_path[32]="\\\\.\\";
    if(device[0]=='\\')
    {
        strcpy(com_path,device);
    }
    else
    {
        strcat(com_path,device);
    }
    HANDLE handle=CreateFileA(com_path,GENERIC_READ|GENERIC_WRITE,FILE_SHARE_READ|FILE_SHARE_WRITE,NULL,OPEN_EXISTING,FILE_ATTRIBUTE_NORMAL,NULL);
    if(handle==INVALID_HANDLE_VALUE)
    {
        return ret;
    }
    {
        DWORD len=0;
        ReadFile(handle,buffer,length,&len,NULL);
        ret=len;
    }
    CloseHandle(handle);
    return ret;
}

int HVCP_DriverRemove(const char *device)
{
    if(device==NULL)
    {
        return -1;
    }
    const char * com=NULL;
    if((com=strstr(device,"COM"))==NULL)
    {
        return -1;
    }
    int ret=-1;
    SP_DEVINFO_DATA device_info_data = { 0 };
    device_info_data.cbSize = sizeof(device_info_data);
    HDEVINFO    device_info = SetupDiGetClassDevs(NULL, 0, 0,DIGCF_PRESENT | DIGCF_ALLCLASSES);
    if (device_info == INVALID_HANDLE_VALUE)
    {
        return ret;
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
            if(strcmp(value,com)==0)
            {
                //已找到设备,注意:当设备被打开时,可能会提示重启
                if(FALSE!=SetupDiRemoveDevice(device_info,&device_info_data))
                {
                    ret=0;
                }
                break;
            }
        }
    }
    SetupDiDestroyDeviceInfoList(device_info);
    return ret;
}

int HVCP_DriverAdd(void)
{
    //inf中的类信息
    GUID ClassGUID= {0};
    CLSIDFromString(L"{4D36E978-E325-11CE-BFC1-08002BE10318}",&ClassGUID);
    const char *ClassName="Ports";
    const char *HWID="UMDF\\HVCPDriver";
    int ret=-1;
    SP_DEVINFO_DATA DeviceInfoData= {0};
    DeviceInfoData.cbSize = sizeof(SP_DEVINFO_DATA);
    HDEVINFO DeviceInfoSet = SetupDiCreateDeviceInfoList(&ClassGUID,0);
    if(DeviceInfoSet == INVALID_HANDLE_VALUE)
    {
        goto final_clean;
    }
    if (!SetupDiCreateDeviceInfoA(DeviceInfoSet,ClassName,&ClassGUID,NULL,0,DICD_GENERATE_ID,&DeviceInfoData))
    {
        goto final_clean;
    }

    if(!SetupDiSetDeviceRegistryPropertyA(DeviceInfoSet,&DeviceInfoData,SPDRP_HARDWAREID,(LPBYTE)HWID,(strlen(HWID)+1)))
    {
        goto final_clean;
    }
    if (!SetupDiCallClassInstaller(DIF_REGISTERDEVICE,DeviceInfoSet,&DeviceInfoData))
    {
        goto final_clean;
    }
    if(DiInstallDeviceFn==NULL)
    {
        //安装驱动，此步骤要求系统中已存在HVCP驱动（即用户已安装成功HVCP驱动,即驱动包已添加到系统,64位系统需要禁用驱动强制签名安装,此操作无需禁用驱动强制签名）
        if(!SetupDiInstallDevice(DeviceInfoSet,&DeviceInfoData))
        {
            goto final_clean;
        }
    }
    else
    {
        if(!DiInstallDeviceFn(NULL,DeviceInfoSet,&DeviceInfoData,NULL,0,NULL))
        {
            goto final_clean;
        }
    }
    ret=0;
final_clean:
    if (DeviceInfoSet != INVALID_HANDLE_VALUE)
    {
        SetupDiDestroyDeviceInfoList(DeviceInfoSet);
    }
    return ret;
}
