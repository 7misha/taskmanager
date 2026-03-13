#pragma once

#include "../include/Task.h"
#include <exception>
#include <stdexcept>
#include <vector>

enum class Verdict { GOOD, EMPTY_TITLE, EMPTY_DESCRIPTION, EMPTY_DATE, WRONG_ID };

class TaskRepository {
private:
    std::vector<Task> tasks_;
    int id_ = 1;

public:
    void addTask(std::string& title, std::string& description, const std::string& status,
                 const std::string& priority, std::string createdAt);

    void delTask(int id);

    Task getTaskById(int id) const {
        for (const auto& to : tasks) {
            if (to.getId() == id) {
                return to;
            }
        }
        throw std::out_of_range("Wrong id!");
    }

    std::vector<Task> getTasks() const {
        return tasks_;
    }
};