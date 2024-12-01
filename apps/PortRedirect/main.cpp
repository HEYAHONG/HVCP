
#include "stdlib.h"
#include "stdio.h"
#include "stdarg.h"
#include "string.h"
#include "HVCP.h"
#include "HCPPBox.h"
#include "hbox.h"
#include <thread>
#include <libserialport.h>
#include <windows.h>
#include "service.h"
#include "serialport.h"
#include <string>
#include <vector>

bool IsServiceMode=false;
static int main_log(const char *fmt,...)
{
    int ret=0;
    va_list va;
    va_start(va,fmt);
    if(!IsServiceMode)
    {
        ret=vprintf(fmt,va);
    }
    else
    {
        CHAR Msg[4096];
        Msg[(sizeof(Msg)/sizeof(Msg[0]))-1]='\0';
        vsnprintf(Msg,(sizeof(Msg)/sizeof(Msg[0]))-1,fmt,va);
        HANDLE hEventSource=RegisterEventSourceA(NULL,"HVCP_PortRedirect");
        if(hEventSource!=NULL)
        {
            LPTSTR lpszString[1]= {Msg};
            ReportEventA(hEventSource,EVENTLOG_INFORMATION_TYPE,0,0,NULL,1,0,(LPCSTR*)&lpszString[0],NULL);
            DeregisterEventSource(hEventSource);
        }
    }
    va_end(va);

    return ret;
}

static int cmd_help(int argc,const char *argv[]);
static int cmd_input_baudrate(int argc,const char *argv[]);
static int cmd_input_databits(int argc,const char *argv[]);
static int cmd_input_parity(int argc,const char *argv[]);
static int cmd_input_stopbits(int argc,const char *argv[]);
static int cmd_input(int argc,const char *argv[]);
static int cmd_output(int argc,const char *argv[]);
static int cmd_output_baudrate(int argc,const char *argv[]);
static int cmd_output_databits(int argc,const char *argv[]);
static int cmd_output_parity(int argc,const char *argv[]);
static int cmd_output_stopbits(int argc,const char *argv[]);
static int cmd_uninstall(int argc,const char *argv[]);
static struct
{
    const char * cmd;
    const char * cmd_short;
    int (*cmd_entry)(int argc,const char *argv[]);
    const char * usage;
    const char * help;
} cmd_list[]=
{
    {
        "--help",
        "-h",
        cmd_help,
        "--help  / -h ",
        "get  help"
    },
    {
        "--input",
        "-i",
        cmd_input,
        "--input=[device]  / -i [device] ",
        "input device"
    },
    {
        "--input-baudrate",
        "-ib",
        cmd_input_baudrate,
        "--input-baudrate=[baudrate]  / -ib [baudrate] ",
        "input baudrate"
    },
    {
        "--input-databits",
        "-id",
        cmd_input_databits,
        "--input-databits=[databits]  / -id [databits] ",
        "input databits"
    },
    {
        "--input-parity",
        "-ip",
        cmd_input_parity,
        "--input-parity=[ N|E|O|M|S]  / -ip [ N|E|O|M|S] ",
        "input parity"
    },
    {
        "--input-stopbits",
        "-is",
        cmd_input_stopbits,
        "--input-stopbits=[stopbits]  / -is [stopbits] ",
        "input stopbits"
    },
    {
        "--output",
        "-o",
        cmd_output,
        "--output=[device]  / -o [device] ",
        "output device"
    },
    {
        "--output-baudrate",
        "-ob",
        cmd_output_baudrate,
        "--output-baudrate=[baudrate]  / -ob [baudrate] ",
        "output baudrate"
    }
    ,
    {
        "--output-databits",
        "-od",
        cmd_output_databits,
        "--output-databits=[databits]  / -od [databits] ",
        "output databits"
    }
    ,
    {
        "--output-parity",
        "-op",
        cmd_output_parity,
        "--output-parity=[ N|E|O|M|S]  / -op [ N|E|O|M|S] ",
        "output parity"
    }
    ,
    {
        "--output-stopbits",
        "-os",
        cmd_output_stopbits,
        "--output-stopbits=[stopbits]  / -os [stopbits] ",
        "output stopbits"
    }
    ,
    {
        "--uninstall",
        "-ui",
        cmd_uninstall,
        "--uninstall  / -ui ",
        "uninstall"
    }
};

