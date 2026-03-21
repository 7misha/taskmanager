#include "../include/SQLiteTaskRepository.h"
#include <chrono>
#include <filesystem>
#include <gtest/gtest.h>
#include <string>

namespace {
std::filesystem::path uniqueDbPath(const std::string& tag) {
    const auto now = std::chrono::steady_clock::now().time_since_epoch().count();
    return std::filesystem::temp_directory_path() /
           ("taskmanager_sqlite_" + tag + "_" + std::to_string(now) + ".db");
}
} // namespace

TEST(SQLiteTaskRepositoryTest, AddGetAndDeleteTask) {
    const auto dbPath = uniqueDbPath("flow");

    {
        SQLiteTaskRepository repo(dbPath.string());
        repo.addTask(Task(1, "A", "desc A", Status::New, Priority::Low, "2026-03-20"));
        repo.addTask(Task(2, "B", "desc B", Status::Done, Priority::High, "2026-03-21"));

        Task& first = repo.getTaskById(1);
        EXPECT_EQ(first.getTitle(), "A");
        EXPECT_EQ(repo.getAllTasks().size(), 2u);

        repo.delTask(1);
        EXPECT_THROW(repo.getTaskById(1), std::out_of_range);
        EXPECT_EQ(repo.getAllTasks().size(), 1u);
        EXPECT_EQ(repo.getAllTasks().front().getId(), 2);
    }

    std::filesystem::remove(dbPath);
}

TEST(SQLiteTaskRepositoryTest, DeleteNonExistentTaskThrows) {
    const auto dbPath = uniqueDbPath("delete_missing");

    {
        SQLiteTaskRepository repo(dbPath.string());
        EXPECT_THROW(repo.delTask(42), std::out_of_range);
    }

    std::filesystem::remove(dbPath);
}

TEST(SQLiteTaskRepositoryTest, DataPersistsAcrossRepositoryInstances) {
    const auto dbPath = uniqueDbPath("persist");

    {
        SQLiteTaskRepository repo(dbPath.string());
        repo.addTask(Task(5, "Persisted", "From first instance", Status::InProgress,
                          Priority::Medium, "2026-03-22"));
    }

    {
        SQLiteTaskRepository repo(dbPath.string());
        Task& loaded = repo.getTaskById(5);
        EXPECT_EQ(loaded.getTitle(), "Persisted");
        EXPECT_EQ(loaded.getDescription(), "From first instance");
        EXPECT_EQ(loaded.getStatus(), Status::InProgress);
        EXPECT_EQ(loaded.getPriority(), Priority::Medium);
    }

    std::filesystem::remove(dbPath);
}

TEST(SQLiteTaskRepositoryTest, UpdateTaskPersistsToDatabase) {
    const auto dbPath = uniqueDbPath("update_persist");

    {
        SQLiteTaskRepository repo(dbPath.string());
        repo.addTask(Task(7, "Before", "Old desc", Status::New, Priority::Low, "2026-03-22"));

        Task& task = repo.getTaskById(7);
        task.updateTitle("After");
        task.updateDescription("New desc");
        task.updateStatus(Status::Done);
        task.updatePriority(Priority::High);
        repo.updateTask(task);
    }

    {
        SQLiteTaskRepository repo(dbPath.string());
        Task& loaded = repo.getTaskById(7);
        EXPECT_EQ(loaded.getTitle(), "After");
        EXPECT_EQ(loaded.getDescription(), "New desc");
        EXPECT_EQ(loaded.getStatus(), Status::Done);
        EXPECT_EQ(loaded.getPriority(), Priority::High);
    }

    std::filesystem::remove(dbPath);
}
