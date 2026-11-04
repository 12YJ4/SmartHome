#include "MainWindow.h"
#include "UdpService.h"
#include "FaceThread.h"
#include "DataBase.h"
#include "AlarmDialog.h"
#include <QAbstractItemView>
#include <QDateTime>
#include <QDialog>
#include <QGroupBox>
#include <QHeaderView>
#include <QHBoxLayout>
#include <QImage>
#include <QLabel>
#include <QLineEdit>
#include <QListWidget>
#include <QMessageBox>
#include <QPixmap>
#include <QPushButton>
#include <QSpinBox>
#include <QTableWidget>
#include <QVBoxLayout>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
{
    buildUi();

    // 1. 打开数据库（报警日志、家人名单）
    DataBase::instance()->open();

    // 2. 启动 UDP 监听 20000 端口
    m_udp = new UdpService(this);
    connect(m_udp, &UdpService::jsonReceived, this, &MainWindow::onJson);
    if (m_udp->start(20000))
        log("UDP 监听 20000 端口成功，等待下位机数据...");
    else
        log("UDP 端口 20000 绑定失败");

    // 3. 启动人脸识别子线程
    m_face = new FaceThread(this);
    connect(m_face, &FaceThread::frameReady, this, &MainWindow::onFrame);
    connect(m_face, &FaceThread::verifyResult, this, &MainWindow::onVerifyResult);
    connect(m_face, &FaceThread::enrolled, this, &MainWindow::onEnrolled);
    connect(m_face, &FaceThread::message, this, &MainWindow::log);
    m_face->start();

    // 4. 每秒检查一次阈值（做"停留超时"判定）
    connect(&m_checkTimer, &QTimer::timeout, this, &MainWindow::checkTimeout);
    m_checkTimer.start(1000);
}

MainWindow::~MainWindow()
{
    m_face->stop();
    DataBase::destroy();
}

