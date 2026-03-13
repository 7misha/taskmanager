#pragma once

#include "../include/TaskRepository.h"

Status getStatus(const std::string& type) {
    if (type == "NEW") {
        return Status::New;
    }
    if (type == "INPROGRESS") {
        return Status::InProgress;
    }
    if (type == "DONE") {
        return Status::Done;
    }
    throw std::out_of_range("Wrong Status!!!");
}

Priority getPriority(const std::string& type) {
    if (type == "LOW") {
        return Priority::Low;
    }
    if (type == "INPROGRESS") {
        return Priority::Medium;
    }
    if (type == "DONE") {
        return Priority::High;
    }
    throw std::out_of_range("Wrong Priority!!!");
}

void TaskRepository::addTask(std::string& title, std::string& description,
                             const std::string& status, const std::string& priority,
                             std::string createdAt) {
    Status eStatus;
    tasks_.emplace_back(id_, title, description, getStatus(status), getPriority(priority),
                        createdAt);
    ++id_;
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

Task TaskRepository::getTaskById(int id) const {
    for (const auto& to : tasks) {
        if (to.getId() == id) {
            return to;
        }
    }
    throw std::out_of_range("Wrong id!");
}

std::vector<Task> TaskRepository::getAllTasks() const {
    return tasks_;
}