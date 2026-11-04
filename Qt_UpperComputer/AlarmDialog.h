#ifndef ALARMDIALOG_H
#define ALARMDIALOG_H

#include <QDialog>
#include <QImage>

class QLabel;

// 报警核验弹窗：显示抓拍图片 + 识别结果，由用户确认身份
class AlarmDialog : public QDialog
{
    Q_OBJECT
public:
    enum Result { Accepted, Stranger };

    AlarmDialog(const QImage &snapshot, bool known, const QString &name,
                double confidence, QWidget *parent = nullptr);

    Result result() const { return m_result; }

private:
    QLabel *m_imageLabel;
    Result m_result = Stranger;
};

#endif // ALARMDIALOG_H
