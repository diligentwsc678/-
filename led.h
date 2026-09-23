/**
 ******************************************************************************
 * @file    led.h
 * @brief   LED 流水灯驱动（4 只 LED：PA0/PB0/PC0 面包板红绿蓝灯 + PC13 板载灯）
 ******************************************************************************
 */
#ifndef __LED_H
#define __LED_H

#include <stdint.h>

/* 流水灯中每只 LED 的编号（顺序即流水灯点亮的先后顺序） */
typedef enum
{
    LED_RED = 0,        /* 红色LED：PA0，面包板外接，高电平点亮 */
    LED_GREEN,          /* 绿色LED：PB0，面包板外接，高电平点亮 */
    LED_BLUE,           /* 蓝色LED：PC0，面包板外接，高电平点亮 */
    LED_ONBOARD,        /* 板载LED：PC13，核心板出厂焊接，低电平点亮 */

    LED_NUM             /* LED 总数（4），注意它不是一只灯，仅作计数 */
} LedId_t;

void LED_Init(void);            /* 使能 GPIOA/B/C 时钟，配置 4 个引脚为推挽输出 */
void LED_On(LedId_t id);        /* 点亮指定 LED */
void LED_Off(LedId_t id);       /* 熄灭指定 LED */

#endif /* __LED_H */
