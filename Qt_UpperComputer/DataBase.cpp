#include "DataBase.h"
#include <QSqlDatabase>
#include <QSqlQuery>
#include <QSqlError>
#include <QSqlRecord>
#include <QVariant>
#include <QDebug>

DataBase *DataBase::m_instance = nullptr;

DataBase *DataBase::instance()
{
    if (!m_instance)
        m_instance = new DataBase;
    return m_instance;
}

void DataBase::destroy()
{
    if (m_instance) {
        m_instance->close();
        delete m_instance;
        m_instance = nullptr;
    }
}

bool DataBase::open(const QString &dbName)
{
    QSqlDatabase db = QSqlDatabase::addDatabase("QSQLITE");
    db.setDatabaseName(dbName);
    if (!db.open()) {
        qDebug() << "数据库打开失败:" << db.lastError().text();
        return false;
    }

    QSqlQuery query(db);
    query.exec("CREATE TABLE IF NOT EXISTS users("
               "id INTEGER PRIMARY KEY AUTOINCREMENT,"
               "name TEXT NOT NULL)");

    query.exec("CREATE TABLE IF NOT EXISTS alarm_log("
               "id INTEGER PRIMARY KEY AUTOINCREMENT,"
               "time TEXT, source TEXT, value INTEGER,"
               "person TEXT, snapshot_path TEXT, result TEXT)");
    return true;
}

void DataBase::close()
{
    QString name = QSqlDatabase::database().connectionName();
    if (!name.isEmpty())
        QSqlDatabase::removeDatabase(name);
}

int DataBase::insertUser(const QString &name)
{
    QSqlQuery query;
    query.prepare("INSERT INTO users(name) VALUES(?)");
    query.addBindValue(name);
    if (!query.exec())
        return -1;
    return query.lastInsertId().toInt();
}

QString DataBase::userName(int id)
{
    QSqlQuery query;
    query.prepare("SELECT name FROM users WHERE id=?");
    query.addBindValue(id);
    if (query.exec() && query.next())
        return query.value(0).toString();
    return QString();
}

bool DataBase::insertAlarm(const QString &time, const QString &source, int value,
                           const QString &person, const QString &snapshotPath,
                           const QString &result)
{
    QSqlQuery query;
    query.prepare("INSERT INTO alarm_log(time, source, value, person, snapshot_path, result) "
                  "VALUES(?,?,?,?,?,?)");
    query.addBindValue(time);
    query.addBindValue(source);
    query.addBindValue(value);
    query.addBindValue(person);
    query.addBindValue(snapshotPath);
    query.addBindValue(result);
    return query.exec();
}

QList<QVariantMap> DataBase::queryAlarms(int limit)
{
    QList<QVariantMap> list;
    QSqlQuery query;
    query.prepare(QString("SELECT * FROM alarm_log ORDER BY id DESC LIMIT %1").arg(limit));
    if (!query.exec())
        return list;
    while (query.next()) {
        QVariantMap row;
        const QSqlRecord rec = query.record();
        for (int i = 0; i < rec.count(); ++i)
            row.insert(rec.fieldName(i), query.value(i));
        list.append(row);
    }
    return list;
}
