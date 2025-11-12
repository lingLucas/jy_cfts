#ifndef DATABASEMANAGER_H
#define DATABASEMANAGER_H

#include <QObject>
#include <QSqlDatabase>
#include <QSqlQuery>
#include <QSqlError>
#include <QString>
#include <QVariantMap>
#include <QVector>

class DatabaseManager : public QObject
{
    Q_OBJECT
public:
    explicit DatabaseManager(const QString& path, QObject *parent = nullptr);
    ~DatabaseManager();

    // 打开数据库连接
    bool open();
    // 关闭数据库连接
    void close();

    // 获取最后一次操作的错误信息
    QString lastError() const;

    // --- 数据库操作 ---
    // 执行自定义SQL语句 (例如: CREATE TABLE, DROP TABLE)
    bool exec(const QString& sql);

    // 插入数据
    // table: 表名
    // data: 键值对，键为字段名，值为要插入的数据
    bool insert(const QString& table, const QVariantMap& data);

    // 更新数据
    // table: 表名
    // data: 键值对，要更新的字段和新值
    // condition: WHERE 条件字符串 (例如: "id = 1")
    bool update(const QString& table, const QVariantMap& data, const QString& condition);

    // 删除数据
    // table: 表名
    // condition: WHERE 条件字符串 (例如: "id = 1")
    bool remove(const QString& table, const QString& condition);

    // 查询数据
    // table: 表名
    // columns: 要查询的字段列表，为空则查询所有 (SELECT *)
    // condition: WHERE 条件字符串
    // orderBy: 排序字段 (例如: "id DESC")
    // 返回包含所有查询结果的 QVector，每个元素是一行数据的 QVariantMap
    QVector<QVariantMap> select(const QString& table, const QStringList& columns = QStringList(),
                                const QString& condition = "", const QString& orderBy = "");

signals:


private:
    QSqlDatabase m_db;
    QString m_connectionName;
    QString m_lastError;

    // 生成一个唯一的连接名，避免冲突
    QString generateConnectionName();
};

#endif // DATABASEMANAGER_H
