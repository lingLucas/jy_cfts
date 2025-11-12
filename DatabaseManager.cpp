#include "DatabaseManager.h"
#include <QSqlRecord>
#include <QDebug>
#include <QUuid>

DatabaseManager::DatabaseManager(const QString& path, QObject *parent)
    : QObject{parent}
{
    // 为每个数据库实例创建一个唯一的连接名
    m_connectionName = generateConnectionName();
    m_db = QSqlDatabase::addDatabase("QSQLITE", m_connectionName);
    m_db.setDatabaseName(path);
}


DatabaseManager::~DatabaseManager()
{
    if (m_db.isOpen()) {
        m_db.close();
    }
    // 析构时必须移除数据库连接，否则会内存泄漏
    QSqlDatabase::removeDatabase(m_connectionName);
}

QString DatabaseManager::generateConnectionName()
{
    // 使用UUID确保连接名绝对唯一
    return QUuid::createUuid().toString(QUuid::WithoutBraces);
}

bool DatabaseManager::open()
{
    if (m_db.isOpen()) {
        return true;
    }
    if (!m_db.open()) {
        m_lastError = m_db.lastError().text();
        return false;
    }
    return true;
}

void DatabaseManager::close()
{
    if (m_db.isOpen()) {
        m_db.close();
    }
}

QString DatabaseManager::lastError() const
{
    return m_lastError;
}

bool DatabaseManager::exec(const QString& sql)
{
    if (!open()) {
        return false;
    }

    QSqlQuery query(m_db);
    if (!query.exec(sql)) {
        m_lastError = query.lastError().text();
        qWarning() << "SQL Error:" << m_lastError;
        qWarning() << "Failed SQL:" << sql;
        return false;
    }
    return true;
}

bool DatabaseManager::insert(const QString& table, const QVariantMap& data)
{
    if (data.isEmpty()) {
        m_lastError = "Insert data is empty.";
        return false;
    }

    QStringList columns = data.keys();
    QStringList placeholders;
    for (int i = 0; i < columns.size(); ++i) {
        placeholders << "?";
    }

    QString sql = QString("INSERT INTO %1 (%2) VALUES (%3)")
                      .arg(table, columns.join(", "), placeholders.join(", "));

    if (!open()) {
        return false;
    }

    QSqlQuery query(m_db);
    query.prepare(sql);

    for (const auto& key : std::as_const(columns)) {
        query.addBindValue(data.value(key));
    }

    if (!query.exec()) {
        m_lastError = query.lastError().text();
        qWarning() << "Insert Error:" << m_lastError;
        return false;
    }
    return true;
}

bool DatabaseManager::update(const QString& table, const QVariantMap& data, const QString& condition)
{
    if (data.isEmpty()) {
        m_lastError = "Update data is empty.";
        return false;
    }
    if (condition.isEmpty()) {
        m_lastError = "Update condition is empty, this is dangerous.";
        return false;
    }

    QStringList setParts;
    for (const auto& key : data.keys()) {
        setParts << QString("%1 = ?").arg(key);
    }

    QString sql = QString("UPDATE %1 SET %2 WHERE %3")
                      .arg(table, setParts.join(", "), condition);

    if (!open()) {
        return false;
    }

    QSqlQuery query(m_db);
    query.prepare(sql);

    for (const auto& key : data.keys()) {
        query.addBindValue(data.value(key));
    }

    if (!query.exec()) {
        m_lastError = query.lastError().text();
        qWarning() << "Update Error:" << m_lastError;
        return false;
    }
    return true;
}

bool DatabaseManager::remove(const QString& table, const QString& condition)
{
    if (condition.isEmpty()) {
        m_lastError = "Delete condition is empty, this is dangerous.";
        return false;
    }

    QString sql = QString("DELETE FROM %1 WHERE %2").arg(table, condition);

    if (!open()) {
        return false;
    }

    QSqlQuery query(m_db);
    if (!query.exec(sql)) {
        m_lastError = query.lastError().text();
        qWarning() << "Delete Error:" << m_lastError;
        return false;
    }
    return true;
}

QVector<QVariantMap> DatabaseManager::select(const QString& table, const QStringList& columns, const QString& condition, const QString& orderBy)
{
    QVector<QVariantMap> results;

    QString columnStr = columns.isEmpty() ? "*" : columns.join(", ");
    QString sql = QString("SELECT %1 FROM %2").arg(columnStr, table);

    if (!condition.isEmpty()) {
        sql += " WHERE " + condition;
    }
    if (!orderBy.isEmpty()) {
        sql += " ORDER BY " + orderBy;
    }

    if (!open()) {
        return results; // 返回空结果
    }

    QSqlQuery query(m_db);
    if (!query.exec(sql)) {
        m_lastError = query.lastError().text();
        qWarning() << "Select Error:" << m_lastError;
        return results;
    }

    QSqlRecord record = query.record();
    while (query.next()) {
        QVariantMap row;
        for (int i = 0; i < record.count(); ++i) {
            row.insert(record.fieldName(i), query.value(i));
        }
        results.append(row);
    }

    return results;
}
