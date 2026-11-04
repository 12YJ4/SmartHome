#include "FaceThread.h"
#include <QDateTime>
#include <QDir>
#include <QFile>

#define MODEL_PATH   "face.xml"
#define SNAPSHOT_DIR "snapshots"

FaceThread::FaceThread(QObject *parent) : QThread(parent)
{
}

FaceThread::~FaceThread()
{
    stop();
}

void FaceThread::requestEnroll(int label, const QString &name)
{
    QMutexLocker locker(&m_mutex);
    m_enrollLabel = label;
    m_enrollName = name;
}

void FaceThread::requestVerify()
{
    QMutexLocker locker(&m_mutex);
    m_verifyFlag = true;
}

void FaceThread::stop()
{
    m_stopFlag = true;
    wait(1000);
}

cv::Mat FaceThread::cropFace(const cv::Mat &frame, const std::vector<cv::Rect> &faces)
{
    if (faces.size() != 1)          // 必须只有一张脸才处理
        return cv::Mat();
    cv::Mat face = frame(faces[0]).clone();
    cv::cvtColor(face, face, cv::COLOR_BGR2GRAY);
    cv::resize(face, face, cv::Size(80, 80));   // 统一尺寸
    return face;
}

void FaceThread::run()
{
    // 1. 打开摄像头
    m_cap.open(0);
    if (!m_cap.isOpened()) {
        emit message("摄像头打开失败");
        return;
    }

    // 2. 加载 Haar 级联分类器：先找运行目录，再找 OpenCV 安装目录
    bool cascadeOK = m_cascade.load("haarcascade_frontalface_alt2.xml");
    if (!cascadeOK) {
        const QStringList candidates = {
            "D:/worktools/opencv/opencv3.4.5_install/sources/data/haarcascades/haarcascade_frontalface_alt2.xml",
            "C:/opencv/sources/data/haarcascades/haarcascade_frontalface_alt2.xml"
        };
        for (const QString &path : candidates) {
            if (m_cascade.load(path.toLocal8Bit().toStdString())) {
                cascadeOK = true;
                break;
            }
        }
    }
    if (!cascadeOK)
        emit message("缺少 haarcascade_frontalface_alt2.xml，仅显示画面，无法人脸检测/识别");

    // 3. 加载 / 创建 LBPH 模型
    if (QFile::exists(MODEL_PATH))
        m_model = cv::face::FaceRecognizer::load<cv::face::LBPHFaceRecognizer>(MODEL_PATH);
    else
        m_model = cv::face::LBPHFaceRecognizer::create();
    m_model->setThreshold(100);

    QDir().mkpath(SNAPSHOT_DIR);

    // 4. 循环采集 + 检测
    while (!m_stopFlag) {
        cv::Mat frame;
        m_cap >> frame;
        if (frame.empty()) {
            msleep(30);
            continue;
        }
        cv::flip(frame, frame, 1);      // 镜像，符合人的直觉

        cv::Mat gray;
        std::vector<cv::Rect> faces;
        if (!m_cascade.empty()) {
            cv::cvtColor(frame, gray, cv::COLOR_BGR2GRAY);
            m_cascade.detectMultiScale(gray, faces, 1.1, 3, 0, cv::Size(80, 80));
            for (const cv::Rect &r : faces)
                cv::rectangle(frame, r, cv::Scalar(0, 0, 255), 2);
        }

        // 4.1 把画面转成 QImage 发给主线程显示
        cv::Mat rgb;
        cv::cvtColor(frame, rgb, cv::COLOR_BGR2RGB);
        emit frameReady(QImage(rgb.data, rgb.cols, rgb.rows,
                               rgb.cols * rgb.channels(),
                               QImage::Format_RGB888).copy());

        // 4.2 处理录入请求
        int enrollLabel = -1;
        QString enrollName;
        {
            QMutexLocker locker(&m_mutex);
            if (m_enrollLabel >= 0) {
                enrollLabel = m_enrollLabel;
                enrollName = m_enrollName;
                m_enrollLabel = -1;
                m_enrollName.clear();
            }
        }
        if (enrollLabel >= 0) {
            cv::Mat face = cropFace(frame, faces);
            if (face.empty()) {
                emit message("录入失败：请保证画面中只有一个人脸");
            } else {
                std::vector<cv::Mat> images{face};
                std::vector<int> labels{enrollLabel};
                m_model->update(images, labels);     // 训练
                m_model->save(MODEL_PATH);           // 保存模型
                emit enrolled(enrollLabel, enrollName);
            }
        }

        // 4.3 处理报警核验请求：抓拍 + 识别
        bool needVerify = false;
        {
            QMutexLocker locker(&m_mutex);
            if (m_verifyFlag) {
                m_verifyFlag = false;
                needVerify = true;
            }
        }
        if (needVerify) {
            QString path = QString("%1/%2.jpg").arg(SNAPSHOT_DIR,
                    QDateTime::currentDateTime().toString("yyyyMMdd_hhmmss"));
            cv::imwrite(path.toLocal8Bit().toStdString(), frame);  // 抓拍存图

            cv::Mat face = cropFace(frame, faces);
            if (face.empty()) {
                emit verifyResult(-1, -1, path);   // 没检测到人脸
            } else {
                int label = -1;
                double confidence = 0;
                m_model->predict(face, label, confidence);
                emit verifyResult(label, confidence, path);
            }
        }

        msleep(50);   // 约 20fps，别占满 CPU
    }

    m_cap.release();
}
