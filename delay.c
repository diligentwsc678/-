/**
 ******************************************************************************
 * @file    delay.c
 * @brief   毫秒级延时函数实现（SysTick 寄存器方式）
 *
 *   SysTick 是 Cortex-M3 内核自带的 24 位递减计数器，寄存器位于
 *   0xE000 E010 ~ 0xE000 E01C（见 stm32f10x_reg.h），不占用片上外设资源。
 *
 *   复位后 STM32F103 默认时钟（参考手册第6章）：
 *     SYSCLK = HSI = 8MHz（RCC_CFGR 复位值 0x0000 0000，SW=00 选择 HSI）
 *     HPRE / PPRE1 / PPRE2 复位值均为 000 → 不分频
 *     即 HCLK = SYSCLK = 8MHz
 ******************************************************************************
 */
#include "delay.h"
#include "stm32f10x_reg.h"

/* 系统时钟频率（Hz）。
 * 本工程保持复位后的默认时钟 HSI 8MHz。
 * 若改用 72MHz（PLL 倍频），请把该值改为 72000000UL。 */
#define SYSCLK_HZ   8000000UL

/**
 * @brief  毫秒级延时
 * @param  ms : 延时毫秒数
 *
 * 实现原理：
 *  1. SysTick 选择 HCLK 作计数时钟（CTRL 位2 CLKSOURCE=1），
 *     即每个计数周期为 1/8MHz = 125ns；
 *  2. 1ms 需要计数 8MHz/1000 = 8000 次，故 LOAD = ms*8000 - 1
 *     （减1是因为计数器从 LOAD 减到 0 共 LOAD+1 个周期）；
 *  3. 写 VAL=0 清零当前值（同时清除 COUNTFLAG 标志）；
 *  4. 使能计数器，等待计数到 0：硬件置位 CTRL 位16 COUNTFLAG，
 *     读 CTRL 会看到该标志（读该位即自动清零）；
 *  5. 关闭计数器。
 *
 *  24 位计数器最大值 0xFFFFFF = 16777215，
 *  故单次延时上限约 16777216/8000 ≈ 2097ms。
 */
void delay_ms(uint32_t ms)
{
    uint32_t load_val;

    /* 计算重装载值：LOAD = 每毫秒计数值 × 毫秒数 − 1 */
    load_val = ms * (SYSCLK_HZ / 1000UL) - 1UL;

    SysTick->LOAD = load_val;                       /* 设置重装载值        */
    SysTick->VAL  = 0UL;                            /* 当前值清零          */
    SysTick->CTRL = SysTick_CTRL_ENABLE
                  | SysTick_CTRL_CLKSOURCE;         /* 使能 + 时钟源=HCLK  */

    /* 忙等待：计数到0后硬件置位 COUNTFLAG（CTRL 位16） */
    while ((SysTick->CTRL & SysTick_CTRL_COUNTFLAG) == 0UL)
    {
        /* 空循环等待，也可在此做低功耗处理（本实验不需要） */
    }

    SysTick->CTRL = 0UL;                            /* 关闭计数器          */
}
