#include "service.h"
#include "windows.h"

bool ServiceIsInstalled(const char *service_name)
{
    bool ret=false;
    if(service_name==NULL || service_name[0]=='\0')
    {
        return ret;
    }
    SC_HANDLE hSCM=OpenSCManager(NULL,NULL,SC_MANAGER_ALL_ACCESS);
    if(hSCM!=NULL)
    {
        SC_HANDLE hService=OpenServiceA(hSCM,service_name,SERVICE_QUERY_CONFIG);
        if(hService!=NULL)
        {
            ret=true;
            CloseServiceHandle(hService);
        }
        CloseServiceHandle(hSCM);
    }
    return ret;
}

bool ServiceInstall(const char *service_name,const char *display_name,const char *cmd)
{
    bool ret=false;
    if(service_name==NULL || service_name[0]=='\0')
    {
        return ret;
    }
    if(display_name==NULL || display_name[0]=='\0')
    {
        return ret;
    }
    if(cmd==NULL || cmd[0]=='\0')
    {
        return ret;
    }
    if(ServiceIsInstalled(service_name))
    {
        return true;
    }
    SC_HANDLE hSCM=OpenSCManager(NULL,NULL,SC_MANAGER_ALL_ACCESS);
    if(hSCM!=NULL)
    {
        SC_HANDLE hService=CreateServiceA(hSCM,service_name,display_name,SERVICE_ALL_ACCESS,SERVICE_WIN32_OWN_PROCESS|SERVICE_INTERACTIVE_PROCESS,SERVICE_AUTO_START,SERVICE_ERROR_NORMAL,cmd,NULL,NULL,NULL,NULL,NULL);
        if(hService!=NULL)
        {
            ret=true;
            CloseServiceHandle(hService);
        }
        CloseServiceHandle(hSCM);
    }
    return ret;
}

bool ServiceUnistall(const char *service_name)
{
    bool ret=false;
    if(service_name==NULL || service_name[0]=='\0')
    {
        return ret;
    }
    if(!ServiceIsInstalled(service_name))
    {
        return true;
    }
    SC_HANDLE hSCM=OpenSCManager(NULL,NULL,SC_MANAGER_ALL_ACCESS);
    if(hSCM!=NULL)
    {
        SC_HANDLE hService=OpenServiceA(hSCM,service_name,SERVICE_ALL_ACCESS);
        if(hService!=NULL)
        {
            SERVICE_STATUS status= {0};
            ControlService(hService,SERVICE_CONTROL_STOP,&status);
            Sleep(1000);
            if(DeleteService(hService))
            {
                ret=true;
            }
            CloseServiceHandle(hService);
        }
        CloseServiceHandle(hSCM);
    }
    return ret;
}

bool ServiceIsStarted(const char *service_name)
{
    bool ret=false;
    if(service_name==NULL || service_name[0]=='\0')
    {
        return ret;
    }
    SC_HANDLE hSCM=OpenSCManager(NULL,NULL,SC_MANAGER_ALL_ACCESS);
    if(hSCM!=NULL)
    {
        SC_HANDLE hService=OpenServiceA(hSCM,service_name,SERVICE_QUERY_CONFIG|SERVICE_QUERY_STATUS);
        if(hService!=NULL)
        {
            SERVICE_STATUS status= {0};
            if(QueryServiceStatus(hService,&status))
            {
                if(status.dwCurrentState==SERVICE_RUNNING)
                {
                    ret=true;
                }
            }
            CloseServiceHandle(hService);
        }
        CloseServiceHandle(hSCM);
    }
    return ret;
}

bool ServiceStart(const char *service_name)
{
    bool ret=false;
    if(service_name==NULL || service_name[0]=='\0')
    {
        return ret;
    }
    if(ServiceIsStarted(service_name))
    {
        return true;
    }
    SC_HANDLE hSCM=OpenSCManager(NULL,NULL,SC_MANAGER_ALL_ACCESS);
    if(hSCM!=NULL)
    {
        SC_HANDLE hService=OpenServiceA(hSCM,service_name,SERVICE_ALL_ACCESS);
        if(hService!=NULL)
        {
            if(StartServiceA(hService,0,NULL))
            {
                ret=true;
            }
            CloseServiceHandle(hService);
        }
        CloseServiceHandle(hSCM);
    }
    return ret;
}

