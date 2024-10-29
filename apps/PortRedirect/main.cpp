
#include "stdlib.h"
#include "stdio.h"
#include "stdarg.h"
#include "string.h"
#include "HVCP.h"
#include <thread>
#include <libserialport.h>

static int main_log(const char *fmt,...)
{
    int ret=0;
    va_list va;
    va_start(va,fmt);
    {
        ret=vprintf(fmt,va);
    }
    va_end(va);

    return ret;
}

static int cmd_help(int argc,const char *argv[]);
static int cmd_input(int argc,const char *argv[]);
static int cmd_output(int argc,const char *argv[]);
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
        "--output",
        "-o",
        cmd_output,
        "--output=[device]  / -o [device] ",
        "output device"
    }
};

static int cmd_help(int argc,const char *argv[])
{
    const size_t cmd_max_len=8;
    const size_t usage_max_len=36;
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
static bool check_com_port(const char *port)
{
    if(port==NULL || port[0]=='\0')
    {
        return false;
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
        if(strcmp(strstr(port,"COM"),strstr(sp_get_port_name(port_list[i]),"COM"))==0)
        {
            ret=true;
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

int main(int argc,const char * argv[])
{
    arg_parse(argc,argv);
    if(strlen(input_device)>0)
    {
        main_log("input_device is %s!\r\n",input_device);
        if(HVCP_Exists(input_device)==0)
        {
            main_log("%s is HVCP Device!\r\n",input_device);
        }
        else
        {
            if(!check_com_port(input_device))
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
        main_log("output_device is %s!\r\n",output_device);
        if(HVCP_Exists(output_device)==0)
        {
            main_log("%s is HVCP Device!\r\n",output_device);
        }
        else
        {
            if(!check_com_port(output_device))
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


