#include "UdpService.h"
#include <QJsonDocument>
#include <QNetworkDatagram>
#include <QDebug>

UdpService::UdpService(QObject *parent) : QObject(parent)
{
    connect(&m_socket, &QUdpSocket::readyRead, this, &UdpService::onReadyRead);
}

bool UdpService::start(quint16 port)
{
    // 下位机是 UDP 广播，允许端口复用
    bool ok = m_socket.bind(QHostAddress::AnyIPv4, port,
                            QUdpSocket::ShareAddress | QUdpSocket::ReuseAddressHint);
    if (!ok)
        qDebug() << "UDP 绑定失败:" << port;
    return ok;
}

void UdpService::onReadyRead()
{
    while (m_socket.hasPendingDatagrams()) {
        QByteArray data = m_socket.receiveDatagram().data();
        QString text = QString::fromUtf8(data).trimmed();
        if (text.isEmpty())
            continue;

        emit rawReceived(text);

        // 解析 JSON，解析失败就直接丢弃
        QJsonParseError err;
        QJsonDocument doc = QJsonDocument::fromJson(text.toUtf8(), &err);
        if (err.error != QJsonParseError::NoError || !doc.isObject()) {
            qDebug() << "JSON 解析失败:" << text;
            continue;
        }
        emit jsonReceived(doc.object());
    }
}

void UdpService::sendJson(const QJsonObject &obj)
{
    QByteArray data = QJsonDocument(obj).toJson(QJsonDocument::Compact);
    // 下位机 ESP8266 配的是 UDP 广播 255.255.255.255:20000
    m_socket.writeDatagram(data, QHostAddress::Broadcast, 20000);
    // 调试回显：额外发一份到本机 20001，方便用网络调试助手监听查看下发内容
    m_socket.writeDatagram(data, QHostAddress::LocalHost, 20001);
}
