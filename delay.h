/**
 ******************************************************************************
 * @file    delay.h
 * @brief   毫秒级延时函数声明（基于 Cortex-M3 内核的 SysTick 定时器）
 ******************************************************************************
 */
#ifndef __DELAY_H
#define __DELAY_H

#include <stdint.h>

/**
 * @brief  毫秒级延时（忙等待方式）
 * @param  ms : 要延时的毫秒数（1 ~ 2097，由 SysTick 24位计数器决定）
 * @note   复位后系统默认时钟为 HSI 8MHz（AHB/APB 均不分频），
 *         即 HCLK = 8MHz；若改用了 72MHz 时钟，请同步修改 delay.c 中的
 *         SYSCLK_HZ 宏定义。
 */
void delay_ms(uint32_t ms);

#endif /* __DELAY_H */
