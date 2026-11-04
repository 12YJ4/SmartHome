#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QJsonObject>
#include <QTimer>

class QLabel;
class QLineEdit;
class QListWidget;
class QPushButton;
class QSpinBox;
class UdpService;
class FaceThread;

// 智能家居上位机主界面：环境数据显示 + 设备控制 + 报警人脸核验
class MainWindow : public QMainWindow
{
    Q_OBJECT
public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow() override;

private slots:
    void onJson(const QJsonObject &obj);        // 处理下位机上报
    void onFrame(const QImage &img);            // 显示摄像头画面
    void onVerifyResult(int label, double confidence, const QString &path);
    void onEnrollClicked();                     // 录入家人人脸
    void onEnrolled(int label, const QString &name);
    void checkTimeout();                        // 每秒检查阈值

private:
    void buildUi();
    void log(const QString &text);
    void startVerify(const QString &reason);
    void sendCmd(const QString &cmd, int val);  // 下发开关命令

    // 界面控件
    QLabel *m_videoLabel;
    QLabel *m_tempLabel;
    QLabel *m_humiLabel;
    QLabel *m_distLabel;
    QLineEdit *m_nameEdit;
    QSpinBox *m_distSpin;
    QSpinBox *m_tempSpin;
    QListWidget *m_logList;
    QPushButton *m_fanBtn;
    QPushButton *m_relayBtn;
    QPushButton *m_beepBtn;
    QPushButton *m_lampBtn;

    // 业务数据
    UdpService *m_udp;
    FaceThread *m_face;
    QTimer m_checkTimer;
    int m_distance = -1;        // 最近一次门口距离
    int m_temperature = 0;      // 最近一次温度
    int m_nearSeconds = 0;      // 连续有人逗留的秒数
    bool m_verifying = false;   // 正在核验，避免重复弹窗
};

#endif // MAINWINDOW_H
