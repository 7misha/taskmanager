#pragma once

#include "../include/Task.h"
#include <exception>
#include <stdexcept>
#include <vector>

class TaskRepository {
private:
    std::vector<Task> tasks_;

public:
    void addTask(const Task& cur);

    void delTask(int id);

    Task& getTaskById(int id);

    const std::vector<Task>& getAllTasks() const;

    std::vector<Task>& getAllTasks();
};