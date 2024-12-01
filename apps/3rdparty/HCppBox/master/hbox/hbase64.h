/***************************************************************
 * Name:      hbase64.h
 * Purpose:   声明base64接口,参考RFC2045～RFC2049。
 * Author:    HYH (hyhsystem.cn)
 * Created:   2024-09-02
 * Copyright: HYH (hyhsystem.cn)
 * License:   MIT
 **************************************************************/
#ifndef __HBASE64_H__
#define __HBASE64_H__
#include "stdint.h"
#include "stdbool.h"
#include "stdlib.h"

#ifdef __cplusplus
extern "C"
{
#endif // __cplusplus

/** \brief 解码
 *
 * \param value uint8_t* 解码数据缓冲区，当其为NULL时仅返回长度不做处理
 * \param value_length size_t 解码数据缓冲区长度
 * \param encoding const char* 待解码数据缓冲区，需要确保内容为标准base64编码，否则可能出错。
 * \param encoding_length size_t 待解码数据缓冲区长度，不含'\0' 字符
 * \return size_t 解码长度
 *
 */
size_t hbase64_decode(uint8_t *value,size_t value_length,const char *encoding,size_t encoding_length);

/** \brief 编码
 *
 * \param encoding char* 编码数据缓冲区，当其为NULL时仅返回长度不做处理
 * \param encoding_length size_t 编码数据缓冲区长度
 * \param uint8_t*value const 待编码数据缓冲区
 * \param value_length size_t 待编码数据缓冲区长度
 * \return size_t 编码长度，注意：本函数不会添加'\0',也不会将'\0'计入长度。
 *
 */
size_t hbase64_encode(char *encoding,size_t encoding_length,const uint8_t*value,size_t value_length);

/** \brief 编码
 *
 * \param encoding char* 编码数据缓冲区
 * \param encoding_length size_t 编码数据缓冲区长度
 * \param uint8_t*value const 待编码数据缓冲区
 * \param value_length size_t 待编码数据缓冲区长度
 * \return size_t 编码长度(含'\0')
 *
 */
size_t hbase64_encode_with_null_terminator(char *encoding,size_t encoding_length,const uint8_t*value,size_t value_length);

/** \brief 获取base64编码字符串长度，通常用于解码
 *
 * \param encoding const char* base64编码字符串（以'\0'结尾的字符串）
 * \return size_t base64编码字符串长度
 *
 */
size_t hbase64_get_encoding_length(const char *encoding);

#ifdef __cplusplus
}
#endif // __cplusplus

#endif // __HBASE64_H__

