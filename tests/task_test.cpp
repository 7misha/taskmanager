#include "../include/Task.h"
#include <gtest/gtest.h>

TEST(TaskTest, ConstructAndUpdateFields) {
    Task task(7, "Initial title", "Initial description", Status::New, Priority::Low, "2026-03-14");

    EXPECT_EQ(task.getId(), 7);
    EXPECT_EQ(task.getTitle(), "Initial title");
    EXPECT_EQ(task.getDescription(), "Initial description");
    EXPECT_EQ(task.getStatus(), Status::New);
    EXPECT_EQ(task.getPriority(), Priority::Low);
    EXPECT_EQ(task.getCreatedAt(), "2026-03-14");

    task.updateTitle("Updated title");
    task.updateDescription("Updated description");
    task.updateStatus(Status::Done);
    task.updatePriority(Priority::High);

    EXPECT_EQ(task.getTitle(), "Updated title");
    EXPECT_EQ(task.getDescription(), "Updated description");
    EXPECT_EQ(task.getStatus(), Status::Done);
    EXPECT_EQ(task.getPriority(), Priority::High);
}
