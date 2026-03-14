#include "../include/TaskService.h"
#include <sstream>
#include <unordered_set>

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

std::string toStringStatus(Status status) {
    switch (status) {
        case Status::New:
            return "NEW";
        case Status::InProgress:
            return "INPROGRESS";
        case Status::Done:
            return "DONE";
        default:
            return "UNKNOWN";
    }
}

std::string toStringPriority(Priority priority) {
    switch (priority) {
        case Priority::Low:
            return "LOW";
        case Priority::Medium:
            return "MEDIUM";
        case Priority::High:
            return "HIGH";
        default:
            return "UNKNOWN";
    }
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

void TaskService::readFromFile(const std::string& fileName) {
    repo_.getAllTasks().clear();
    std::ifstream in(fileName);
    if (!in.is_open()) {
        return;
    }
    std::string line;
    int maxId = 0;
    std::unordered_set<int> allId;
    while (std::getline(in, line)) {
        std::stringstream ss(line);
        std::vector<std::string> vec;
        std::string buf;
        while (std::getline(ss, buf, '\t')) {
            vec.push_back(buf);
        }
        if (vec.size() != 6) {
            throw std::invalid_argument("Wrong file format!!!");
        }
        int curId = 0;
        for (auto to : vec[0]) {
            if (!std::isdigit(to)) {
                throw std::invalid_argument("Wrong id format in file!!!");
            }
            curId = curId * 10 + (to - '0');
        }
        if (!allId.insert(curId).second) {
            throw std::invalid_argument("Duplicate task id in file!!!");
        }
        repo_.addTask(Task(curId, vec[1], vec[2], getStatus(vec[3]), getPriority(vec[4]), vec[5]));
        maxId = std::max(maxId, curId);
    }
    id_ = maxId + 1;
    in.close();
}

void TaskService::LoadToFile(const std::string& fileName) const {
    std::ofstream out(fileName);
    if (!out.is_open()) {
        throw std::runtime_error("Could not open file for writing.");
    }
    const std::vector<Task>& tasks = repo_.getAllTasks();
    for (const auto& task : tasks) {
        out << task.getId() << '\t' << task.getTitle() << '\t' << task.getDescription() << '\t'
            << toStringStatus(task.getStatus()) << '\t' << toStringPriority(task.getPriority())
            << '\t' << task.getCreatedAt() << '\n';
    }
    out.close();
}
