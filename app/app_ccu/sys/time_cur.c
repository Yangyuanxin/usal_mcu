/***************************************************************************************************   
Copyright @ 2021-20xxС�ۿƼ�

�ļ�:   time_cur
����:   km
˵��:   ʱ��.
***************************************************************************************************/

#include <string.h>
#include <stdio.h>
#include "fun.h"

#include "time_cur.h"

static uint32_t   g_uCur1sTime = 0;
static volatile uint32_t  g_uCur1msTime = 0;
static char timeString[32];
static char timeStringSingle[8];
static char timeBuffer[8];
static char timeBuffer2[7];

uint16_t  sycTimeToScree = 0; 

//ԭ�ģ�https://blog.csdn.net/kernel_digger/article/details/6730772?utm_source=copy
uint32_t kernel_mktime(const uint16_t year0, const uint16_t mon0, const uint16_t day, 
    const uint16_t hour, const uint16_t min, const uint16_t sec)
{   
    uint16_t mon = mon0, year = year0;  /* 1..12 -> 11,12,1..10 */  

    if (0 >= (int) (mon -= 2)) 
    {        
        mon += 12;  /* Puts Feb last since it has leap day */       
        year -= 1;  
    }  
    
    return ((((uint32_t)(year/4 - year/100 + year/400 + 367*mon/12 + day) 
        + year*365 - 719499)*24 
        + hour /* now have hours */)*60 
        + min /* now have minutes */)*60 
        + sec; /* finally seconds */
}

void real_timer_init_delay(uint32_t year,uint32_t mon_day,uint32_t hour_min_sec,uint16_t delayTime)
{    
    struct tm tm_time = {(hour_min_sec)&0xff, ((hour_min_sec>>8)&0xff), (hour_min_sec>>16), 
        (mon_day&0x0ff), ((mon_day>>8)-1), (year-1900)};

    set_curtimer_val_delay((uint32_t)mktime(&tm_time), delayTime);
}

/***************************************************************************************************
����:   getTimerVal.
����:   ��.
����:   g_uCur1sTime-��ǰʱ�����.
˵��:   ��ȡ��ǰʱ�����.
***************************************************************************************************/
uint32_t get_curtimer_val(void)
{
    uint32_t secondCount = g_uCur1sTime;
    
    return secondCount;
}

uint32_t get_curtimer_ms(void)
{
    uint32_t secondCount = g_uCur1msTime;
    
    return secondCount;
}

void set_curtimer_val(uint32_t second)
{
    g_uCur1sTime = second;
}

void set_curtimer_val_delay(uint32_t second, uint16_t delayTime)
{
    set_curtimer_val(second);
    
    sycTimeToScree = delayTime;
}

/***************************************************************************************************
����:   getTimerVal.
����:   val-��ǰʱ�����.
����:   ��.
˵��:   ���õ�ǰʱ�����.
***************************************************************************************************/
//void set_curtimer_val(uint32_t val)
//{
//    g_uCur1sTime = val+8*3600;//ת���ɱ���ʱ��
//}

/***************************************************************************************************
����:   Timer1ms.
����:   ��.
����:   ��.
˵��:   Timer��1S��ʱִ�к���.
***************************************************************************************************/
//������������������У���Ϊ������ܻᱻ����������ϵͳʱ����
void timer1ms_handle_sys(void)
{
    if(1000 <= ++g_uCur1msTime)
    {
        g_uCur1sTime++;
        g_uCur1msTime = 0;
    }
}

/***************************************************************************************************
����:   Timer1s.
����:   ��.
����:   ��.
˵��:   Timer��1S��ʱִ�к���.
***************************************************************************************************/
void timer1s_handle(void)
{
    //g_uCur1sTime++;
    //g_uCur1msTime = 0;
    if (sycTimeToScree > 0)
    {
        sycTimeToScree--;
    }
}

void get_time_string_second(char * pBuffer, uint32_t timeSecond)
{    
    struct tm * pt = localtime(&timeSecond);
    
    sprintf((char *)pBuffer, 
        "%d-%02d-%02d %02d:%02d:%02d",
        pt->tm_year + 1900, pt->tm_mon + 1, pt->tm_mday, pt->tm_hour, pt->tm_min, pt->tm_sec);
}

