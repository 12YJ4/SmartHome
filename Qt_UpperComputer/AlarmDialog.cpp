#include "AlarmDialog.h"
#include <QLabel>
#include <QPixmap>
#include <QPushButton>
#include <QVBoxLayout>

AlarmDialog::AlarmDialog(const QImage &snapshot, bool known, const QString &name,
                         double confidence, QWidget *parent)
    : QDialog(parent)
{
    setWindowTitle("安防报警 - 人脸核验");
    setModal(true);
    resize(400, 520);

    auto *layout = new QVBoxLayout(this);

    m_imageLabel = new QLabel(this);
    m_imageLabel->setMinimumSize(360, 320);
    m_imageLabel->setAlignment(Qt::AlignCenter);
    m_imageLabel->setStyleSheet("border:1px solid #444;background:#222;");
    if (!snapshot.isNull())
        m_imageLabel->setPixmap(QPixmap::fromImage(snapshot).scaled(
            m_imageLabel->size(), Qt::KeepAspectRatio, Qt::SmoothTransformation));
    layout->addWidget(m_imageLabel);

    auto *text = new QLabel(this);
    text->setWordWrap(true);
    text->setAlignment(Qt::AlignCenter);
    if (confidence < 0)
        text->setText("画面中未检测到人脸\n请确认来访者身份：");
    else if (known)
        text->setText(QString("识别结果：%1（置信度 %2）\n请确认来访者身份：")
                      .arg(name).arg(confidence, 0, 'f', 1));
    else
        text->setText(QString("识别结果：陌生人（置信度 %1）\n请确认来访者身份：")
                      .arg(confidence, 0, 'f', 1));
    layout->addWidget(text);

    auto *okBtn = new QPushButton("确认是家人", this);
    auto *strangerBtn = new QPushButton("标记为陌生人", this);
    strangerBtn->setStyleSheet("background:#c0392b;color:white;padding:6px;");
    layout->addWidget(okBtn);
    layout->addWidget(strangerBtn);

    connect(okBtn, &QPushButton::clicked, this, [this] {
        m_result = Accepted;
        accept();
    });
    connect(strangerBtn, &QPushButton::clicked, this, [this] {
        m_result = Stranger;
        accept();
    });
}
