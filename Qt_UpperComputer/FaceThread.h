#ifndef FACETHREAD_H
#define FACETHREAD_H

#include <QThread>
#include <QMutex>
#include <QString>
#include <QImage>
#include <opencv2/opencv.hpp>
#include <opencv2/face.hpp>

// OpenCV 人脸识别线程：摄像头采集 + Haar 检测 + LBPH 识别
// 放在子线程里跑，避免 Qt 界面卡顿（主线程只负责显示和业务逻辑）
class FaceThread : public QThread
{
    Q_OBJECT
public:
    explicit FaceThread(QObject *parent = nullptr);
    ~FaceThread() override;

    void requestEnroll(int label, const QString &name); // 录入一张人脸并训练
    void requestVerify();                               // 报警时抓拍 + 识别
    void stop();                                        // 结束线程

signals:
    void frameReady(const QImage &img);                              // 摄像头画面
    void verifyResult(int label, double confidence, const QString &path); // 识别结果
    void enrolled(int label, const QString &name);                   // 录入完成
    void message(const QString &text);                               // 状态提示

protected:
    void run() override;

private:
    cv::Mat cropFace(const cv::Mat &frame,
                     const std::vector<cv::Rect> &faces);  // 截取人脸

    cv::VideoCapture m_cap;
    cv::CascadeClassifier m_cascade;
    cv::Ptr<cv::face::LBPHFaceRecognizer> m_model;

    QMutex m_mutex;
    int m_enrollLabel = -1;
    QString m_enrollName;
    bool m_verifyFlag = false;
    bool m_stopFlag = false;
};

#endif // FACETHREAD_H
