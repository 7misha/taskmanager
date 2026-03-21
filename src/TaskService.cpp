#include "../include/TaskService.h"
#include <cerrno>
#include <cstdio>
#include <cstring>
#include <fcntl.h>
#include <filesystem>
#include <sstream>
#include <unistd.h>
#include <unordered_set>

TaskService::TaskService(ITaskRepository& repo) : repo_(repo) {
}

std::string normalizeToken(std::string me) {
    for (auto& to : me) {
        to = static_cast<char>(std::toupper(static_cast<unsigned char>(to)));
    }
    return me;
}

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
    repo_.updateTask(now);
}

void TaskService::changeTitle(int id, const std::string& title) {
    if (title.empty()) {
        throw std::invalid_argument("Empty title");
    }
    Task& now = repo_.getTaskById(id);
    now.updateTitle(title);
    repo_.updateTask(now);
}

void TaskService::changeStatus(int id, const std::string& status) {
    Status curStatus = getStatus(status);
    Task& now = repo_.getTaskById(id);
    now.updateStatus(curStatus);
    repo_.updateTask(now);
}

void TaskService::changePriority(int id, const std::string& priority) {
    Priority curPriority = getPriority(priority);
    Task& now = repo_.getTaskById(id);
    now.updatePriority(curPriority);
    repo_.updateTask(now);
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

void TaskService::loadFromFile(const std::string& fileName) {
    std::vector<Task> tempTasks;
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
        if (vec.empty()) {
            continue;
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
        vec[3] = normalizeToken(vec[3]);
        vec[4] = normalizeToken(vec[4]);
        tempTasks.emplace_back(
            Task(curId, vec[1], vec[2], getStatus(vec[3]), getPriority(vec[4]), vec[5]));
        maxId = std::max(maxId, curId);
    }
    repo_.getAllTasks() = std::move(tempTasks);
    id_ = maxId + 1;
    in.close();
}

void TaskService::saveToFile(const std::string& fileName) const {
    namespace fs = std::filesystem;
    fs::path target(fileName);
    fs::path dir = target.parent_path().empty() ? fs::path(".") : target.parent_path();

    std::string tmpl = (dir / ".taskmanager.tmp.XXXXXX").string();
    std::vector<char> buf(tmpl.begin(), tmpl.end());
    buf.push_back('\0');

    int fd = ::mkstemp(buf.data());
    if (fd == -1) {
        throw std::runtime_error("Could not create temporary file in " + dir.string() + ": " +
                                 std::strerror(errno));
    }

    fs::path tempPath(buf.data());
    bool removeTemp = true;
    try {
        std::ofstream out(tempPath, std::ios::binary);
        if (!out) {
            throw std::runtime_error("Could not open temporary file for writing: " +
                                     tempPath.string());
        }
        for (const auto& task : repo_.getAllTasks()) {
            out << task.getId() << '\t' << task.getTitle() << '\t' << task.getDescription() << '\t'
                << toStringStatus(task.getStatus()) << '\t' << toStringPriority(task.getPriority())
                << '\t' << task.getCreatedAt() << '\n';
        }
        out.flush();
        if (!out) {
            throw std::runtime_error("Could not flush temporary file: " + tempPath.string());
        }
        if (::fsync(fd) != 0) {
            throw std::runtime_error("Could not fsync temporary file: " + tempPath.string());
        }
        out.close();
        if (!out) {
            throw std::runtime_error("Could not close temporary file: " + tempPath.string());
        }

        if (::close(fd) != 0) {
            throw std::runtime_error("Could not close temporary file descriptor: " +
                                     tempPath.string());
        }
        fd = -1;
        fs::rename(tempPath, target);
        removeTemp = false;
        int dfd = ::open(dir.c_str(), O_RDONLY);
        if (dfd != -1) {
            ::fsync(dfd);
            ::close(dfd);
        }
    } catch (...) {
        if (fd != -1) {
            ::close(fd);
        }
        if (removeTemp) {
            std::error_code ec;
            fs::remove(tempPath, ec);
        }
        throw;
    }
}
