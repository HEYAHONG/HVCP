#ifndef __MODBUSRTU_H__
#define __MODBUSRTU_H__
#include "hbox.h"

#ifdef __cplusplus
extern "C"
{
#endif // __cplusplus

/** \brief 初始化modbus上下文
 *
 * \param ctx modbus_rtu_slave_tiny_context_t* modbus rtu上下文
 *
 */
void modbus_init_ctx(modbus_rtu_slave_tiny_context_t* ctx);

#ifdef __cplusplus
}
#endif // __cplusplus



#endif // __MODBUSRTU_H__
