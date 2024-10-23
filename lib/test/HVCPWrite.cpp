#include "HVCP.h"
#include "stdlib.h"
#include "stdio.h"
int main(int argc,const char *argv[])
{
    if(argc >= 3)
    {
        return (0!=HVCP_Write(argv[1],argv[2],strlen(argv[2])))?0:-1;
    }
    return -1;
}

