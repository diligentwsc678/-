/**
 ******************************************************************************
 * @file    led.c
 * @brief   LED 流水灯驱动实现（纯寄存器方式）
 *
 *  引脚分配：
 *    PA0  → 面包板红色LED（GPIO输出高电平 → LED点亮）
 *    PB0  → 面包板绿色LED（同上，高电平点亮）
 *    PC0  → 面包板蓝色LED（同上，高电平点亮）
 *    PC13 → 核心板板载LED（板载原理图：3.3V → 1kΩ → LED阳极，
 *                          阴极接PC13，故输出低电平 → LED点亮）
 *
 *  主要寄存器（详见《STM32中文参考手册》第6、8章）：
 *    RCC_APB2ENR  偏移0x18 @0x4002 1018：IOPAEN位2 / IOPBEN位3 / IOPCEN位4
 *    GPIOx_CRL    偏移0x00：引脚0~7 配置，每引脚4位 [MODE(1:0)+CNF(1:0)]
 *    GPIOx_CRH    偏移0x04：引脚8~15 配置，格式同上
 *    GPIOx_BSRR   偏移0x10：低16位BSy写1→ODR对应位置1；高16位BRy写1→清0
 *    GPIOx_BRR    偏移0x14：低16位BRy写1→ODR对应位清0
 ******************************************************************************
 */
#include "led.h"
#include "stm32f10x_reg.h"

/* 每只 LED 的属性描述 */
typedef struct
{
    GPIO_TypeDef *port;     /* 所在的 GPIO 端口 */
    uint16_t      pin;      /* 引脚编号（0~15） */
    uint8_t       active_high; /* 1=高电平点亮(引脚输出1灯亮)；0=低电平点亮(输出0灯亮) */
    uint8_t       cfg;      /* 写入 CRL/CRH 该引脚4位配置域的值：
                             *   bit[1:0] = MODE  速度：01=10MHz  10=2MHz  11=50MHz
                             *   bit[3:2] = CNF   输出类型：00=通用推挽  01=通用开漏 */
} Led_Attr_t;

/* LED 属性表：增加/减少 LED 只需修改这张表，程序主体不用动 */
static const Led_Attr_t LED_TABLE[LED_NUM] =
{
    /* 端口    引脚   点亮电平   4位配置(CNF=00推挽, MODE速度) */
    { GPIOA,     0,        1,       0x3 },   /* LED_RED     : PA0  高电平点亮 50MHz推挽 */
    { GPIOB,     0,        1,       0x3 },   /* LED_GREEN   : PB0  高电平点亮 50MHz推挽 */
    { GPIOC,     0,        1,       0x3 },   /* LED_BLUE    : PC0  高电平点亮 50MHz推挽 */
    { GPIOC,    13,        0,       0x2 },   /* LED_ONBOARD : PC13 低电平点亮 2MHz推挽
                                              * 注：数据手册表3注4——PC13/PC14/PC15经内部
                                              * 电源开关供电，输出模式仅允许2MHz(最大30pF
                                              * 负载)，且三者同时只能有1个作输出，故这里
                                              * 配置为 MODE=10(2MHz) 而非 50MHz */
};

/**
 * @brief  LED 初始化
 *  1. 使能 GPIOA、GPIOB、GPIOC 端口时钟（GPIO 挂在 APB2 总线上，
 *     参考手册第6章：复位后所有外设时钟默认关闭，不使能时钟写寄存器无效）；
 *  2. 按属性表把 4 个引脚配置为通用推挽输出；
 *  3. 熄灭全部 LED（上电后引脚默认浮空输入，电平不确定，先给确定状态）。
 */
void LED_Init(void)
{
    uint8_t i;

    /* ---- 第1步：使能端口时钟 ----
     * RCC_APB2ENR |= IOPAEN(位2) | IOPBEN(位3) | IOPCEN(位4) */
    RCC->APB2ENR |= RCC_APB2ENR_IOPAEN
                  | RCC_APB2ENR_IOPBEN
                  | RCC_APB2ENR_IOPCEN;

    /* ---- 第2步：逐个引脚配置为推挽输出 ----
     * CRL 管理引脚0~7，CRH 管理引脚8~15；
     * 每个引脚占 4 位，引脚 n 对应的位域是 bit[4n+3 : 4n]（n≤7），
     * 或 bit[4(n-8)+3 : 4(n-8)]（n≥8）。
     * 复位值 0x4444 4444 = 所有引脚"浮空输入"。
     * 采用"读-改-写"：先屏蔽(清0)该4位，再写入新配置，不影响其他引脚。 */
    for (i = 0; i < LED_NUM; i++)
    {
        const Led_Attr_t *p = &LED_TABLE[i];
        uint32_t pos;

        if (p->pin < 8)
        {
            pos = (uint32_t)p->pin * 4U;                          /* 位域起始位置 */
            p->port->CRL = (p->port->CRL & ~(0x0FU << pos))       /* 先清0该4位 */
                         | ((uint32_t)p->cfg << pos);             /* 再写入新值 */
        }
        else
        {
            pos = (uint32_t)(p->pin - 8U) * 4U;
            p->port->CRH = (p->port->CRH & ~(0x0FU << pos))
                         | ((uint32_t)p->cfg << pos);
        }
    }

    /* ---- 第3步：初始状态全部熄灭 ---- */
    for (i = 0; i < LED_NUM; i++)
    {
        LED_Off((LedId_t)i);
    }
}

/**
 * @brief  点亮指定 LED
 *
 * 不使用"读 ODR → 修改 → 写回 ODR"的方式，而用 BSRR/BRR：
 *   - BSRR 低16位 BSy 写1 → ODR 对应位被硬件置1（输出高电平）；
 *   - BRR  低16位 BRy 写1 → ODR 对应位被硬件清0（输出低电平）；
 * 写 0 的位不起任何作用。这是硬件原子操作，读-改-写期间即使发生
 * 中断也不会误改其他引脚（参考手册8.1节）。
 */
void LED_On(LedId_t id)
{
    const Led_Attr_t *p = &LED_TABLE[id];

    if (p->active_high)
    {
        p->port->BSRR = (uint32_t)p->pin;   /* 高电平点亮：ODR置1 */
    }
    else
    {
        p->port->BRR  = (uint32_t)p->pin;   /* 低电平点亮：ODR清0 */
    }
}

/**
 * @brief  熄灭指定 LED（与 LED_On 恰好相反的电平）
 */
void LED_Off(LedId_t id)
{
    const Led_Attr_t *p = &LED_TABLE[id];

    if (p->active_high)
    {
        p->port->BRR  = (uint32_t)p->pin;   /* 高电平点亮的灯：ODR清0即灭 */
    }
    else
    {
        p->port->BSRR = (uint32_t)p->pin;   /* 低电平点亮的灯：ODR置1即灭 */
    }
}
