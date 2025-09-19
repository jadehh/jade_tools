/**
# @File     : sqlite_helper.cpp
# @Author   : jade
# @Date     : 2025/9/8 09:28
# @Email    : jadehh@1ive.com
# @Software : Samples
# @Desc     : sqlite_helper.cpp
*/

#include <map>
#include <mutex>
#include "include/jade_tools.h"
#if defined(__has_include)
#  if __has_include(<sqlite3.h>)  // 标准化的头文件存在性检查
#    include <sqlite3.h>
#    define SQLITE3_ENABLE 1
#endif
#endif
#define MODULE_NAME "Sqlite3"

using namespace jade;
class SqliteHelper::Impl{
public:
    explicit Impl(const char* dbPath)
    {
#ifdef SQLITE3_ENABLE
        db_ = nullptr;
        if (sqlite3_open(dbPath, &db_) != SQLITE_OK) {
            throw std::runtime_error("Failed to open database");
        }
        // 启用WAL模式（提高并发性能）
        execute("PRAGMA journal_mode=WAL;");
#endif

    };

    void close()
    {
#ifdef SQLITE3_ENABLE
        if (db_)
        {
            std::lock_guard lock(db_mutex_);
            // 如果还有未提交的事务，回滚
            if (transaction_count_ > 0) {
                sqlite3_exec(db_, "ROLLBACK;", nullptr, nullptr, nullptr);
            }
            sqlite3_close(db_);
            DLL_LOG_TRACE(MODULE_NAME) << "关闭sqlite3完成";
        }
#endif
    }

    ~Impl()
    {
        close();
    }

    bool execute(const std::string& sql)
    {
        std::lock_guard lock(db_mutex_);
#ifdef SQLITE3_ENABLE
        char* errMsg = nullptr;
        if (sqlite3_exec(db_, sql.c_str(), nullptr, nullptr, &errMsg) != SQLITE_OK) {
            DLL_LOG_WARN(MODULE_NAME) << "SQL WARN: " << errMsg;
            sqlite3_free(errMsg);
            return false;
        }
#endif
        return true;
    }

     std::vector<std::map<std::string,SQLiteValue>> query(const std::string& sql)
    {
#ifdef SQLITE3_ENABLE
        std::lock_guard lock(db_mutex_);
        std::vector<std::map<std::string,SQLiteValue>> results;
        sqlite3_stmt* stmt = nullptr;

        if (sqlite3_prepare_v2(db_, sql.c_str(), -1, &stmt, nullptr) != SQLITE_OK) {
            DLL_LOG_ERROR(MODULE_NAME) << "Failed to prepare statement";
            throw std::runtime_error("Failed to prepare statement");
        }
        const int colCount = sqlite3_column_count(stmt);
        while (sqlite3_step(stmt) == SQLITE_ROW) {
            std::map<std::string,SQLiteValue> row;
            for (int i = 0; i < colCount; ++i) {
                const char* columnName = sqlite3_column_name(stmt, i);
                switch (sqlite3_column_type(stmt, i)) {
                case SQLITE_NULL:
                    row[columnName] = nullptr;
                    break;
                case SQLITE_INTEGER:
                    row[columnName] = sqlite3_column_int64(stmt, i);
                    break;
                case SQLITE_FLOAT:
                    row[columnName] = sqlite3_column_double(stmt, i);
                    break;
                case SQLITE_TEXT:
                    row[columnName] = reinterpret_cast<const char*>(sqlite3_column_text(stmt, i));
                    break;
                case SQLITE_BLOB:
                    {
                        const void* blob = sqlite3_column_blob(stmt, i);
                        const int size = sqlite3_column_bytes(stmt, i);
                        const auto data = static_cast<const uint8_t*>(blob);
                        row[columnName] = std::vector<uint8_t>(data, data + size);
                        break;
                    }
                default:
                    DLL_LOG_ERROR(MODULE_NAME) << "Unsupported SQLite data type: " << columnName;
                    throw std::runtime_error("Unsupported SQLite data type");
                }
            }
            results.emplace_back(row);
        }
        sqlite3_finalize(stmt);
        return results;
#else
        return {}
#endif
    }

