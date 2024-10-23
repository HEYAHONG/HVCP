#include "HVCP.h"
#include "stdlib.h"
#include "stdio.h"
int main(int argc,const char *argv[])
{
    if(argc == 1)
    {
        return -1;
    }
    while(true)
    {
        char buff[4096]= {0};
        size_t len=HVCP_Read(argv[1],buff,sizeof(buff)-1);
        if(len > 0)
        {
            printf("%s",buff);
        }
    }
    return 0;
}


