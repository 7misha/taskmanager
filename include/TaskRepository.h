#pragma once

#include "ITaskRepository.h"

class TaskRepository : public ITaskRepository {
private:
    std::vector<Task> tasks_;

public:
    void addTask(const Task& cur) override;

    void delTask(int id) override;

    Task& getTaskById(int id) override;

    const std::vector<Task>& getAllTasks() const override;

    std::vector<Task>& getAllTasks() override;
};