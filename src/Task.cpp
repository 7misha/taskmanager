#include "../include/Task.h"

Task::Task(int id, std::string& title, std::string& description, Status status, Priority priority,
           std::string& createdAt)
    : id_(id), title_(std::move(title)), description_(std::move(description)), status_(status),
      priority_(priority), createdAt_(std::move(createdAt)) {
}

void Task::updateTitle(std::string& newTitle) {
    title_ = std::move(newTitle);
}

void Task::updateDescription(std::string& newDescription) {
    description_ = std::move(newDescription);
}

void Task::updateStatus(Status newStatus) {
    status_ = newStatus;
}

void Task::updatePriority(Priority newPriority) {
    priority_ = newPriority;
}

void Task::print() {
    std::cout << "TASK: " << id_ + 1 << '\n';
    std::cout << "TITLE: " << title_ << '\n';
    std::cout << "DESCRIPTION: " << description_ << '\n';
    std::cout << "STATUS: ";
    switch (status_) {
        case Status::New:
            std::cout << "New\n";
        case Status::InProgress:
            std::cout << "InProgress\n";
        case Status::Done:
            std::cout << "Done\n";
    }
    std::cout << "PRIORITY: ";
    switch (priority_) {
        case Priority::Low:
            std::cout << "Low\n";
        case Priority::Medium:
            std::cout << "Medium\n";
        case Priority::High:
            std::cout << "High\n";
    }
    std::cout << "CREATED AT: " << createdAt_ << '\n';
}