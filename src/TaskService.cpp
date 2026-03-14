#include "../include/TaskService.h"

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
    if (type == "MEDIUM") {
        return Priority::Medium;
    }
    if (type == "HIGH") {
        return Priority::High;
    }
    throw std::out_of_range("Wrong Priority!!!");
}

void TaskService::addTaskService(const std::string& title, const std::string& description,
                                 const std::string& priority, const std::string& createdAt) {
    if (title.empty()) {
        throw std::invalid_argument("Empty title");
    }
    if (description.empty()) {
        throw std::invalid_argument("Empty description");
    }
    if (createdAt.empty()) {
        throw std::invalid_argument("Empty date");
    }
    Priority curPriority = getPriority(priority);
    Task now(id_++, title, description, Status::New, curPriority, createdAt);
    repo_.addTask(now);
}

void TaskService::changeDescription(int id, const std::string& description) {
    if (description.empty()) {
        throw std::invalid_argument("Empty description");
    }
    Task& now = repo_.getTaskById(id);
    now.updateDescription(description);
}

void TaskService::changeTitle(int id, const std::string& title) {
    if (title.empty()) {
        throw std::invalid_argument("Empty title");
    }
    Task& now = repo_.getTaskById(id);
    now.updateTitle(title);
}

void TaskService::changeStatus(int id, const std::string& status) {
    Status curStatus = getStatus(status);
    Task& now = repo_.getTaskById(id);
    now.updateStatus(curStatus);
}

void TaskService::changePriority(int id, const std::string& priority) {
    Priority curPriority = getPriority(priority);
    Task& now = repo_.getTaskById(id);
    now.updatePriority(curPriority);
}

void TaskService::sortByPriority() {
    std::vector<Task>& tasks = repo_.getAllTasks();
    std::sort(tasks.begin(), tasks.end(),
              [&](const auto& A, const auto& B) { return A.getPriority() < B.getPriority(); });
}

void TaskService::sortByStatus() {
    std::vector<Task>& tasks = repo_.getAllTasks();
    std::sort(tasks.begin(), tasks.end(),
              [&](const auto& A, const auto& B) { return A.getStatus() < B.getStatus(); });
}

void TaskService::sortById() {
    std::vector<Task>& tasks = repo_.getAllTasks();
    std::sort(tasks.begin(), tasks.end(),
              [&](const auto& A, const auto& B) { return A.getId() < B.getId(); });
}

std::vector<int> TaskService::findByTitle(const std::string& title) const {
    const std::vector<Task>& tasks = repo_.getAllTasks();
    std::vector<int> answer;
    for (const auto& to : tasks) {
        if (to.getTitle().find(title) != std::string::npos) {
            answer.emplace_back(to.getId());
        }
    }
    return answer;
}

void TaskService::deleteTask(int id) {
    repo_.delTask(id);
}

void TaskService::printAllTasks() const {
    const std::vector<Task>& tasks = repo_.getAllTasks();
    if (tasks.empty()) {
        std::cout << "No tasks yet.\n";
        return;
    }

    for (const auto& task : tasks) {
        task.print();
        std::cout << "--------------------\n";
    }
}
