/**
# @File     : testSqlite3.cpp
# @Author   : jade
# @Date     : 2025/9/8 10:19
# @Email    : jadehh@1ive.com
# @Software : Samples
# @Desc     : testSqlite3.cpp
*/

#ifdef JADE_TOOLS
#include "jade_tools/jade_tools.h"
#else
#include "include/jade_tools.h"
#endif
#include "test/include/testSqlite3.h"
#include <thread>

#include <iostream>
#include <string>
// 多线程使用示例
void workerTask(const int id)
{
    try
    {
        auto& db = jade::SqliteHelper::getInstance();
        // 使用事务
        {
            jade::SqliteHelper::Transaction trans(db);

            // 参数化插入
            if (!db.executeWithParams(
                "INSERT INTO users (name, age) VALUES (?, ?);",
                {"User" + std::to_string(id), std::to_string(20 + id)}
            ))
            {
                LOG_WARN() << "Failed to insert users";
            }
            std::vector<jade::SqliteHelper::SQLiteValue> values;
            values.emplace_back(static_cast<SqliteInt64>(180));
            values.emplace_back(static_cast<SqliteInt64>(2));
            if (!db.executeWithParams("UPDATE users SET age = ? WHERE id = ?;", values))
            {
                LOG_WARN() << "Failed to update age";
            }
            trans.commit();
        }

        // 查询数据
        const auto results = db.query("SELECT * FROM users;");
    }
    catch (const std::exception& e)
    {
        std::cerr << "Thread " << id << " error: " << e.what() << std::endl;
    }
}

void testSqlite3()
{
    LOG_INFO() << "=====================================Sqlite3 测试开始" << "=====================================";

    jade::SqliteHelper::getInstance().init("threadsafe.db");
    // 创建表
    const bool status = jade::SqliteHelper::getInstance().execute("CREATE TABLE IF NOT EXISTS users ("
        "id INTEGER PRIMARY KEY AUTOINCREMENT,"
        "name TEXT NOT NULL,"
        "age INTEGER);");
    if (status)
    {
        // 创建多个工作线程
        std::vector<std::thread> threads;
        threads.reserve(1);
        for (int i = 0; i < 1; ++i)
        {
            threads.emplace_back(workerTask, i);
        }
        // 等待所有线程完成
        for (auto& t : threads)
        {
            t.join();
        }
    }
    LOG_INFO() << "=====================================Sqlite3 测试结束" << "=====================================";
}
