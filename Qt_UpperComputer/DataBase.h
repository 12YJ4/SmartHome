#ifndef DATABASE_H
#define DATABASE_H

#include <QString>
#include <QList>
#include <QVariantMap>

// SQLite 数据库：保存家人名单和报警记录（含抓拍图片路径）
class DataBase
{
public:
    static DataBase *instance();     // 单例
    static void destroy();

    bool open(const QString &dbName = "smarthome.db");
    void close();

    int insertUser(const QString &name);            // 新增家人，返回 id（也就是人脸 label）
    QString userName(int id);                       // 根据 label 查姓名

    bool insertAlarm(const QString &time, const QString &source, int value,
                     const QString &person, const QString &snapshotPath,
                     const QString &result);

    QList<QVariantMap> queryAlarms(int limit = 200);  // 查询报警记录

private:
    DataBase() = default;
    static DataBase *m_instance;
};

#endif // DATABASE_H
