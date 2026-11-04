# 智能家居安防监控与设备联动系统

基于 STM32 + ESP8266 + Qt + OpenCV 的智能家居系统，包含下位机（数据采集/执行）与上位机（监控中控台）两部分。

## 项目结构

```
SmartHome/
├─ STM32_LowerComputer/   # 下位机：Keil5 + STM32G030 + ESP8266
└─ Qt_UpperComputer/      # 上位机：Qt5 + OpenCV 人脸识别中控台
```

## 功能

1. 温湿度 / 超声波距离采集，ESP8266 通过 UDP 上报 JSON 报文
2. Qt 中控台实时显示环境数据，支持一键下发风扇/继电器/蜂鸣器/RGB 灯控制
3. 分级预警 + 超时判定（距离 < 阈值持续 25 秒、温度超阈值）
4. OpenCV 人脸核验：报警触发后抓拍 + LBPH 识别 + 弹窗用户确认
5. 人脸识别部署在独立子线程，界面流畅
6. SQLite 记录报警日志与陌生人抓拍图片路径

## 说明

- 上位机详细运行说明见 `Qt_UpperComputer/README.md`
- 下位机通信协议见 `STM32_LowerComputer/Src/bsp_esp8266.c`（JSON，UDP 20000 端口）

## 演示截图

（在此处贴上前端界面、报警弹窗、日志、数据库等截图）
