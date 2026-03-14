#include "../include/TaskRepository.h"
#include <gtest/gtest.h>

TEST(TaskRepositoryTest, AddGetAndDeleteTask) {
    TaskRepository repo;
    repo.addTask(Task(1, "A", "desc A", Status::New, Priority::Low, "d1"));
    repo.addTask(Task(2, "B", "desc B", Status::Done, Priority::High, "d2"));

    Task& first = repo.getTaskById(1);
    EXPECT_EQ(first.getTitle(), "A");

    repo.delTask(1);
    EXPECT_THROW(repo.getTaskById(1), std::out_of_range);
    EXPECT_EQ(repo.getAllTasks().size(), 1u);
    EXPECT_EQ(repo.getAllTasks().front().getId(), 2);
}

TEST(TaskRepositoryTest, DeleteNonExistentTaskThrows) {
    TaskRepository repo;
    EXPECT_THROW(repo.delTask(42), std::out_of_range);
}