static int cmd_help(int argc,const char *argv[])
{
    const size_t cmd_max_len=16;
    const size_t usage_max_len=50;
    {
        main_log("\r\n%s [options]\r\n",argv[0]);
    }
    {
        //打印标题栏
        for(size_t i=0; i<cmd_max_len; i++)
        {
            main_log("%s","-");
        }
        main_log("\t");
        for(size_t i=0; i<usage_max_len; i++)
        {
            main_log("%s","-");
        }
        main_log("\r\n");
    }
    {
        for(size_t i=0; i<sizeof(cmd_list)/sizeof(cmd_list[0]); i++)
        {
            {
                //打印cmd
                main_log("%s",cmd_list[i].cmd);
                int cmd_pad_len=(int)cmd_max_len-strlen(cmd_list[i].cmd);
                if(cmd_pad_len > 0)
                {
                    for(size_t i=0; i<cmd_pad_len; i++)
                    {
                        main_log(" ");
                    }
                }
            }
            main_log("\t");
            {
                //打印usage
                main_log("%s",cmd_list[i].usage);
                int usage_pad_len=(int)usage_max_len-strlen(cmd_list[i].usage);
                if(usage_pad_len > 0)
                {
                    for(size_t i=0; i<usage_pad_len; i++)
                    {
                        main_log(" ");
                    }
                }
            }
            main_log("\t");
            {
                main_log("%s",cmd_list[i].help);
            }
            main_log("\r\n");
        }
    }
    //退出程序
    exit(0);
    return 0;
}


static void arg_parse(int argc,const char *argv[])
{
    if(argc==1)
    {
        cmd_help(argc,argv);
    }
    else
    {
        for(int i=0; i<argc; i++)
        {
            const char *cmd=argv[i];
            for(size_t j=0; j<sizeof(cmd_list)/sizeof(cmd_list[0]); j++)
            {
                if(cmd_list[j].cmd_short==NULL)
                {
                    cmd_list[j].cmd_short="";
                }
                if(cmd_list[j].cmd==NULL)
                {
                    cmd_list[j].cmd="";
                }
                if(strcmp(cmd,cmd_list[j].cmd_short)==0)
                {
                    if(cmd_list[j].cmd_entry!=NULL)
                    {
                        cmd_list[j].cmd_entry(argc,argv);
                    }
                }
                {
                    char temp[256]= {0};
                    strcat_s(temp,sizeof(temp)-1,cmd);
                    for(size_t k=0; k<strlen(temp); k++)
                    {
                        if(temp[k]=='=')
                        {
                            temp[k]='\0';
                        }
                    }
                    if(strcmp(temp,cmd_list[j].cmd)==0)
                    {
                        if(cmd_list[j].cmd_entry!=NULL)
                        {
                            cmd_list[j].cmd_entry(argc,argv);
                        }
                    }
                }
            }
        }
    }
}


