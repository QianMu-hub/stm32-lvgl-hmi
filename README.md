STM32 LVGL HMI
基于 STM32 与 LVGL 图形库的嵌入式人机交互界面项目。

简介
本项目是一个运行在 STM32 微控制器上的 HMI 工程，使用 LVGL 作为图形库。项目包含完整的 BSP 驱动、LVGL 移植适配层以及用户 UI 逻辑，适用于嵌入式设备上的图形界面开发与学习。

功能特性
基于 LVGL 的图形用户界面，支持红外遥控交互

已完成显示驱动与输入驱动的移植适配（Port/lv_port_disp.c、Port/lv_port_indev.c）

已完成相关硬件驱动

提供最小化 UI 实现、网络连接 、 红外交互

支持日期时间戳转换（User/date_to_timestamp.c）

提供城市配置模块（User/city_config.c）

硬件要求
STM32F407xx核心板

ESP-01s

TFT-LCD 显示屏（st7789主控）

红外接收模块及红外遥控器

调试器（ST-Link / J-Link 等）

软件依赖

MDK-ARM（Keil）或 vscode 项目开发环境

快速开始
1. 获取源码
git clone https://github.com/QianMu-hub/stm32-lvgl-hmi.git

2. 打开工程
使用 MDK-ARM（Keil）打开 MDK-ARM 目录下的工程文件，或使用vscode插件导入项目。

3. 编译与烧录
自行完成api key的宏定义并导入，在 IDE 中编译工程，通过调试器将固件烧录至 STM32 开发板。

目录结构
.
├── BSP/              # 板级支持包
├── Core/             # STM32cubemx 创建的部分核心代码
├── Drivers/          # STM32 HAL 驱动库
├── MDK-ARM/          # Keil 工程文件
├── Middlewares/      # LVGL 图形库及freertos库文件
├── Port/             # LVGL 移植适配层
│   ├── lv_port_disp.c    # 显示驱动移植
│   ├── lv_port_disp.h
│   ├── lv_port_indev.c   # 输入驱动移植
│   └── lv_port_indev.h
├── User/             # 用户应用代码
│   ├── app_mem_layout.h        # 内存布局定义
│   ├── city_config.c/.h        # 城市配置
│   ├── date_to_timestamp.c/.h  # 日期转换
│   ├── json_analysis.c/.h      # JSON 解析
│   ├── ui_ir.c/.h              # UI 界面
│   └── user_main.c             # 程序入口 
└── .gitignore
