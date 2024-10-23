#include "HVCP.h"
#include "stdlib.h"
#include "stdio.h"
int main()
{
    HVCP_Enum([](const char *port,void *usr)
    {
        (void)usr;
        printf("%s\r\n",port);

    },NULL);
    return 0;
}


