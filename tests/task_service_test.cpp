#include "../include/TaskService.h"
#include <filesystem>
#include <fstream>
#include <gtest/gtest.h>
#include <sstream>
#include <string>
#include <vector>

namespace {
std::vector<std::string> splitTabs(const std::string& line) {
    std::vector<std::string> parts;
    std::stringstream ss(line);
    std::string chunk;
    while (std::getline(ss, chunk, '\t')) {
        parts.push_back(chunk);
    }
    return parts;
}

std::filesystem::path uniquePath(const std::string& suffix) {
    return std::filesystem::temp_directory_path() / ("taskmanager_" + suffix);
}
} // namespace

TEST(TaskServiceTest, AddFindAndDeleteFlow) {
    TaskRepository repo;
    TaskService service(repo);

    service.addTaskService("Buy milk", "Whole milk", "LOW", "2026-03-14");
    service.addTaskService("Write docs", "README", "MEDIUM", "2026-03-15");

    auto ids = service.findByTitle("milk");
    ASSERT_EQ(ids.size(), 1u);
    EXPECT_EQ(ids[0], 1);

    service.deleteTask(1);
    EXPECT_TRUE(service.findByTitle("milk").empty());
}

TEST(TaskServiceTest, ChangeFieldsAndSaveToFile) {
    TaskRepository repo;
    TaskService service(repo);
    service.addTaskService("A", "B", "LOW", "2026-03-14");

    service.changeTitle(1, "Updated title");
    service.changeDescription(1, "Updated description");
    service.changeStatus(1, "DONE");
    service.changePriority(1, "HIGH");

    const auto path = uniquePath("save_test.tsv");
    service.saveToFile(path.string());

    std::ifstream in(path);
    ASSERT_TRUE(in.is_open());
    std::string line;
    ASSERT_TRUE(std::getline(in, line));

    const auto fields = splitTabs(line);
    ASSERT_EQ(fields.size(), 6u);
    EXPECT_EQ(fields[0], "1");
    EXPECT_EQ(fields[1], "Updated title");
    EXPECT_EQ(fields[2], "Updated description");
    EXPECT_EQ(fields[3], "DONE");
    EXPECT_EQ(fields[4], "HIGH");
    EXPECT_EQ(fields[5], "2026-03-14");

    std::filesystem::remove(path);
}

TEST(TaskServiceTest, ReadFromFileReplacesTasksAndResyncsIds) {
    TaskRepository repo;
    TaskService service(repo);
    service.addTaskService("Old task", "old desc", "LOW", "2026-03-10");

    const auto inPath = uniquePath("read_input.tsv");
    {
        std::ofstream out(inPath);
        out << "5\tTask five\tD5\tNEW\tLOW\t2026-03-11\n";
        out << "9\tTask nine\tD9\tINPROGRESS\tMEDIUM\t2026-03-12\n";
    }

    service.loadFromFile(inPath.string());
    EXPECT_TRUE(service.findByTitle("Old task").empty());
    EXPECT_EQ(service.findByTitle("Task five").size(), 1u);
    EXPECT_EQ(service.findByTitle("Task nine").size(), 1u);

    service.addTaskService("After load", "D10", "HIGH", "2026-03-13");
    const auto outPath = uniquePath("read_output.tsv");
    service.saveToFile(outPath.string());

    std::ifstream in(outPath);
    ASSERT_TRUE(in.is_open());
    std::string line;
    bool foundNewTask = false;
    while (std::getline(in, line)) {
        const auto fields = splitTabs(line);
        ASSERT_EQ(fields.size(), 6u);
        if (fields[1] == "After load") {
            foundNewTask = true;
            EXPECT_EQ(fields[0], "10");
        }
    }
    EXPECT_TRUE(foundNewTask);

    std::filesystem::remove(inPath);
    std::filesystem::remove(outPath);
}

TEST(TaskServiceTest, ReadFromFileWithDuplicateIdsThrows) {
    TaskRepository repo;
    TaskService service(repo);
    const auto path = uniquePath("duplicate_ids.tsv");
    {
        std::ofstream out(path);
        out << "1\tA\tD1\tNEW\tLOW\t2026-03-11\n";
        out << "1\tB\tD2\tDONE\tHIGH\t2026-03-12\n";
    }

    EXPECT_THROW(service.loadFromFile(path.string()), std::invalid_argument);
    std::filesystem::remove(path);
}

TEST(TaskServiceTest, ReadFromFileErrorDoesNotOverwriteExistingTasks) {
    TaskRepository repo;
    TaskService service(repo);
    service.addTaskService("Keep me", "original", "LOW", "2026-03-10");

    const auto badPath = uniquePath("bad_input.tsv");
    {
        std::ofstream out(badPath);
        out << "2\tImported\tD2\tNEW\tLOW\t2026-03-11\n";
        out << "broken-line-without-tabs\n";
    }

    EXPECT_THROW(service.loadFromFile(badPath.string()), std::invalid_argument);
    EXPECT_EQ(service.findByTitle("Keep me").size(), 1u);
    EXPECT_TRUE(service.findByTitle("Imported").empty());

    std::filesystem::remove(badPath);
}

TEST(TaskServiceTest, SaveToFileWorksInNestedDirectory) {
    TaskRepository repo;
    TaskService service(repo);
    service.addTaskService("Nested", "Dir", "LOW", "2026-03-16");

    const auto baseDir = uniquePath("save_nested_dir");
    const auto nestedDir = baseDir / "sub";
    std::filesystem::create_directories(nestedDir);
    const auto outPath = nestedDir / "tasks.tsv";

    service.saveToFile(outPath.string());

    std::ifstream in(outPath);
    ASSERT_TRUE(in.is_open());
    std::string line;
    ASSERT_TRUE(std::getline(in, line));
    const auto fields = splitTabs(line);
    ASSERT_EQ(fields.size(), 6u);
    EXPECT_EQ(fields[1], "Nested");

    std::filesystem::remove_all(baseDir);
}
