#include "HVCP.h"
#include "stdlib.h"
#include "stdio.h"
int main()
{
    printf("----HVCP_Enum----\r\n");
    HVCP_Enum([](const char *port,void *usr)
    {
        (void)usr;
        printf("%s\r\n",port);
    },NULL);
    printf("----HVCP_Enum----\r\n");
}