    bool executeWithParams(const std::string& sql, const std::vector<SQLiteValue>& params)
    {
#ifdef SQLITE3_ENABLE
        std::lock_guard lock(db_mutex_);
        sqlite3_stmt* stmt = nullptr;

        if (sqlite3_prepare_v2(db_, sql.c_str(), -1, &stmt, nullptr) != SQLITE_OK) {
            return false;
        }

        for (size_t i = 0; i < params.size(); ++i) {
            std::visit([&](auto&& arg) {
                using T = std::decay_t<decltype(arg)>;
                if constexpr (std::is_same_v<T, std::monostate>) {
                    sqlite3_bind_null(stmt, static_cast<int>(i)+1);
                }
                else if constexpr (std::is_same_v<T, int64_t>) {
                    sqlite3_bind_int64(stmt, i+1, arg);
                }
                else if constexpr (std::is_same_v<T, double>)
                {
                    sqlite3_bind_double(stmt, i+1, arg);
                }
                else if constexpr (std::is_same_v<T, std::string>) {
                  sqlite3_bind_text(stmt, i+1, arg.c_str(), -1, SQLITE_TRANSIENT);
                }
                else if constexpr (std::is_same_v<T, std::vector<uint8_t>>) {
                    sqlite3_bind_blob(stmt, i+1, arg.data(), arg.size(), SQLITE_TRANSIENT);
            }
            }, params[i]);
        }

        const int result = sqlite3_step(stmt);
        sqlite3_finalize(stmt);
        return (result == SQLITE_DONE);
#else
        return false;
#endif
    }

    // 添加事务管理方法
    void beginTransaction() {
        std::lock_guard<std::mutex> lock(trans_mutex_);
        if (transaction_count_ == 0) {
#ifdef SQLITE3_ENABLE
            if (!execute("BEGIN TRANSACTION;")) {
                DLL_LOG_ERROR(MODULE_NAME) << "Failed to begin transaction";
                throw std::runtime_error("Failed to begin transaction");
            }
#endif
        }
        transaction_count_++;
    }

    void commitTransaction() {
        std::lock_guard lock(trans_mutex_);
        if (transaction_count_ <= 0) {
            DLL_LOG_ERROR(MODULE_NAME) << "No active transaction to commit";
            throw std::runtime_error("No active transaction to commit");
        }

        transaction_count_--;
        if (transaction_count_ == 0) {
            if (!execute("COMMIT;")) {
                DLL_LOG_ERROR(MODULE_NAME) << "Failed to commit transaction";
                throw std::runtime_error("Failed to commit transaction");
            }
        }
    }
    void rollbackTransaction() {
        std::lock_guard lock(trans_mutex_);
        if (transaction_count_ <= 0) {
            DLL_LOG_ERROR(MODULE_NAME) << "No active transaction to rollback";
            throw std::runtime_error("No active transaction to rollback");
        }

        transaction_count_ = 0; // 重置计数器
        if (!execute("ROLLBACK;")) {
            DLL_LOG_ERROR(MODULE_NAME) << "Failed to rollback transaction";
            throw std::runtime_error("Failed to rollback transaction");
        }
    }
    // 禁止复制
    Impl(const Impl&) = delete;
    Impl& operator=(const Impl&) = delete;
private:
#ifdef SQLITE3_ENABLE
    sqlite3* db_;
#endif
    std::mutex db_mutex_;
    std::mutex trans_mutex_; // 事务互斥锁
    int transaction_count_ = 0;  // 事务嵌套计数器
};

SqliteHelper& SqliteHelper::getInstance()
{
    static SqliteHelper instance;
    return instance;
}

bool SqliteHelper::execute(const std::string& sql) const
{
    return impl_->execute(sql);
}

std::vector<std::map<std::string,SqliteHelper::SQLiteValue>>  SqliteHelper::query(const std::string& sql) const
{
    return impl_->query(sql);
}


bool SqliteHelper::executeWithParams(const std::string& sql, const std::vector<SQLiteValue>& params) const
{
    return impl_->executeWithParams(sql,params);
}

SqliteHelper::SqliteHelper():impl_(nullptr)
{
}

void SqliteHelper::init(const char* dbPath)
{
    impl_ = new Impl(dbPath);
}

SqliteHelper::Transaction::Transaction(SqliteHelper& db):db_(db),committed(false)
{
    db.impl_->beginTransaction();
}

SqliteHelper::Transaction::~Transaction()
{
    if (!committed) {
        try {
            db_.impl_->rollbackTransaction();
        } catch (const std::exception& e) {
            DLL_LOG_ERROR(MODULE_NAME) << "Error during transaction rollback: " << e.what();
        }
    }
}

void SqliteHelper::Transaction::commit()
{
    if (committed) {
        DLL_LOG_ERROR(MODULE_NAME) << "Transaction already committed";
        throw std::runtime_error("Transaction already committed");
    }

    try {
        db_.impl_->commitTransaction();
        committed = true;
    } catch (const std::exception& e) {
        DLL_LOG_ERROR(MODULE_NAME) << "Failed to commit transaction: " + std::string(e.what());
        throw std::runtime_error("Failed to commit transaction: " + std::string(e.what()));
    }
}
void SqliteHelper::close() const
{
    if (impl_)
    {
        impl_->close();
    }
}

