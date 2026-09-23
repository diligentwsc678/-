# 四灯流水灯工程（纯寄存器版）使用说明

## 工程文件

| 文件 | 说明 |
|------|------|
| `main.c` | 主程序：流水灯逻辑（含 `SystemInit` 空实现，保持默认 8MHz 时钟） |
| `led.c` / `led.h` | LED 驱动：初始化、点亮、熄灭（含 PC13 低电平点亮处理） |
| `delay.c` / `delay.h` | SysTick 毫秒延时 |
| `stm32f10x_reg.h` | 寄存器定义（RCC/GPIO/SysTick 结构体与位定义），不依赖任何库 |

## 在 Keil MDK5 中建立工程

1. **新建工程**：`Project → New µVision Project`，器件选择 `STM32F103C8`。
   （若列表里没有，需先安装器件支持包：`Pack Installer → STMicroelectronics → Keil.STM32F1xx_DFP`）

2. **添加启动文件**：本工程不依赖标准外设库，只需一个启动文件
   `startup_stm32f10x_md.s`（中容量产品），可从 ST 标准外设库 V3.5 中获取：
   `STM32F10x_StdPeriph_Lib_V3.5.0\Libraries\CMSIS\CM3\DeviceSupport\ST\STM32F10x\startup\arm\startup_stm32f10x_md.s`

3. **添加本工程源码**：把本目录下 6 个文件（2 个 .h、3 个 .c 及启动文件）全部加入工程。

4. **配置编译选项**（`Options for Target`，魔术棒图标）：
   - `C/C++` 选项卡 → `Include Paths` 添加本源码所在目录；
     若使用 AC5（armcc）编译器，需勾选 `C99 Mode`（代码中用了 `for(int i...)` 语法）；
     若使用 AC6（armclang）则默认支持，无需勾选。
   - `Debug` 选项卡 → 选择你的仿真器：ST-Link / J-Link / DAP-Link。
   - `Utilities` 选项卡 → 勾选 `Reset and Run`（下载后自动复位运行）。

5. **编译下载**：`Build`（F7）→ `Download`（F8）。
   若提示找不到 flash 算法：`Options for Target → Debug → Settings → Flash Download`
   中选择 `STM32F10x Med-density Flash (128K)`。

6. 把核心板 `BOOT0` 拨到 `0`（从 Flash 启动），按下复位键即可看到流水灯。

## 接线对照表（与 `led.c` 中 `LED_TABLE` 一致）

| LED | 颜色 | 引脚 | 接线方式 | 点亮电平 |
|-----|------|------|----------|----------|
| D1 | 红 | PA0 | PA0 → 限流电阻(330Ω~1kΩ) → LED阳极，LED阴极 → GND | 高电平 |
| D2 | 绿 | PB0 | 同上 | 高电平 |
| D3 | 蓝 | PC0 | 同上 | 高电平 |
| D4 | — | PC13 | 核心板板载（原理图：3.3V→1kΩ→LED→PC13） | 低电平 |

## 重要注意事项

- **时钟**：本工程使用复位后默认时钟 **HSI 8MHz**（AHB/APB 均不分频）。
  `main.c` 中的空 `SystemInit` 正是为了"拦住"启动文件对时钟的初始化。
  - 若你的模板自带 `system_stm32f10x.c`（72MHz 倍频），
    要么把它移出工程，要么把 `delay.c` 中 `SYSCLK_HZ` 改为 `72000000UL`。
  - 不要同时保留两个 `SystemInit` 定义，否则链接报重复定义错误。
- **PC13**：数据手册表3注4——PC13/PC14/PC15 经内部电源开关供电，
  输出模式最大 2MHz、负载最大 30pF，且三者同时只能有一个作输出。
  故本工程把 PC13 配置为 2MHz 推挽输出（`led.c` 中 `cfg=0x2`），
  这是官方要求，不属于 BUG。
- **板载灯电平**：板载 LED 低电平点亮（输出 0 灯亮），
  与面包板三只高电平点亮的灯恰好相反，驱动里已通过 `active_high` 字段统一处理。