void MainWindow::buildUi()
{
    setWindowTitle("智能家居安防监控中控台");
    resize(1100, 680);
    setStyleSheet(
        "QMainWindow{background:#1e2229;}"
        "QLabel{color:#d8dee6;}"
        "QGroupBox{color:#9fb0c0;border:1px solid #333a44;border-radius:6px;"
        "margin-top:8px;font-weight:bold;}"
        "QGroupBox::title{subcontrol-origin:margin;left:10px;}"
        "QPushButton{background:#2d3440;color:#d8dee6;border:1px solid #3d4654;"
        "border-radius:5px;padding:6px 12px;}"
        "QPushButton:hover{background:#3a4353;}"
        "QPushButton:checked{background:#1f7a4d;border-color:#2ecc71;}"
        "QSpinBox,QLineEdit{background:#262c36;color:#d8dee6;"
        "border:1px solid #3d4654;border-radius:4px;padding:4px;}"
        "QListWidget{background:#232830;color:#c3cbd5;border:1px solid #333a44;}");

    auto *central = new QWidget(this);
    setCentralWidget(central);
    auto *root = new QHBoxLayout(central);

    // ---------- 左边：监控画面 + 人脸录入 ----------
    auto *left = new QVBoxLayout;
    m_videoLabel = new QLabel("摄像头启动中...", central);
    m_videoLabel->setMinimumSize(640, 480);
    m_videoLabel->setAlignment(Qt::AlignCenter);
    m_videoLabel->setStyleSheet("background:#14171c;border-radius:6px;");
    left->addWidget(m_videoLabel);

    auto *enrollRow = new QHBoxLayout;
    m_nameEdit = new QLineEdit(central);
    m_nameEdit->setPlaceholderText("输入家人姓名，正对摄像头后点右侧按钮录入");
    auto *enrollBtn = new QPushButton("录入家人人脸", central);
    enrollRow->addWidget(m_nameEdit);
    enrollRow->addWidget(enrollBtn);
    left->addLayout(enrollRow);
    connect(enrollBtn, &QPushButton::clicked, this, &MainWindow::onEnrollClicked);
    root->addLayout(left, 3);

    // ---------- 右边：数据 / 控制 / 日志 ----------
    auto *right = new QVBoxLayout;
    root->addLayout(right, 2);

    auto *dataBox = new QGroupBox("环境数据", central);
    auto *dataLayout = new QHBoxLayout(dataBox);
    auto makeData = [&](const QString &name) {
        auto *box = new QVBoxLayout;
        auto *title = new QLabel(name);
        auto *value = new QLabel("--");
        value->setAlignment(Qt::AlignCenter);
        value->setStyleSheet("font-size:24px;font-weight:bold;color:#4fc3f7;");
        box->addWidget(title);
        box->addWidget(value);
        dataLayout->addLayout(box);
        return value;
    };
    m_tempLabel = makeData("温度(℃)");
    m_humiLabel = makeData("湿度(%)");
    m_distLabel = makeData("距离(cm)");
    right->addWidget(dataBox);

    auto *ctrlBox = new QGroupBox("设备联动", central);
    auto *ctrlLayout = new QHBoxLayout(ctrlBox);
    m_fanBtn = new QPushButton("风扇", central);
    m_relayBtn = new QPushButton("继电器", central);
    m_beepBtn = new QPushButton("蜂鸣器", central);
    m_lampBtn = new QPushButton("照明灯", central);
    for (QPushButton *btn : {m_fanBtn, m_relayBtn, m_beepBtn, m_lampBtn}) {
        btn->setCheckable(true);
        ctrlLayout->addWidget(btn);
    }
    // 一键下发控制命令
    connect(m_fanBtn, &QPushButton::toggled, this,
            [this](bool on) { sendCmd("fan", on); });
    connect(m_relayBtn, &QPushButton::toggled, this,
            [this](bool on) { sendCmd("relay", on); });
    connect(m_beepBtn, &QPushButton::toggled, this,
            [this](bool on) { sendCmd("beep", on); });
    connect(m_lampBtn, &QPushButton::toggled, this, [this](bool on) {
        QJsonObject obj;
        obj["cmd"] = "lamp";
        obj["r"] = on ? 255 : 0;
        obj["g"] = on ? 255 : 0;
        obj["b"] = on ? 255 : 0;
        m_udp->sendJson(obj);
        log(QString("下发: 照明灯 %1").arg(on ? "开" : "关"));
    });
    right->addWidget(ctrlBox);

    auto *thBox = new QGroupBox("报警阈值", central);
    auto *thLayout = new QHBoxLayout(thBox);
    thLayout->addWidget(new QLabel("距离小于"));
    m_distSpin = new QSpinBox(central);
    m_distSpin->setRange(5, 200);
    m_distSpin->setValue(25);
    m_distSpin->setSuffix(" cm");
    thLayout->addWidget(m_distSpin);
    thLayout->addWidget(new QLabel("温度大于"));
    m_tempSpin = new QSpinBox(central);
    m_tempSpin->setRange(20, 60);
    m_tempSpin->setValue(30);
    m_tempSpin->setSuffix(" ℃");
    thLayout->addWidget(m_tempSpin);
    auto *testBtn = new QPushButton("模拟报警(测试)", central);
    thLayout->addWidget(testBtn);
    connect(testBtn, &QPushButton::clicked, this,
            [this] { startVerify("模拟报警测试"); });
    right->addWidget(thBox);

    auto *logBox = new QGroupBox("报警 / 操作日志", central);
    auto *logLayout = new QVBoxLayout(logBox);
    auto *viewBtn = new QPushButton("查看报警记录(数据库)", logBox);
    logLayout->addWidget(viewBtn);
    connect(viewBtn, &QPushButton::clicked, this, [this] {
        // 弹出表格窗口，展示 alarm_log 表
        QDialog dlg(this);
        dlg.setWindowTitle("报警记录 (alarm_log)");
        dlg.resize(760, 400);
        auto *v = new QVBoxLayout(&dlg);
        auto *table = new QTableWidget(&dlg);
        table->setEditTriggers(QAbstractItemView::NoEditTriggers);
        table->setColumnCount(6);
        table->setHorizontalHeaderLabels({ "时间", "来源", "身份", "置信度", "抓拍图片", "结果" });
        table->horizontalHeader()->setStretchLastSection(true);
        const QList<QVariantMap> rows = DataBase::instance()->queryAlarms();
        table->setRowCount(rows.size());
        int r = 0;
        for (const QVariantMap &m : rows) {
            table->setItem(r, 0, new QTableWidgetItem(m.value("time").toString()));
            table->setItem(r, 1, new QTableWidgetItem(m.value("source").toString()));
            table->setItem(r, 2, new QTableWidgetItem(m.value("person").toString()));
            table->setItem(r, 3, new QTableWidgetItem(m.value("value").toString()));
            table->setItem(r, 4, new QTableWidgetItem(m.value("snapshot_path").toString()));
            table->setItem(r, 5, new QTableWidgetItem(m.value("result").toString()));
            ++r;
        }
        v->addWidget(table);
        dlg.exec();
    });
    m_logList = new QListWidget(logBox);
    logLayout->addWidget(m_logList);
    right->addWidget(logBox, 1);
}

