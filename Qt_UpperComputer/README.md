# 智能家居安防监控与设备联动系统（Qt 上位机）

配合 Keil5 下位机工程（`d:/code/smart_home`）使用。上位机负责：数据展示、设备控制、报警判定、OpenCV 人脸核验、日志记录。

## 一、功能

1. **环境数据展示**：通过 UDP 接收下位机 JSON 报文，实时显示温度、湿度、门口距离。
2. **设备联动控制**：一键下发风扇 / 继电器 / 蜂鸣器 / RGB 灯控制命令。
3. **分级预警 + 超时判定**：门口有人靠近连续超过 25 秒（或温度超过阈值）才判定为异常。
4. **OpenCV 人脸核验**：报警触发后抓拍一帧，Haar 检测 + LBPH 比对，弹窗由用户确认身份。
5. **独立子线程**：摄像头采集、检测、识别全部在 `FaceThread` 子线程执行，界面不卡顿。
6. **SQLite 记录**：家人名单、报警日志、陌生人抓拍图片路径入库。

## 二、文件说明

| 文件 | 作用 |
| --- | --- |
| `main.cpp` | 程序入口 |
| `MainWindow.h/.cpp` | 主界面：数据、控制、报警判定、核验弹窗调度 |
| `UdpService.h/.cpp` | UDP 收发（绑定 20000 端口，JSON 解析 / 广播下发） |
| `FaceThread.h/.cpp` | 人脸识别子线程：采集 / Haar 检测 / LBPH 训练与识别 / 抓拍 |
| `AlarmDialog.h/.cpp` | 报警核验弹窗（抓拍图片 + 识别结果 + 用户确认） |
| `DataBase.h/.cpp` | SQLite 单例：家人名单 + 报警日志 |

## 三、通信协议（JSON，UDP 20000 端口）

下位机 ESP8266 工作于透传 + UDP 广播模式。

上报（下位机 -> 上位机）：
```json
{"dev":"temp_humi","temp":26,"humi":58,"alarm":0}
{"dev":"ultrasonic","dist":120,"alarm":0}
{"dev":"fan","state":1}
{"dev":"lamp","r":255,"g":0,"b":0}
```
下发（上位机 -> 下位机）：
```json
{"cmd":"fan","val":1}
{"cmd":"relay","val":0}
{"cmd":"beep","val":1}
{"cmd":"lamp","r":255,"g":0,"b":0}
```

## 四、运行步骤

1. 确保安装了 Qt5 + OpenCV 3.4.5（含 opencv_contrib/face 模块）。
2. 修改 `SmartHomeQt.pro` 里的 OpenCV 路径为你的实际安装路径：
   ```
   INCLUDEPATH += D:\worktools\opencv\opencv3.4.5_install\include
   LIBS += D:\worktools\opencv\opencv3.4.5_install\x64\mingw\lib\libopencv_*.a
   ```
3. 把 `haarcascade_frontalface_alt2.xml` 从 OpenCV 安装目录复制到程序运行目录（exe 同级）。
4. 用 Qt Creator 打开 `SmartHomeQt.pro` 编译运行。
5. 首次录入：在“输入家人姓名”处输入姓名，正对摄像头点“录入家人人脸”。
6. 下位机上电后会自动上报温湿度 / 距离；触发报警时会弹出人脸核验窗口。

## 五、数据库表

- `users(id, name)`：家人名单，id 即 LBPH 的 label。
- `alarm_log(id, time, source, value, person, snapshot_path, result)`：报警记录，含抓拍图片路径。

## 六、下位机对应改动（`d:/code/smart_home`）

- `Src/bsp_esp8266.c`：上报由二进制帧改为 JSON（`packet_json`），命令解析改为 `analysis_json`，并本地判断距离 / 温度阈值写入 `alarm` 字段。
- `Src/main.c`：主函数改为「外设初始化 -> BSP_Init 识别板卡 -> ESP8266_Init -> 主循环 ESP_TASK/SEND_TASK」。
