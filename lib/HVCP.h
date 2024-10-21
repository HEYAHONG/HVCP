/***************************************************************
 * Name:      HVCP.h
 * Purpose:   HVCP头文件
 * Author:    HYH (hyhsystem.cn)
 * Created:   2024-10-20
 * Copyright: HYH (hyhsystem.cn)
 * License:   MIT
 **************************************************************/
#ifdef __cplusplus
extern "C"
{
#endif // __cplusplus
#include "stdlib.h"
#include "string.h"

/** \brief 枚举HVCP设备。
 *
 * \param OnEnum void(*)(const char * device,void *usr) 回调函数,第一个参数为的设备名称(如COM1、COM2等)，第二个参数为用户指针。
 * \param usr void* 用于回调函数的用户指针。
 *
 */
void HVCP_Enum(void(*OnEnum)(const char * device,void *usr),void *usr);

/** \brief 写入HVCP数据。
 *
 * \param device const char* 设备名称,注意:设备名称中的字母均需大写。
 * \param buffer const void* 数据缓冲区。
 * \param length size_t 待写入的字节数。
 * \return size_t 已写入的字节数。
 *
 */
size_t HVCP_Write(const char *device,const void *buffer,size_t length);

/** \brief 读取HVCP数据,注意:此函数的返回由驱动决定，不一定会立即返回。
 *
 * \param device const char* 设备名称,注意:设备名称中的字母均需大写。
 * \param buffer const void* 数据缓冲区。
 * \param length size_t 待读取的字节数。
 * \return size_t 已读取的字节数。
 *
 */
size_t HVCP_Read(const char *device,void *buffer,size_t length);

#ifdef __cplusplus
}
#endif // __cplusplus