static void com_name_strip(char *name)
{
    if(name!=NULL)
    {
        strupr(name);
        const char *com_name=strstr(name,"COM");
        if(com_name!=NULL)
        {
            if(name != com_name)
            {
                size_t len=strlen(name);
                name[0]='\0';
                strcat_s(name,len,com_name);
            }
        }
        else
        {
            //不是串口设备(COMx)
            name[0]='\0';
        }
    }
}
static bool check_com_port(const char *portname,int baudrate=115200,int databits=8,char parity='N',int stopbits=1)
{
    if(portname==NULL || portname[0]=='\0')
    {
        return false;
    }
    if(HVCP_Exists(portname)==0)
    {
        //HVCP串口只检查是否存在，不检查其它参数
        return true;
    }
    struct sp_port **port_list=NULL;
    sp_list_ports(&port_list);
    if(port_list==NULL)
    {
        return false;
    }
    bool ret=false;
    for(size_t i=0; port_list[i]!=NULL; i++)
    {
        struct sp_port *port=port_list[i];
        if(strcmp(strstr(portname,"COM"),strstr(sp_get_port_name(port),"COM"))==0)
        {
            ret=true;
            if(sp_open(port,SP_MODE_READ_WRITE)!=SP_OK)
            {
                ret=false;
            }
            else
            {
                if(SP_OK!=sp_set_baudrate(port,baudrate))
                {
                    ret=false;
                }
                if(SP_OK!=sp_set_bits(port,databits))
                {
                    ret=false;
                }
                {
                    sp_parity _parity=SP_PARITY_NONE;
                    switch(parity)
                    {
                    case 'N':
                    {
                        _parity=SP_PARITY_NONE;
                    }
                    break;
                    case 'O':
                    {
                        _parity=SP_PARITY_ODD;
                    }
                    break;
                    case 'E':
                    {
                        _parity=SP_PARITY_EVEN;
                    }
                    break;
                    case 'M':
                    {
                        _parity=SP_PARITY_MARK;
                    }
                    break;
                    case 'S':
                    {
                        _parity=SP_PARITY_SPACE;
                    }
                    break;
                    default:
                        break;
                    }
                    if(SP_OK!=sp_set_parity(port,_parity))
                    {
                        ret=false;
                    }
                }
                if(SP_OK!=sp_set_stopbits(port,stopbits))
                {
                    ret=false;
                }
                sp_close(port);
            }
            break;
        }
    }
    sp_free_port_list(port_list);
    return ret;
}
static char input_device[32]= {0};
static int cmd_input(int argc,const char *argv[])
{
    for(int i=0; i<argc; i++)
    {
        {
            char temp[256]= {0};
            const char *para=NULL;
            strcat_s(temp,sizeof(temp)-1,argv[i]);
            for(size_t k=0; k<strlen(temp); k++)
            {
                if(temp[k]=='=')
                {
                    temp[k]='\0';
                    para=&temp[k+1];
                    break;
                }
            }
            if(strcmp("--input",temp)==0)
            {
                if(para!=NULL)
                {
                    strcat_s(input_device,sizeof(input_device)-1,para);
                    break;
                }
            }
            if(strcmp("-i",argv[i])==0)
            {
                if((i+1)<argc)
                {
                    strcat_s(input_device,sizeof(input_device)-1,argv[i+1]);
                    break;
                }
            }
        }

    }
    com_name_strip(input_device);
    return 0;
}

static int input_baudrate=115200;
static int cmd_input_baudrate(int argc,const char *argv[])
{
    for(int i=0; i<argc; i++)
    {
        {
            char temp[256]= {0};
            const char *para=NULL;
            strcat_s(temp,sizeof(temp)-1,argv[i]);
            for(size_t k=0; k<strlen(temp); k++)
            {
                if(temp[k]=='=')
                {
                    temp[k]='\0';
                    para=&temp[k+1];
                    break;
                }
            }
            if(strcmp("--input-baudrate",temp)==0)
            {
                if(para!=NULL)
                {
                    input_baudrate=atoi(para);
                    break;
                }
            }
            if(strcmp("-ib",argv[i])==0)
            {
                if((i+1)<argc)
                {
                    input_baudrate=atoi(argv[i+1]);
                    break;
                }
            }
        }

    }
    if(input_baudrate<=0)
    {
        input_baudrate=115200;
    }
    return 0;
}

