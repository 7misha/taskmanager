#pragma once

#include "Task.h"
#include <vector>

class ITaskRepository {
public:
    virtual void addTask(const Task& cur) = 0;
    virtual void delTask(int id) = 0;
    virtual void updateTask(const Task& task) = 0;

    virtual Task& getTaskById(int id) = 0;

    virtual std::vector<Task>& getAllTasks() = 0;
    virtual const std::vector<Task>& getAllTasks() const = 0;

    virtual ~ITaskRepository() = default;
};