#include "../include/SQLiteTaskRepository.h"
#include "../include/Task.h"
#include <stdexcept>

namespace {
std::string safeColumnText(sqlite3_stmt* stmt, int col) {
    const unsigned char* txt = sqlite3_column_text(stmt, col);
    return txt ? reinterpret_cast<const char*>(txt) : "";
}
} // namespace

void SQLiteTaskRepository::initSchema() {
    const char* sql = R"SQL(
        CREATE TABLE IF NOT EXISTS tasks (
            id INTEGER PRIMARY KEY,
            title TEXT NOT NULL,
            description TEXT NOT NULL,
            status TEXT NOT NULL,
            priority TEXT NOT NULL,
            created_at TEXT NOT NULL
        );
    )SQL";
    char* errorMessage = nullptr;
    auto verdict = sqlite3_exec(db_, sql, nullptr, nullptr, &errorMessage);
    if (verdict != SQLITE_OK) {
        std::string totalError = errorMessage ? errorMessage : "Unknown SQLite error";
        sqlite3_free(errorMessage);
        throw std::runtime_error("Failed " + totalError);
    }
}

std::string SQLiteTaskRepository::toStringStatus(Status status) {
    if (status == Status::Done) {
        return "DONE";
    }
    if (status == Status::InProgress) {
        return "INPROGRESS";
    }
    if (status == Status::New) {
        return "NEW";
    }
    throw std::invalid_argument("Bad status from table");
}

std::string SQLiteTaskRepository::toStringPriority(Priority priority) {
    if (priority == Priority::Low) {
        return "LOW";
    }
    if (priority == Priority::Medium) {
        return "MEDIUM";
    }
    if (priority == Priority::High) {
        return "HIGH";
    }
    throw std::invalid_argument("Bad status from table");
}

Status SQLiteTaskRepository::parseStatus(const std::string& s) {
    if (s == "NEW") {
        return Status::New;
    }
    if (s == "INPROGRESS") {
        return Status::InProgress;
    }
    if (s == "DONE") {
        return Status::Done;
    }
    throw std::out_of_range("Invalid status in DB: " + s);
}

Priority SQLiteTaskRepository::parsePriority(const std::string& s) {
    if (s == "LOW") {
        return Priority::Low;
    }
    if (s == "MEDIUM") {
        return Priority::Medium;
    }
    if (s == "HIGH") {
        return Priority::High;
    }
    throw std::out_of_range("Invalid status in DB: " + s);
}

void SQLiteTaskRepository::reloadCache() const {
    const char* sql = "SELECT id, title, description, status, priority, created_at "
                      "FROM tasks "
                      "ORDER BY id;";
    sqlite3_stmt* stmt = nullptr;
    auto verdict = sqlite3_prepare_v2(db_, sql, -1, &stmt, nullptr);
    if (verdict != SQLITE_OK) {
        throw std::runtime_error("Failed to prepare SELECT : " + std::string(sqlite3_errmsg(db_)));
    }
    cache_.clear();
    while ((verdict = sqlite3_step(stmt)) == SQLITE_ROW) {
        int id = sqlite3_column_int(stmt, 0);
        std::string title = safeColumnText(stmt, 1);
        std::string description = safeColumnText(stmt, 2);
        Status curStatus = parseStatus(safeColumnText(stmt, 3));
        Priority curPriority = parsePriority(safeColumnText(stmt, 4));
        std::string createdAt = safeColumnText(stmt, 5);
        cache_.emplace_back(Task(id, title, description, curStatus, curPriority, createdAt));
    }
    if (verdict != SQLITE_DONE) {
        std::string error = sqlite3_errmsg(db_);
        sqlite3_finalize(stmt);
        throw std::runtime_error("Failed to read tasks: " + error);
    }
    sqlite3_finalize(stmt);
}

SQLiteTaskRepository::SQLiteTaskRepository(const std::string& dbPath) : dbPath_(dbPath) {
    if (sqlite3_open(dbPath.c_str(), &db_) != SQLITE_OK) {
        std::string err = db_ ? sqlite3_errmsg(db_) : "Cannot open DB";
        if (db_) {
            sqlite3_close(db_);
        }
        db_ = nullptr;
        throw std::runtime_error("sqlite3_open failed: " + err);
    }
    initSchema();
    reloadCache();
}

