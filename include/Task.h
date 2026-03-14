#pragma once

#include <iostream>
#include <string>

enum class Status { New, InProgress, Done };

enum class Priority { Low, Medium, High };

struct Task {
private:
    int id_;
    std::string title_;
    std::string description_;
    Status status_;
    Priority priority_;
    std::string createdAt_;

public:
    Task(int id, const std::string& title, const std::string& description, Status status,
         Priority priority, const std::string& createdAt);

    int getId() const;
    const std::string& getTitle() const;
    const std::string& getDescription() const;
    Status getStatus() const;
    Priority getPriority() const;
    const std::string& getCreatedAt() const;
    void updateTitle(const std::string& newTitle);
    void updateDescription(const std::string& newDescription);
    void updateStatus(Status newStatus);
    void updatePriority(Priority newPriority);
    void print() const;
};
