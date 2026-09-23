/**
 ******************************************************************************
 * @file    stm32f10x_reg.h
 * @brief   STM32F103C8T6 寄存器定义（本实验只用 RCC / GPIO / SysTick）
 *
 *          所有地址均出自：
 *          《STM32中文参考手册 RM0008 V10》 表1(寄存器组起始地址)、
 *          第6章(RCC)、第8章(GPIO)，以及 ARM《Cortex-M3技术参考手册》(SysTick)。
 *
 *          本头文件不依赖任何库文件，仅需 C 标准头 stdint.h，
 *          可在"启动文件 + 本工程源码"的最小工程中直接编译。
 ******************************************************************************
 */
#ifndef __STM32F10X_REG_H
#define __STM32F10X_REG_H

#include <stdint.h>

/*============================================================================
 * 一、基地址定义
 *==========================================================================*/

/* Cortex-M3 内核外设基地址（SysTick 属于内核，不属于片上外设总线） */
#define SysTick_BASE    0xE000E010UL   /* SysTick 定时器：CTRL/LOAD/VAL/CALIB */

/* APB2 总线外设区基地址（参考手册表1：0x4001 0000 起为 APB2 外设区） */
#define APB2_BASE       0x40010000UL

/* 复位和时钟控制 RCC：0x4002 1000（参考手册表1、6.3.11节） */
#define RCC_BASE        (APB2_BASE + 0x1000UL)

/* GPIO 端口基地址（参考手册表1、8.5节 GPIO寄存器地址映像） */
#define GPIOA_BASE      (APB2_BASE + 0x0800UL)   /* 0x4001 0800 GPIO端口A */
#define GPIOB_BASE      (APB2_BASE + 0x0C00UL)   /* 0x4001 0C00 GPIO端口B */
#define GPIOC_BASE      (APB2_BASE + 0x1000UL)   /* 0x4001 1000 GPIO端口C */

/*============================================================================
 * 二、寄存器结构体定义
 *     按寄存器在地址空间中的排列顺序声明成员，偏移自动与芯片一致
 *==========================================================================*/

/* RCC 结构体（参考手册 6.3.11 RCC寄存器地址映像） */
typedef struct
{
    volatile uint32_t CR;        /* 0x00 时钟控制寄存器           */
    volatile uint32_t CFGR;      /* 0x04 时钟配置寄存器           */
    volatile uint32_t CIR;       /* 0x08 时钟中断寄存器           */
    volatile uint32_t APB2RSTR;  /* 0x0C APB2外设复位寄存器       */
    volatile uint32_t APB1RSTR;  /* 0x10 APB1外设复位寄存器       */
    volatile uint32_t AHBENR;    /* 0x14 AHB外设时钟使能寄存器    */
    volatile uint32_t APB2ENR;   /* 0x18 APB2外设时钟使能寄存器   */
    volatile uint32_t APB1ENR;   /* 0x1C APB1外设时钟使能寄存器   */
    volatile uint32_t BDCR;      /* 0x20 备份域控制寄存器         */
    volatile uint32_t CSR;       /* 0x24 控制/状态寄存器          */
} RCC_TypeDef;

/* GPIO 结构体（参考手册 8.5 GPIO寄存器地址映像）
 * 每个端口均有以下 7 个寄存器，偏移固定： */
typedef struct
{
    volatile uint32_t CRL;       /* 0x00 端口配置低寄存器 (引脚0~7)，复位值0x4444 4444 */
    volatile uint32_t CRH;       /* 0x04 端口配置高寄存器 (引脚8~15)，复位值0x4444 4444 */
    volatile uint32_t IDR;       /* 0x08 端口输入数据寄存器 (只读)                     */
    volatile uint32_t ODR;       /* 0x0C 端口输出数据寄存器                            */
    volatile uint32_t BSRR;      /* 0x10 端口位设置/清除寄存器 (只写)                  */
    volatile uint32_t BRR;       /* 0x14 端口位清除寄存器 (只写)                       */
    volatile uint32_t LCKR;      /* 0x18 端口配置锁定寄存器                            */
} GPIO_TypeDef;

/* SysTick 结构体（ARM Cortex-M3 TRM：基地址 0xE000 E010） */
typedef struct
{
    volatile uint32_t CTRL;      /* 0x00 控制和状态寄存器    */
    volatile uint32_t LOAD;      /* 0x04 重装载值寄存器      */
    volatile uint32_t VAL;       /* 0x08 当前值寄存器        */
    volatile uint32_t CALIB;     /* 0x0C 校准值寄存器        */
} SysTick_TypeDef;

/* 把基地址强制转换成结构体指针后，即可用"结构体成员"方式访问寄存器，
 * 例如 RCC->APB2ENR、GPIOA->CRL、SysTick->LOAD 等 */
#define RCC      ((RCC_TypeDef *)RCC_BASE)
#define GPIOA    ((GPIO_TypeDef *)GPIOA_BASE)
#define GPIOB    ((GPIO_TypeDef *)GPIOB_BASE)
#define GPIOC    ((GPIO_TypeDef *)GPIOC_BASE)
#define SysTick  ((SysTick_TypeDef *)SysTick_BASE)

/*============================================================================
 * 三、常用位定义
 *==========================================================================*/

/* RCC_APB2ENR（参考手册 6.3.7节，GPIO 挂在 APB2 高速总线上） */
#define RCC_APB2ENR_AFIOEN  (1UL << 0)    /* 位0 ：复用功能IO时钟使能    */
#define RCC_APB2ENR_IOPAEN  (1UL << 2)    /* 位2 ：IO端口A时钟使能       */
#define RCC_APB2ENR_IOPBEN  (1UL << 3)    /* 位3 ：IO端口B时钟使能       */
#define RCC_APB2ENR_IOPCEN  (1UL << 4)    /* 位4 ：IO端口C时钟使能       */

/* SysTick_CTRL（ARM Cortex-M3 TRM） */
#define SysTick_CTRL_ENABLE     (1UL << 0)    /* 位0 ：计数器使能             */
#define SysTick_CTRL_TICKINT    (1UL << 1)    /* 位1 ：计数到0产生异常(本实验不用中断，置0) */
#define SysTick_CTRL_CLKSOURCE  (1UL << 2)    /* 位2 ：时钟源 1=HCLK, 0=HCLK/8 */
#define SysTick_CTRL_COUNTFLAG  (1UL << 16)   /* 位16：计数到0时硬件置1，读该位自动清零 */

#endif /* __STM32F10X_REG_H */