/***************************************************************************************************
����:   getTimeString.
����:   ��.
����:   ��.
˵��:   ��ʾ��ǰʱ��.
***************************************************************************************************/
char *get_time_string(void)
{
    static uint32_t timeVal = 0;
    if(timeVal != g_uCur1sTime)
    {
        timeVal = g_uCur1sTime;
        get_time_string_second(timeString, timeVal);
    }
    
    uint32_t msCount = g_uCur1msTime;
    sprintf(&timeString[19],".%03d",msCount);

    return timeString;
}

char * get_time_string_single(void)
{
    uint32_t msCount = g_uCur1msTime;     
    uint32_t timeVal = g_uCur1sTime; 
    
    sprintf((char *)&timeStringSingle, "%03d.%03d", timeVal % 1000, msCount);    
    
    return timeStringSingle;
}

void get_time_string_stack(char * pString)
{
    uint32_t msCount = g_uCur1msTime;     
    uint32_t timeVal = g_uCur1sTime; 
    
    get_time_string_second(pString, timeVal);
    sprintf(&pString[19],".%03d",msCount);   
}

//��׼ʱ��ʱ�䣬��ʽ�� xxxx-xx-xx-xx-xx-xx �� -��-��-ʱ-��-�룬��
//BCD ���ʾ,���һλ�Ǳ���λ 0xff
uint32_t get_time_format_second(char * pTimeBcdBuffer)
{
    struct tm nowtime;
    int year = FROM_BCD(pTimeBcdBuffer[0]) * 100 + FROM_BCD(pTimeBcdBuffer[1]);
    
    nowtime.tm_sec = FROM_BCD(pTimeBcdBuffer[6]);
    nowtime.tm_min = FROM_BCD(pTimeBcdBuffer[5]);
    nowtime.tm_hour = FROM_BCD(pTimeBcdBuffer[4]);
    nowtime.tm_mday = FROM_BCD(pTimeBcdBuffer[3]);
    nowtime.tm_mon = FROM_BCD(pTimeBcdBuffer[2]) - 1;
    nowtime.tm_year = year - 1900;
    nowtime.tm_isdst = -1;
    
    return mktime(&nowtime) - (8 * 60 * 60);    
}

void get_time_format_bcd(char * pBuffer, uint32_t timeSecond)
{
    struct tm * pt = localtime(&timeSecond);    

    int year = (pt->tm_year + 1900) / 100;
    pBuffer[0] = HEXTOBCD(year);
    year = (pt->tm_year + 1900) % 100;
    pBuffer[1] = HEXTOBCD(year);
    
    int mon = pt->tm_mon + 1;
    pBuffer[2] = HEXTOBCD(mon);
    pBuffer[3] = HEXTOBCD(pt->tm_mday);
    pBuffer[4] = HEXTOBCD(pt->tm_hour);//INFO("----%d", pt->tm_hour);
    pBuffer[5] = HEXTOBCD(pt->tm_min);
    pBuffer[6] = HEXTOBCD(pt->tm_sec);
    pBuffer[7] = 0xff;    
}

/***************************************************************************************************
����:   get_time_buffer.
����:   .
����:   .
˵��:   ��ǰʱ��.
***************************************************************************************************/
char *get_time_buffer(void)
{
    uint32_t timeVal = get_curtimer_val();  

    get_time_format_bcd(timeBuffer, timeVal);
    
    return timeBuffer;
}

/***************************************************************************************************
����:   get_time_buffer_cp56time2a.
����:   .
����:   .
˵��:   ��ǰʱ��.
***************************************************************************************************/
char *get_time_buffer_cp56time2a(void)
{
    struct tm *pt = 0;
    uint32_t timeVal = get_curtimer_val();
    uint8_t year = 0;
    uint16_t msec = 0;
    pt = localtime(&timeVal);
    
    year = (uint8_t)(pt->tm_year - 2000);
    timeBuffer2[6] = year;
    pt->tm_mon = pt->tm_mon + 1;
    timeBuffer2[5] = pt->tm_mon;
    
    timeBuffer2[4] = pt->tm_mday;
    timeBuffer2[3] = pt->tm_hour;
    timeBuffer2[2] = pt->tm_min;
	msec = pt->tm_sec*1000;
    timeBuffer2[1] = (uint8_t)(msec>>8) & 0xff;
	msec = pt->tm_sec*1000 ;
    timeBuffer2[0] = (uint8_t)msec & 0xff ;

    return timeBuffer2;
}


/***************************************************************************************************
�ļ�����.
***************************************************************************************************/

