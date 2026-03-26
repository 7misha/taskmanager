#pragma once

#include "ITaskRepository.h"

class TaskRepository : public ITaskRepository {
private:
    std::vector<Task> tasks_;

public:
    void addTask(const Task& cur) override;

    void delTask(int id) override;

    void updateTask(const Task& task) override;

    void replaceAllTasks(const std::vector<Task>& tasks) override;

    Task& getTaskById(int id) override;

    const std::vector<Task>& getAllTasks() const override;

    std::vector<Task>& getAllTasks() override;
};
