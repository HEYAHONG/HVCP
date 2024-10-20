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

void HVCP_Enum(void(*OnEnum)(const char * device,void *usr),void *usr);

#ifdef __cplusplus
}
#endif // __cplusplus

