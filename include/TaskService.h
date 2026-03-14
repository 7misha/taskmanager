#pragma once

#include "../include/TaskRepository.h"
#include <algorithm>
#include <fstream>
#include <stdexcept>
#include <string>
#include <vector>

class TaskService {
private:
    TaskRepository repo_;
    int id_ = 1;

public:
    void addTaskService(const std::string& title, const std::string& description,
                        const std::string& priority, const std::string& createdAt);

    void changeTitle(int id, const std::string& title);

    void changeDescription(int id, const std::string& description);

    void changeStatus(int id, const std::string& newStatus);

    void changePriority(int id, const std::string& newPriority);

    void sortByPriority();

    void sortByStatus();

    void sortById();

    std::vector<int> findByTitle(const std::string& title) const;

    void deleteTask(int id);

    void printAllTasks() const;

    void readFromFile(const std::string& fileName);

    void LoadToFile(const std::string& fileName) const;
};