static int input_databits=8;
static int cmd_input_databits(int argc,const char *argv[])
{
    for(int i=0; i<argc; i++)
    {
        {
            char temp[256]= {0};
            const char *para=NULL;
            strcat_s(temp,sizeof(temp)-1,argv[i]);
            for(size_t k=0; k<strlen(temp); k++)
            {
                if(temp[k]=='=')
                {
                    temp[k]='\0';
                    para=&temp[k+1];
                    break;
                }
            }
            if(strcmp("--input-databits",temp)==0)
            {
                if(para!=NULL)
                {
                    input_databits=atoi(para);
                    break;
                }
            }
            if(strcmp("-id",argv[i])==0)
            {
                if((i+1)<argc)
                {
                    input_databits=atoi(argv[i+1]);
                    break;
                }
            }
        }

    }
    if(input_databits<=5 || input_databits > 8)
    {
        input_databits=8;
    }
    return 0;
}

static char input_parity='N';
static int cmd_input_parity(int argc,const char *argv[])
{
    for(int i=0; i<argc; i++)
    {
        {
            char temp[256]= {0};
            const char *para=NULL;
            strcat_s(temp,sizeof(temp)-1,argv[i]);
            for(size_t k=0; k<strlen(temp); k++)
            {
                if(temp[k]=='=')
                {
                    temp[k]='\0';
                    para=&temp[k+1];
                    break;
                }
            }
            if(strcmp("--input-parity",temp)==0)
            {
                if(para!=NULL)
                {
                    input_parity=para[0];
                    break;
                }
            }
            if(strcmp("-ip",argv[i])==0)
            {
                if((i+1)<argc)
                {
                    input_parity=argv[i+1][0];
                    break;
                }
            }
        }

    }
    switch(input_parity)
    {
    case 'N':
    case 'O':
    case 'E':
    case 'M':
    case 'S':
        break;
    default:
    {
        input_parity='N';
    }
    break;
    }
    return 0;
}

static int input_stopbits=1;
static int cmd_input_stopbits(int argc,const char *argv[])
{
    for(int i=0; i<argc; i++)
    {
        {
            char temp[256]= {0};
            const char *para=NULL;
            strcat_s(temp,sizeof(temp)-1,argv[i]);
            for(size_t k=0; k<strlen(temp); k++)
            {
                if(temp[k]=='=')
                {
                    temp[k]='\0';
                    para=&temp[k+1];
                    break;
                }
            }
            if(strcmp("--input-stopbits",temp)==0)
            {
                if(para!=NULL)
                {
                    input_stopbits=atoi(para);
                    break;
                }
            }
            if(strcmp("-is",argv[i])==0)
            {
                if((i+1)<argc)
                {
                    input_stopbits=atoi(argv[i+1]);
                    break;
                }
            }
        }

    }
    if(input_stopbits<=0 || input_stopbits > 2)
    {
        input_stopbits=1;
    }
    return 0;
}

static int output_baudrate=115200;
static int cmd_output_baudrate(int argc,const char *argv[])
{
    for(int i=0; i<argc; i++)
    {
        {
            char temp[256]= {0};
            const char *para=NULL;
            strcat_s(temp,sizeof(temp)-1,argv[i]);
            for(size_t k=0; k<strlen(temp); k++)
            {
                if(temp[k]=='=')
                {
                    temp[k]='\0';
                    para=&temp[k+1];
                    break;
                }
            }
            if(strcmp("--output-baudrate",temp)==0)
            {
                if(para!=NULL)
                {
                    output_baudrate=atoi(para);
                    break;
                }
            }
            if(strcmp("-ob",argv[i])==0)
            {
                if((i+1)<argc)
                {
                    output_baudrate=atoi(argv[i+1]);
                    break;
                }
            }
        }

    }
    if(output_baudrate<=0)
    {
        output_baudrate=115200;
    }
    return 0;
}

