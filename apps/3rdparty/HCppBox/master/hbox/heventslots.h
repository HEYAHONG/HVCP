/***************************************************************
 * Name:      heventslots.h
 * Purpose:   声明heventslots接口
 * Author:    HYH (hyhsystem.cn)
 * Created:   2023-07-14
 * Copyright: HYH (hyhsystem.cn)
 * License:   MIT
 **************************************************************/


#ifndef __HEVENTSLOTS_H_INCLUDED__
#define __HEVENTSLOTS_H_INCLUDED__

/*
本文件用于实现一些事件调用常用的回调函数功能,通常配合heventloop.h中的函数使用
    本文件通常用于事件回调函数场合,包含回调函数注册、回调函数取消注册、发送信号(调用回调函数)。
*/

#ifdef __cplusplus
extern "C"
{
#endif // __cplusplus

#include "stdint.h"
#include "stdlib.h"
#include "string.h"
#include "stdbool.h"

struct heventslots; /**< heventslots前置声明 */
typedef struct heventslots heventslots_t; /**< heventslots_t定义 */

/** \brief 获取使用内部堆的heventslots的最小大小
 *
 * \return size_t 使用内部堆的heventslots的最小大小
 *
 */
size_t heventslots_with_internal_heap_min_size(void);

/** \brief 初始化heventslots_t
 *
 * \param  usr void* 用户指针
 * \param  mutex_lock 加锁,不要使用第一个参数
 * \param  mutex_unlock 解锁,不要使用第一个参数
 * \param  mem 内存（此内存可静态分配，大小不小于heventslots_with_internal_heap_min_size()返回的值）
 * \param  length 内存大小
 * \return heventslots_t* heventslots_t指针
 *
 */
heventslots_t * heventslots_with_internal_heap_init(void *usr,void (*mutex_lock)(void *),void (*mutex_unlock)(void *),void *mem,size_t length);

/** \brief 创建heventslots_t
 *
 * \param  usr void* 用户指针
 * \param  mem_alloc 内存分配函数,第一个参数是待分配的字节数，第二个参数是用户指针
 * \param  mem_free  内存释放函数,第一个参数是已分配的指针，第二个参数是用户指针
 * \param  mutex_lock 加锁,第一个参数是用户指针
 * \param  mutex_unlock 解锁,第一个参数是用户指针
 * \return heventslots_t* heventslots_t指针
 *
 */
heventslots_t * heventslots_new_with_memmang_and_lock(void *usr,void *(*mem_alloc)(size_t,void *),void (*mem_free)(void *,void *),void (*mutex_lock)(void *),void (*mutex_unlock)(void *));

/** \brief 创建heventslots_t
 *
 * \param  usr void* 用户指针
 * \param  mem_alloc 内存分配函数,第一个参数是待分配的字节数，第二个参数是用户指针
 * \param  mem_free  内存释放函数,第一个参数是已分配的指针，第二个参数是用户指针
 * \return heventslots_t* heventslots_t指针
 *
 */
heventslots_t * heventslots_new_with_memmang(void *usr,void *(*mem_alloc)(size_t,void *),void (*mem_free)(void *,void *));

/** \brief 创建heventslots_t
 *
 * \param  usr void* 用户指针
 * \param  mutex_lock 加锁,第一个参数是用户指针
 * \param  mutex_unlock 解锁,第一个参数是用户指针
 * \return heventslots_t* heventslots_t指针
 *
 */
heventslots_t * heventslots_new_with_lock(void *usr,void (*mutex_lock)(void *),void (*mutex_unlock)(void *));

/** \brief 创建heventslots_t
 *
 * \param  usr void* 用户指针
 * \return heventslots_t* heventslots_t指针
 *
 */
heventslots_t * heventslots_new(void *usr);

/** \brief 获取heventslots_t的用户指针(usr)
 *
 * \param slots heventslots_t* heventslots_t指针
 * \return void* usr指针
 *
 */

void * heventslots_get_usr_ptr(heventslots_t *slots);

/** \brief 释放heventslots_t
 *
 * \param slots heventslots_t* heventslots_t指针
 *
 */
void heventslots_free(heventslots_t *slots);

/** \brief 发射信号(即依次以信号调用槽函数)
 *
 * \param slots heventslots_t* heventslots_t指针
 * \param signal void* 信号指针
 *
 */
void heventslots_emit_signal(heventslots_t *slots,void *signal);

/** \brief 注册槽函数
 *
 * \param slots heventslots_t* heventslots_t指针
 * \param slot_usr void*槽中的用户参数
 * \param slot 槽函数,第一个参数为信号,第二个参数为用户参数。
 * \param onfree 槽释放回调,第一个参数为用户参数。
 * \return uint32_t 注册id,为0表示失败
 *
 */

uint32_t heventslots_register_slot(heventslots_t *slots,void *slot_usr,void (*slot)(void *,void *),void (*onfree)(void *));


/** \brief 取消槽函数注册
 *
 * \param slots heventslots_t* heventslots_t指针
 * \param id uint32_t 注册id,由注册时返回
 *
 */
void heventslots_unregister_slot(heventslots_t *slots,uint32_t id);


/** \brief 取消所有槽函数注册
 *
 * \param slots heventslots_t* heventslots_t指针
 *
 */
void heventslots_unregister_all_slot(heventslots_t *slots);
#ifdef __cplusplus
}
#endif // __cplusplus

#endif // HEVENTSLOTS_H_INCLUDED
