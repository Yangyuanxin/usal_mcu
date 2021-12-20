/***************************************************************************************************   
Copyright @ 2021-20xxС�ۿƼ�

�ļ�:   log
����:   km
˵��:   ���ڴ�ӡ
***************************************************************************************************/

#ifndef __LOG_H__                     
#define __LOG_H__

#include "base_type.h"

/*
  * @brief ��ӡ�ֳ������ȼ���log_d��log_show��log_flash
  * log_d ���ڴ�ӡ(DEBUG)
  * log_hex ���ڴ�ӡHEX
  * log_show ���ڴ�ӡ+��Ļ��ʾ
  * log_flash ���ڴ�ӡ+��Ļ��ʾ+FLASH��־
  */
void log_d(const char *format, ...);
void log_hex(char *head, uint8_t *data, uint8_t len, uint32_t cmd);
void log_show(const char *format, ...);
void log_flash(const char *format, ...);

#ifdef CHECK_VERSION
#define DLOG(format, args...)
//#elif(DCHARGER_VERSION_BRANCH % 2 == 1)
//#define DLOG log_d
#else
#define DLOG log_d
#define XJ_LOG log_d
//#define DLOG(format, args...)
#endif





#endif  //__CFUN_H__

/***************************************************************************************************
�ļ�����.
***************************************************************************************************/


