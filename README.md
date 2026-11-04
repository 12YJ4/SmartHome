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
<img width="1280" height="566" alt="qq_pic_merged_1793791055185" src="https://github.com/user-attachments/assets/43b5354f-6769-4ecf-af3b-1f1f7bd51000" />
图1 显示温湿度
<img width="848" height="614" alt="qq_pic_merged_1793791090609" src="https://github.com/user-attachments/assets/074cf04a-7483-4b53-9bad-b6c2f42e4e4b" />
图2 录入人员信息
<img width="1280" height="455" alt="qq_pic_merged_1793791116494" src="https://github.com/user-attachments/assets/4cce1862-55cf-4c03-84b0-afd14b8ca707" />
图3 防止外卖员/快递员误报
<img width="1078" height="562" alt="qq_pic_merged_1793791149455" src="https://github.com/user-attachments/assets/2383e0dc-aa06-419c-bc3a-db7af20ccf0f" />
图4 远程联调
<img width="1280" height="427" alt="Cache_-229d0293c5cb5f5f" src="https://github.com/user-attachments/assets/6129af43-dea4-41e4-b3cc-358cd4d68d0c" />
图5 数据持久化




