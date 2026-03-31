# 软件设计结构


---

## 目录详解

ECU 风格（车载项目更贴近实际）<br>
特点：借鉴 AUTOSAR 风格，强调功能解耦

### 1. `platform/` - 平台层
硬件抽象层，包含所有与硬件直接相关的代码，确保应用层与硬件解耦。

- **`core/`**: CMSIS 核心文件
  - `core_cm4.h/.c`: Cortex-M4 内核外设访问层
  - `startup_stm32f407xx.s`: 芯片启动文件 (汇编)
  - `system_stm32f4xx.h/.c`: 系统时钟初始化 (通常也可放在 `user/`)

- **`fwlib/`**: STM32F4xx 标准外设库 (StdPeriph Driver)
  - `inc/`: 外设头文件 (如 `stm32f4xx_gpio.h`, `stm32f4xx_tim.h`)
  - `src/`: 外设源文件 (如 `stm32f4xx_gpio.c`, `stm32f4xx_tim.c`)
  - *注：如果使用 HAL/LL 库，此目录可替换为 `hal/` 或 `ll/`*

- **`bsp/`**: 板级支持包 (Board Support Package)
  - `bsp_motor.c/.h`: 电机驱动 (BTS7960) 的初始化与控制函数
  - `bsp_lcd.c/.h`: LCD 触摸屏的初始化与底层读写函数
  - `bsp_key.c/.h`: 按键扫描
  - `bsp_adc.c/.h`: ADC 采样配置 (用于电压、电流检测)
  - `bsp_can.c/.h`: CAN 控制器初始化与收发 (若使用)
  - `board.c/.h`: 板级硬件初始化汇总 (时钟、GPIO、外设)

### 2. `services/` - 系统服务层
为应用程序提供通用的、与硬件无关的系统服务。

- **`os/`**: 操作系统抽象层
  - `cmsis_os.h` 适配层 (如果使用 CMSIS-RTOS API)
  - 任务 (Task)、信号量 (Semaphore)、队列 (Queue)、事件组 (Event Groups) 的创建与管理
  - 系统滴答定时器 (SysTick) 钩子函数

- **`storage/`**: 存储管理
  - `nvs.c/.h`: 非易失存储管理，基于内部 Flash 或外部 EEPROM
  - 实现参数的读写、擦除、校验等功能

- **`update/`**: OTA 升级服务
  - `bootloader_com.c/.h`: 与 Bootloader 的通信协议 (如自定义串口协议)
  - `fw_update.c/.h`: 固件接收、校验、存储管理 (在 Application 中运行)

- **`diagnostics/`**: 诊断服务
  - `diagnostics.c/.h`: 系统状态监控 (电压、温度、错误码)
  - `fault_handler.c/.h`: 故障处理与安全状态管理

### 3. `application/` - 应用层
实现产品的具体业务逻辑，依赖下层提供的服务。

- **`motor_ctrl/`**: 电机控制核心
  - `motor.c/.h`: 电机对象封装 (状态、目标速度、电流)
  - `pid.c/.h`: PID 控制算法实现 (速度环、电流环)
  - `encoder.c/.h`: 编码器读数与速度计算

- **`hmi/`**: 人机交互
  - `lcd_ui.c/.h`: LCD 界面绘制、菜单逻辑
  - `touch.c/.h`: 触摸事件处理
  - `menu_config.c/.h`: 菜单项与参数配置逻辑

- **`vehicle_logic/`**: 车辆应用逻辑
  - `vehicle_state.c/.h`: 车辆状态机 (运行、停止、错误等模式)
  - `param_manager.c/.h`: 车辆参数管理 (与 `storage` 服务交互)

### 4. `bootloader/` (可选独立仓库或目录)
Bootloader 独立项目，负责应用程序的验证与跳转。
- `bootmain.c`: 主逻辑
- `ymodem.c/.h` 或 `custom_protocol.c/.h`: 传输协议
- `flash_if.c/.h`: Flash 擦写操作

### 5. `config/` - 配置文件
集中管理项目的各类配置，避免魔数 (Magic Number) 散落在代码中。

- **`FreeRTOSConfig.h`**: FreeRTOS 内核配置 (任务优先级、堆栈大小、功能裁剪)
- **`partition_table.ld`**: 链接脚本，定义 Flash 和 RAM 的分区 (Bootloader/App 区域、NVS 区域)
- **`parameters.h`**: 系统参数宏定义 (如电机 PID 参数、保护阈值、引脚映射)

### 6. 其他重要目录

- **`tools/`**: 开发辅助工具
  - 串口调试助手脚本
  - OTA 固件打包工具
  - CAN 总线分析脚本

- **`build/`**: 编译输出目录 (通常被 `.gitignore` 忽略)
  - `obj/`: 中间目标文件
  - `list/`: 链接映射文件
  - `bin/`: 生成的二进制文件 (.bin, .hex)

- **`docs/`**: 项目文档
  - 硬件原理图
  - 软件设计说明
  - API 参考
  - 测试报告

- **`user/`**: 应用入口与芯片级代码
  - `main.c`: 主函数，系统初始化、创建主任务
  - `stm32f4xx_it.c/.h`: 中断服务函数
  - `system_stm32f4xx.c`: 系统时钟配置 (HSE, PLL)

## 设计理念

1.  **分层架构**: 下层为上层提供服务，上层不关心下层的具体实现，降低耦合度。
2.  **模块化**: 每个功能模块职责单一，便于独立开发、测试和维护。
3.  **可移植性**: 通过 `platform/` 和 `bsp/` 层抽象硬件，更换 MCU 或开发板时，只需修改底层代码。
4.  **可配置性**: 所有配置集中于 `config/` 目录，方便根据不同需求进行裁剪和调整。

此结构为复杂嵌入式系统提供了一个清晰、健壮的框架。