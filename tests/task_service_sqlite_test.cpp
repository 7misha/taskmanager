#include "../include/SQLiteTaskRepository.h"
#include "../include/TaskService.h"
#include <chrono>
#include <filesystem>
#include <gtest/gtest.h>
#include <string>

namespace {
std::filesystem::path uniqueDbPath(const std::string& tag) {
    const auto now = std::chrono::steady_clock::now().time_since_epoch().count();
    return std::filesystem::temp_directory_path() /
           ("taskmanager_service_sqlite_" + tag + "_" + std::to_string(now) + ".db");
}
} // namespace

TEST(TaskServiceSQLiteTest, AddAndFindPersistAcrossInstances) {
    const auto dbPath = uniqueDbPath("add_find");

    {
        SQLiteTaskRepository repo(dbPath.string());
        TaskService service(repo);
        service.addTaskService("Buy milk", "2L", "LOW", "2026-03-21");
        service.addTaskService("Write docs", "README", "MEDIUM", "2026-03-21");

        const auto ids = service.findByTitle("milk");
        ASSERT_EQ(ids.size(), 1u);
        EXPECT_EQ(ids[0], 1);
    }

    {
        SQLiteTaskRepository repo(dbPath.string());
        TaskService service(repo);
        const auto idsMilk = service.findByTitle("milk");
        const auto idsDocs = service.findByTitle("docs");
        ASSERT_EQ(idsMilk.size(), 1u);
        ASSERT_EQ(idsDocs.size(), 1u);
        EXPECT_EQ(idsMilk[0], 1);
        EXPECT_EQ(idsDocs[0], 2);
    }

    std::filesystem::remove(dbPath);
}

TEST(TaskServiceSQLiteTest, ChangeOperationsPersistAcrossInstances) {
    const auto dbPath = uniqueDbPath("change_persist");

    {
        SQLiteTaskRepository repo(dbPath.string());
        TaskService service(repo);
        service.addTaskService("Before", "Old desc", "LOW", "2026-03-21");
        service.changeTitle(1, "After");
        service.changeDescription(1, "New desc");
        service.changeStatus(1, "DONE");
        service.changePriority(1, "HIGH");
    }

    {
        SQLiteTaskRepository repo(dbPath.string());
        TaskService service(repo);
        const auto ids = service.findByTitle("After");
        ASSERT_EQ(ids.size(), 1u);
        Task& task = repo.getTaskById(1);
        EXPECT_EQ(task.getDescription(), "New desc");
        EXPECT_EQ(task.getStatus(), Status::Done);
        EXPECT_EQ(task.getPriority(), Priority::High);
    }

    std::filesystem::remove(dbPath);
}

TEST(TaskServiceSQLiteTest, DeletePersistsAcrossInstances) {
    const auto dbPath = uniqueDbPath("delete_persist");

    {
        SQLiteTaskRepository repo(dbPath.string());
        TaskService service(repo);
        service.addTaskService("Temp", "Will be deleted", "LOW", "2026-03-21");
        EXPECT_EQ(service.findByTitle("Temp").size(), 1u);
        service.deleteTask(1);
        EXPECT_TRUE(service.findByTitle("Temp").empty());
    }

    {
        SQLiteTaskRepository repo(dbPath.string());
        TaskService service(repo);
        EXPECT_TRUE(service.findByTitle("Temp").empty());
        EXPECT_THROW(repo.getTaskById(1), std::out_of_range);
    }

    std::filesystem::remove(dbPath);
}
