#include "HVCP.h"
#include "stdlib.h"
#include "stdio.h"
int main(int argc,const char *argv[])
{
    if(argc > 1)
    {
        return HVCP_DriverRemove(argv[1]);
    }
    return -1;
}


