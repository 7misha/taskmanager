#pragma once

#include <iostream>
#include <memory>
#include <string>

enum class Status { New, InProgress, Done };

enum class Priority { Low, Medium, High };

class Task {
private:
    int id_;
    std::string title_;
    std::string description_;
    Status status_;
    Priority priority_;
    std::string createdAt_;

public:
    Task(int id, std::string& title, std::string& description, Status status, Priority priority,
         std::string& createdAt);
    int getId() const {
        return id_;
    }
    void updateTitle(std::string& newTitle);
    void updateDescription(std::string& newDescription);
    void updateStatus(Status newStatus);
    void updatePriority(Priority newPriority);
    void print() const;
};