static int output_databits=8;
static int cmd_output_databits(int argc,const char *argv[])
{
    for(int i=0; i<argc; i++)
    {
        {
            char temp[256]= {0};
            const char *para=NULL;
            strcat_s(temp,sizeof(temp)-1,argv[i]);
            for(size_t k=0; k<strlen(temp); k++)
            {
                if(temp[k]=='=')
                {
                    temp[k]='\0';
                    para=&temp[k+1];
                    break;
                }
            }
            if(strcmp("--output-databits",temp)==0)
            {
                if(para!=NULL)
                {
                    output_databits=atoi(para);
                    break;
                }
            }
            if(strcmp("-od",argv[i])==0)
            {
                if((i+1)<argc)
                {
                    output_databits=atoi(argv[i+1]);
                    break;
                }
            }
        }

    }
    if(output_databits<=5 || output_databits > 8)
    {
        output_databits=8;
    }
    return 0;
}

static char output_parity='N';
static int cmd_output_parity(int argc,const char *argv[])
{
    for(int i=0; i<argc; i++)
    {
        {
            char temp[256]= {0};
            const char *para=NULL;
            strcat_s(temp,sizeof(temp)-1,argv[i]);
            for(size_t k=0; k<strlen(temp); k++)
            {
                if(temp[k]=='=')
                {
                    temp[k]='\0';
                    para=&temp[k+1];
                    break;
                }
            }
            if(strcmp("--output-parity",temp)==0)
            {
                if(para!=NULL)
                {
                    output_parity=para[0];
                    break;
                }
            }
            if(strcmp("-op",argv[i])==0)
            {
                if((i+1)<argc)
                {
                    output_parity=argv[i+1][0];
                    break;
                }
            }
        }

    }
    switch(output_parity)
    {
    case 'N':
    case 'O':
    case 'E':
    case 'M':
    case 'S':
        break;
    default:
    {
        output_parity='N';
    }
    break;
    }
    return 0;
}

static int output_stopbits=1;
static int cmd_output_stopbits(int argc,const char *argv[])
{
    for(int i=0; i<argc; i++)
    {
        {
            char temp[256]= {0};
            const char *para=NULL;
            strcat_s(temp,sizeof(temp)-1,argv[i]);
            for(size_t k=0; k<strlen(temp); k++)
            {
                if(temp[k]=='=')
                {
                    temp[k]='\0';
                    para=&temp[k+1];
                    break;
                }
            }
            if(strcmp("--output-stopbits",temp)==0)
            {
                if(para!=NULL)
                {
                    output_stopbits=atoi(para);
                    break;
                }
            }
            if(strcmp("-os",argv[i])==0)
            {
                if((i+1)<argc)
                {
                    output_stopbits=atoi(argv[i+1]);
                    break;
                }
            }
        }

    }
    if(output_stopbits<=0 || output_stopbits > 2)
    {
        output_stopbits=1;
    }
    return 0;
}

static char output_device[32]= {0};
static int cmd_output(int argc,const char *argv[])
{
    for(int i=0; i<argc; i++)
    {
        {
            char temp[256]= {0};
            const char *para=NULL;
            strcat_s(temp,sizeof(temp)-1,argv[i]);
            for(size_t k=0; k<strlen(temp); k++)
            {
                if(temp[k]=='=')
                {
                    temp[k]='\0';
                    para=&temp[k+1];
                    break;
                }
            }
            if(strcmp("--output",temp)==0)
            {
                if(para!=NULL)
                {
                    strcat_s(output_device,sizeof(output_device)-1,para);
                    break;
                }
            }
            if(strcmp("-o",argv[i])==0)
            {
                if((i+1)<argc)
                {
                    strcat_s(output_device,sizeof(output_device)-1,argv[i+1]);
                    break;
                }
            }
        }

    }
    com_name_strip(output_device);
    return 0;
}

static bool uninstall_mode=false;
static int cmd_uninstall(int argc,const char *argv[])
{
    for(int i=0; i<argc; i++)
    {
        {
            char temp[256]= {0};
            const char *para=NULL;
            strcat_s(temp,sizeof(temp)-1,argv[i]);
            for(size_t k=0; k<strlen(temp); k++)
            {
                if(temp[k]=='=')
                {
                    temp[k]='\0';
                    para=&temp[k+1];
                    break;
                }
            }
            if(strcmp("--uninstall",temp)==0)
            {
                {
                    uninstall_mode=true;
                    break;
                }
            }
            if(strcmp("-ui",argv[i])==0)
            {
                {
                    uninstall_mode=true;
                    break;
                }
            }
        }

    }
    return 0;
}

