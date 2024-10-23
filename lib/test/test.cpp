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

        /*
        //取消注释以测试读写
        {
            char buff[4096]="Hello HVCP";
            printf("Write %s (%d bytes)!\r\n",buff,(int)HVCP_Write(port,buff,strlen(buff)));
            printf("Read %s (%d bytes)!\r\n",buff,(int)HVCP_Read(port,buff,sizeof(buff)));
        }
        */

    },NULL);
    printf("----HVCP_Enum----\r\n");

}


