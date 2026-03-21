#include "../include/TaskRepository.h"

void TaskRepository::addTask(const Task& cur) {
    tasks_.emplace_back(cur);
}

void TaskRepository::delTask(int id) {
    size_t curId = 0;
    for (const auto& to : tasks_) {
        if (to.getId() == id) {
            break;
        }
        curId++;
    }
    if (curId == tasks_.size()) {
        throw std::out_of_range("Wrong id!!!");
    }
    tasks_.erase(tasks_.begin() + curId);
}

void TaskRepository::updateTask(const Task& task) {
    Task& x = getTaskById(task.getId());
    x = task;
}

Task& TaskRepository::getTaskById(int id) {
    for (auto& to : tasks_) {
        if (to.getId() == id) {
            return to;
        }
    }
    throw std::out_of_range("Wrong id!");
}

std::vector<Task>& TaskRepository::getAllTasks() {
    return tasks_;
}

const std::vector<Task>& TaskRepository::getAllTasks() const {
    return tasks_;
}