//配置id,用于区分不同的服务
static char config_id[4096]= {0};
static const char *get_config_id()
{
    if(strcmp(input_device,output_device) < 0)
    {
        snprintf(config_id,sizeof(config_id)-1,"%s_To_%s",input_device,output_device);
    }
    else
    {
        snprintf(config_id,sizeof(config_id)-1,"%s_To_%s",output_device,input_device);
    }
    return config_id;
}

static void check_service_mode()
{
    char username[256]= {0};
    DWORD len=sizeof(username)-1;
    GetUserNameA(username,&len);
    if(strcmp(username,"SYSTEM")==0)
    {
        //SYSTEM,服务模式
        IsServiceMode=true;
    }
    main_log("User is %s,config_id is %s\r\n",username,get_config_id());
}

static int check_com_settings()
{
    if(strlen(input_device)>0)
    {
        main_log("input_device is %s!\r\n",input_device);
        if(HVCP_Exists(input_device)==0)
        {
            main_log("%s is HVCP Device!\r\n",input_device);
        }
        {
            main_log("input config:baudrate=%d,databits=%d,parity=%c,stopbits=%d\r\n",input_baudrate,input_databits,input_parity,input_stopbits);
            if(!check_com_port(input_device,input_baudrate,input_databits,input_parity,input_stopbits))
            {
                main_log("check %s port failed!\r\n",input_device);
                return -1;
            }
        }
    }
    else
    {
        main_log("input_device is not set!\r\n");
        return -1;
    }
    if(strlen(output_device)>0)
    {
        if(strcmp(strstr(input_device,"COM"),strstr(output_device,"COM"))==0)
        {
            main_log("output_device can not be input_device!\r\n",output_device);
            return -1;
        }
        main_log("output_device is %s!\r\n",output_device);
        if(HVCP_Exists(output_device)==0)
        {
            main_log("%s is HVCP Device!\r\n",output_device);
        }
        {
            main_log("output config:baudrate=%d,databits=%d,parity=%c,stopbits=%d\r\n",output_baudrate,output_databits,output_parity,output_stopbits);
            if(!check_com_port(output_device,output_baudrate,output_databits,output_parity,output_stopbits))
            {
                main_log("check %s port failed!\r\n",output_device);
                return -1;
            }
        }
    }
    else
    {
        main_log("output_device is not set!\r\n");
        return -1;
    }
    return 0;
}

static std::vector<std::string> hvcp_portnames;
static bool check_hvcp_port_removal()
{
    bool ret=true;
    {
        for(auto it=hvcp_portnames.begin(); it!=hvcp_portnames.end(); it++)
        {
            if(HVCP_Exists((*it).c_str())!=0)
            {
                ret=false;
                main_log("%s removed!\r\n",(*it).c_str());
                break;
            }
        }
    }
    return ret;
}

