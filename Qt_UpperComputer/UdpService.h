#ifndef UDPSERVICE_H
#define UDPSERVICE_H

#include <QObject>
#include <QUdpSocket>
#include <QJsonObject>

// UDP 通信：接收下位机 ESP8266 上报的 JSON，向上位机广播下发控制命令
class UdpService : public QObject
{
    Q_OBJECT
public:
    explicit UdpService(QObject *parent = nullptr);

    bool start(quint16 port = 20000);            // 绑定端口，开始监听
    void sendJson(const QJsonObject &obj);        // 广播一条 JSON 命令

signals:
    void jsonReceived(const QJsonObject &obj);    // 收到一条合法的 JSON
    void rawReceived(const QString &text);        // 原始文本（方便调试）

private slots:
    void onReadyRead();

private:
    QUdpSocket m_socket;
};

#endif // UDPSERVICE_H
