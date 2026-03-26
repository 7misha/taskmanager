#pragma once

#include "ITaskRepository.h"
#include "Task.h"
#include <sqlite3.h>

class SQLiteTaskRepository : public ITaskRepository {
private:
    sqlite3* db_ = nullptr;
    std::string dbPath_;
    mutable std::vector<Task> cache_;

    Task& findInCacheOrThrow(int id);

    void initSchema();

    void reloadCache() const;

public:
    explicit SQLiteTaskRepository(const std::string& dbPath);

    void addTask(const Task& cur) override;

    void delTask(int id) override;

    void updateTask(const Task& task) override;

    Task& getTaskById(int id) override;

    const std::vector<Task>& getAllTasks() const override;

    std::vector<Task>& getAllTasks() override;

    void replaceAllTasks(const std::vector<Task>& v) override;

    static Status parseStatus(const std::string& s);

    static Priority parsePriority(const std::string& s);

    static std::string toStringStatus(Status status);

    static std::string toStringPriority(Priority priority);

    ~SQLiteTaskRepository() override;
};
