// database_manager.h
#ifndef DATABASE_MANAGER_H
#define DATABASE_MANAGER_H

#include <QSqlDatabase>
#include <QSqlError>
#include <QDebug>

class DatabaseManager {
public:
    static DatabaseManager& instance() {
        static DatabaseManager manager;
        return manager;
    }

    bool openDatabase(const QString& dbName = "mydatabase.db") {
        if (db.isOpen())
            return true;

        db = QSqlDatabase::addDatabase("QSQLITE", "MainConnection");
        db.setDatabaseName(dbName);

        if (!db.open()) {
            qDebug() << "Database error:" << db.lastError().text();
            return false;
        }

        qDebug() << "Database opened successfully";
        return true;
    }

    QSqlDatabase& database() {
        return db;
    }

    void closeDatabase() {
        if (db.isOpen()) {
            db.close();
            qDebug() << "Database closed";
        }
    }

private:
    DatabaseManager() = default;
    ~DatabaseManager() { closeDatabase(); }

    DatabaseManager(const DatabaseManager&) = delete;
    DatabaseManager& operator=(const DatabaseManager&) = delete;

    QSqlDatabase db;
};

#endif // DATABASE_MANAGER_H
