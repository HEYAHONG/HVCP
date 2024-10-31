#ifndef __SERVICE_H__
#define __SERVICE_H__

#ifdef __cplusplus
extern "C"
{
#endif // __cplusplus
#include "stdbool.h"


/** \brief 服务是否安装
 *
 * \param service_name const char* 服务名称
 * \return bool 是否安装
 *
 */
bool ServiceIsInstalled(const char *service_name);

/** \brief 服务安装
 *
 * \param service_name const char* 服务名称
 * \param display_name const char* 服务显示名称
 * \param cmd const char* 命令
 * \return bool 是否安装成功
 */
bool ServiceInstall(const char *service_name,const char *display_name,const char *cmd);

/** \brief 服务卸载
 *
 * \param service_name const char* 服务名称
 * \return bool 是否卸载成功
 */
bool ServiceUnistall(const char *service_name);

/** \brief 服务是否启动
 *
 * \param service_name const char* 服务名称
 * \return bool 服务是否启动
 *
 */
bool ServiceIsStarted(const char *service_name);

/** \brief 服务启动
 *
 * \param service_name const char* 服务名称
 * \return bool 服务是否启动
 *
 */
bool ServiceStart(const char *service_name);


#ifdef __cplusplus
}
#endif // __cplusplus

#endif // __SERVICE_H__
