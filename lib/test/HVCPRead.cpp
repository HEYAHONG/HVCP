#include "HVCP.h"
#include "stdlib.h"
#include "stdio.h"
#include "windows.h"
int main(int argc,const char *argv[])
{
    if(argc == 1)
    {
        return -1;
    }
    while(0==HVCP_Exists(argv[1]))
    {
        char buff[4096]= {0};
        size_t len=HVCP_Read(argv[1],buff,sizeof(buff)-1);
        if(len > 0)
        {
            printf("%s",buff);
        }
        Sleep(50);
    }
    return 0;
}