// ---------------- 下位机数据处理 ----------------
void MainWindow::onJson(const QJsonObject &obj)
{
    const QString dev = obj["dev"].toString();

    if (dev == "temp_humi") {
        m_temperature = obj["temp"].toInt();
        m_tempLabel->setText(QString::number(m_temperature));
        m_humiLabel->setText(QString::number(obj["humi"].toInt()));
    } else if (dev == "ultrasonic") {
        m_distance = obj["dist"].toInt();
        m_distLabel->setText(QString::number(m_distance));
    } else if (obj["state"].isDouble()) {
        // 开关类设备的状态回传，仅记录日志
        log(QString("设备 %1 状态: %2").arg(dev).arg(obj["state"].toInt() ? "开" : "关"));
    }

    // 下位机自己判断出报警（alarm=1）时立即核验
    if (obj["alarm"].toInt() == 1)
        startVerify(QString("%1 触发报警").arg(dev));
}

// ---------------- 分级预警：停留超时判定 ----------------
void MainWindow::checkTimeout()
{
    // 温度超阈值：立即报警
    if (m_temperature > m_tempSpin->value())
        startVerify("温度超过阈值");

    // 有人靠近：连续超过 25 秒才判定为异常（避免快递员路过误报）
    if (m_distance > 0 && m_distance < m_distSpin->value()) {
        m_nearSeconds++;
        if (m_nearSeconds == 25) {
            startVerify("门口有人逗留超过 25 秒");
            m_nearSeconds = 0;
        }
    } else {
        m_nearSeconds = 0;
    }
}

void MainWindow::startVerify(const QString &reason)
{
    if (m_verifying)
        return;
    m_verifying = true;
    log("【报警】" + reason + "，开始人脸核验");
    m_face->requestVerify();
}

// ---------------- 核验结果 ----------------
void MainWindow::onVerifyResult(int label, double confidence, const QString &path)
{
    m_verifying = false;

    // LBPH 距离越小越像，这里约定 80 以内认为认识
    bool known = (label >= 0 && confidence >= 0 && confidence <= 80);
    QString name = "陌生人";
    if (label >= 0) {
        QString dbName = DataBase::instance()->userName(label);
        if (known && !dbName.isEmpty())
            name = dbName;
    }

    // 弹窗抓拍 + 用户确认
    QImage snap(path);
    AlarmDialog dlg(snap, known, name, confidence, this);
    dlg.exec();
    QString result = (dlg.result() == AlarmDialog::Accepted) ? "已确认为家人" : "标记为陌生人";

    // 写入数据库（含抓拍图片路径，保证可追溯）
    DataBase::instance()->insertAlarm(
        QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss"),
        "门口人脸核验", known ? 1 : 0, name, path, result);

    log(QString("核验结果: %1（置信度 %2） -> %3")
        .arg(name).arg(confidence, 0, 'f', 1).arg(result));
    m_logList->scrollToBottom();
}

// ---------------- 人脸录入 ----------------
void MainWindow::onEnrollClicked()
{
    QString name = m_nameEdit->text().trimmed();
    if (name.isEmpty()) {
        QMessageBox::information(this, "提示", "请先输入姓名");
        return;
    }
    int label = DataBase::instance()->insertUser(name);
    m_face->requestEnroll(label, name);
}

void MainWindow::onEnrolled(int, const QString &name)
{
    log(QString("已录入家人人脸: %1").arg(name));
    m_nameEdit->clear();
}

// ---------------- 其它 ----------------
void MainWindow::onFrame(const QImage &img)
{
    m_videoLabel->setPixmap(QPixmap::fromImage(img).scaled(
        m_videoLabel->size(), Qt::KeepAspectRatio, Qt::SmoothTransformation));
}

void MainWindow::sendCmd(const QString &cmd, int val)
{
    QJsonObject obj;
    obj["cmd"] = cmd;
    obj["val"] = val;
    m_udp->sendJson(obj);
    log(QString("下发: %1 %2").arg(cmd, val ? "开" : "关"));
}

void MainWindow::log(const QString &text)
{
    m_logList->addItem(QDateTime::currentDateTime().toString("hh:mm:ss ") + text);
    m_logList->scrollToBottom();
}