SQLiteTaskRepository::~SQLiteTaskRepository() {
    if (db_) {
        sqlite3_close(db_);
        db_ = nullptr;
    }
}

void SQLiteTaskRepository::addTask(const Task& cur) {
    const char* sql = "INSERT INTO TASKS(id, title, description, status, priority, created_at) "
                      "VALUES(?, ?, ?, ?, ?, ?);";
    sqlite3_stmt* stmt = nullptr;
    auto verdict = sqlite3_prepare_v2(db_, sql, -1, &stmt, nullptr);
    if (verdict != SQLITE_OK) {
        throw std::runtime_error("prepare INSERT failed: " + std::string(sqlite3_errmsg(db_)));
    }
    sqlite3_bind_int(stmt, 1, cur.getId());
    sqlite3_bind_text(stmt, 2, cur.getTitle().c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_text(stmt, 3, cur.getDescription().c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_text(stmt, 4, toStringStatus(cur.getStatus()).c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_text(stmt, 5, toStringPriority(cur.getPriority()).c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_text(stmt, 6, cur.getCreatedAt().c_str(), -1, SQLITE_TRANSIENT);

    verdict = sqlite3_step(stmt);
    sqlite3_finalize(stmt);
    if (verdict != SQLITE_DONE) {
        throw std::runtime_error("INSERT failed: " + std::string(sqlite3_errmsg(db_)));
    }
    cache_.emplace_back(cur);
}

void SQLiteTaskRepository::delTask(int id) {
    const char* sql = "DELETE FROM tasks WHERE id = ?;";
    sqlite3_stmt* stmt = nullptr;
    auto verdict = sqlite3_prepare_v2(db_, sql, -1, &stmt, nullptr);
    if (verdict != SQLITE_OK) {
        throw std::runtime_error("prepare DELETE failed: " + std::string(sqlite3_errmsg(db_)));
    }
    sqlite3_bind_int(stmt, 1, id);
    verdict = sqlite3_step(stmt);
    sqlite3_finalize(stmt);
    if (verdict != SQLITE_DONE) {
        throw std::runtime_error("DELETE failed: " + std::string(sqlite3_errmsg(db_)));
    }
    if (sqlite3_changes(db_) == 0) {
        throw std::out_of_range("Wrong id!!!");
    }
    reloadCache();
}

void SQLiteTaskRepository::updateTask(const Task& task) {
    const char* sql = "UPDATE tasks "
                      "SET title = ?, description = ?, status = ?, priority = ?, created_at = ? "
                      "WHERE id = ?;";
    sqlite3_stmt* stmt = nullptr;
    auto verdict = sqlite3_prepare_v2(db_, sql, -1, &stmt, nullptr);
    if (verdict != SQLITE_OK) {
        throw std::runtime_error("Wrong update!!!");
    }
    sqlite3_bind_text(stmt, 1, task.getTitle().c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_text(stmt, 2, task.getDescription().c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_text(stmt, 3, toStringStatus(task.getStatus()).c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_text(stmt, 4, toStringPriority(task.getPriority()).c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_text(stmt, 5, task.getCreatedAt().c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_int(stmt, 6, task.getId());

    verdict = sqlite3_step(stmt);
    sqlite3_finalize(stmt);
    if (verdict != SQLITE_DONE) {
        throw std::runtime_error("Update failed");
    }
    if (sqlite3_changes(db_) == 0) {
        throw std::out_of_range("Wrong id!");
    }
    reloadCache();
}

Task& SQLiteTaskRepository::findInCacheOrThrow(int id) {
    for (auto& task : cache_) {
        if (task.getId() == id) {
            return task;
        }
    }
    throw std::out_of_range("Wrong id!");
}

Task& SQLiteTaskRepository::getTaskById(int id) {
    reloadCache();
    return findInCacheOrThrow(id);
}

std::vector<Task>& SQLiteTaskRepository::getAllTasks() {
    reloadCache();
    return cache_;
}

const std::vector<Task>& SQLiteTaskRepository::getAllTasks() const {
    reloadCache();
    return cache_;
}