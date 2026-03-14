#include "../include/Task.h"

Task::Task(int id, const std::string& title, const std::string& description, Status status,
           Priority priority, const std::string& createdAt)
    : id_(id), title_(title), description_(description), status_(status), priority_(priority),
      createdAt_(createdAt) {
}

int Task::getId() const {
    return id_;
}

const std::string& Task::getTitle() const {
    return title_;
}

const std::string& Task::getDescription() const {
    return description_;
}

Status Task::getStatus() const {
    return status_;
}

Priority Task::getPriority() const {
    return priority_;
}

const std::string& Task::getCreatedAt() const {
    return createdAt_;
}

void Task::updateTitle(const std::string& newTitle) {
    title_ = newTitle;
}

void Task::updateDescription(const std::string& newDescription) {
    description_ = newDescription;
}

void Task::updateStatus(Status newStatus) {
    status_ = newStatus;
}

void Task::updatePriority(Priority newPriority) {
    priority_ = newPriority;
}

void Task::print() const {
    std::cout << "TASK: " << id_ << '\n';
    std::cout << "TITLE: " << title_ << '\n';
    std::cout << "DESCRIPTION: " << description_ << '\n';
    std::cout << "STATUS: ";
    switch (status_) {
        case Status::New:
            std::cout << "New\n";
            break;
        case Status::InProgress:
            std::cout << "InProgress\n";
            break;
        case Status::Done:
            std::cout << "Done\n";
            break;
        default:
            break;
    }
    std::cout << "PRIORITY: ";
    switch (priority_) {
        case Priority::Low:
            std::cout << "Low\n";
            break;
        case Priority::Medium:
            std::cout << "Medium\n";
            break;
        case Priority::High:
            std::cout << "High\n";
            break;
        default:
            break;
    }
    std::cout << "CREATED AT: " << createdAt_ << '\n';
}