static SERVICE_STATUS m_status= {0};
static bool service_stop_pending=false;
static void WINAPI Handler(DWORD dwOpcode)
{
    switch(dwOpcode)
    {
    case SERVICE_CONTROL_STOP:
    {
        service_stop_pending=true;
    }
    break;
    default:
    {

    }
    break;
    }
};
static void WINAPI ServiceMain(DWORD dwNumServicesArgs,LPSTR *lpServiceArgVectors)
{
    m_status.dwServiceType= SERVICE_WIN32_OWN_PROCESS;
    m_status.dwCurrentState=SERVICE_START_PENDING;
    m_status.dwControlsAccepted=SERVICE_ACCEPT_STOP;
    SERVICE_STATUS_HANDLE hServiceStatusHandle=RegisterServiceCtrlHandlerA(get_config_id(),Handler);
    if(hServiceStatusHandle==NULL)
    {
        main_log("handler not installed!");
        return;
    }
    m_status.dwWin32ExitCode=S_OK;
    SetServiceStatus(hServiceStatusHandle,&m_status);
    {
        //运行服务,应当是一个死循环，退出后服务退出。
        SerialPort inputdev;
        SerialPort outputdev;
        {
            //添加要检查的HVCP端口
            if(HVCP_Exists(input_device)==0)
            {
                hvcp_portnames.push_back(input_device);
            }
            if(HVCP_Exists(output_device)==0)
            {
                hvcp_portnames.push_back(output_device);
            }
        }
        m_status.dwCurrentState=SERVICE_RUNNING;
        SetServiceStatus(hServiceStatusHandle,&m_status);
        while(!service_stop_pending)
        {
            {
                //打开设备
                inputdev.Open(input_device,input_baudrate,input_databits,input_parity,input_stopbits);
                outputdev.Open(output_device,output_baudrate,output_databits,output_parity,output_stopbits);
            }
            {
                uint8_t buffer[4096];
                size_t len=inputdev.Read(buffer,sizeof(buffer));
                if(len>0)
                {
                    outputdev.Write(buffer,len);
                }
            }
            {
                uint8_t buffer[4096];
                size_t len=outputdev.Read(buffer,sizeof(buffer));
                if(len>0)
                {
                    inputdev.Write(buffer,len);
                }
            }
            {
                //HVCP 端口移除后，相关服务也自动移除
                if(!check_hvcp_port_removal())
                {
                    ServiceUnistall(get_config_id());
                }
            }
            Sleep(10);
        }
    }
    m_status.dwCurrentState=SERVICE_STOPPED;
    SetServiceStatus(hServiceStatusHandle,&m_status);

};
static void ServiceMainEntry()
{
    SERVICE_TABLE_ENTRY st[]=
    {
        {
            (LPSTR)get_config_id(),ServiceMain
        },
        {
            NULL,NULL
        }
    };
    if(!StartServiceCtrlDispatcherA(st))
    {
        main_log("StartServiceCtrlDispatcher failed!\r\n");
    }
}

static void ServiceMaintenance(int argc,const char *argv[])
{
    if(uninstall_mode)
    {
        main_log("service uninstall!\r\n");
        if(!ServiceUnistall(get_config_id()))
        {
            main_log("service uninstall failed!\r\n");
        }
    }
    else
    {
        main_log("service install!\r\n");
        if(ServiceIsInstalled(get_config_id()))
        {
            //服务已安装
            if(!ServiceStart(get_config_id()))
            {
                main_log("service not started!\r\n");
            }

        }
        else
        {
            //服务未安装
            std::string service_cmd;
            {
                CHAR file_path[_MAX_PATH]= {0};
                GetModuleFileNameA(NULL,file_path,sizeof(file_path)-1);
                service_cmd+=file_path;
            }
            for(size_t i=1; i<argc; i++)
            {
                service_cmd+=" ";
                service_cmd+=argv[i];
            }
            main_log("service:name=%s,cmd=%s\r\n",get_config_id(),service_cmd.c_str());
            if(!ServiceInstall(get_config_id(),"HVCP PortRedirect",service_cmd.c_str()))
            {
                main_log("service install failed!\r\n");
            }
            else
            {
                if(!ServiceStart(get_config_id()))
                {
                    main_log("service not started!\r\n");
                }
            }
        }
    }
}

int main(int argc,const char * argv[])
{
    arg_parse(argc,argv);
    check_service_mode();
    if(!IsServiceMode)
    {
        if(check_com_settings()==0 || uninstall_mode)
        {
            ServiceMaintenance(argc,argv);
        }
        else
        {
            return -1;
        }
    }
    else
    {
        ServiceMainEntry();
    }
    return 0;
}


