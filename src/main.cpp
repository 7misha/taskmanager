#include "../include/TaskService.h"
#include <algorithm>
#include <cctype>
#include <iostream>
#include <limits>
#include <string>

namespace {
int readInt() {
    int value = 0;
    std::cin >> value;
    if (!std::cin) {
        throw std::invalid_argument("Expected number");
    }
    std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
    return value;
}

std::string readLine() {
    std::string line;
    std::getline(std::cin, line);
    return line;
}

std::string toUpper(std::string input) {
    std::transform(input.begin(), input.end(), input.begin(),
                   [](unsigned char ch) { return static_cast<char>(std::toupper(ch)); });
    return input;
}

void printMenu() {
    std::cout << "\nTask Manager Menu\n";
    std::cout << "1. Add task\n";
    std::cout << "2. List tasks\n";
    std::cout << "3. Change title\n";
    std::cout << "4. Change description\n";
    std::cout << "5. Change status (NEW/INPROGRESS/DONE)\n";
    std::cout << "6. Change priority (LOW/MEDIUM/HIGH)\n";
    std::cout << "7. Sort by priority\n";
    std::cout << "8. Sort by status\n";
    std::cout << "9. Sort by id\n";
    std::cout << "10. Find by title\n";
    std::cout << "11. Delete task\n";
    std::cout << "0. Exit\n";
    std::cout << "Choose action: ";
}
} // namespace

int main() {
    TaskService service;

    while (true) {
        try {
            printMenu();
            int action = readInt();

            if (action == 0) {
                std::cout << "Bye!\n";
                break;
            }

            if (action == 1) {
                std::cout << "Title: ";
                const std::string title = readLine();
                std::cout << "Description: ";
                const std::string description = readLine();
                std::cout << "Priority (LOW/MEDIUM/HIGH): ";
                const std::string priority = toUpper(readLine());
                std::cout << "Created at (free format): ";
                const std::string createdAt = readLine();

                service.addTaskService(title, description, priority, createdAt);
                std::cout << "Task added.\n";
                service.printAllTasks();
                continue;
            }

            if (action == 2) {
                service.printAllTasks();
                continue;
            }

            if (action == 3) {
                std::cout << "Task id: ";
                const int id = readInt();
                std::cout << "New title: ";
                const std::string title = readLine();
                service.changeTitle(id, title);
                service.printAllTasks();
                continue;
            }

            if (action == 4) {
                std::cout << "Task id: ";
                const int id = readInt();
                std::cout << "New description: ";
                const std::string description = readLine();
                service.changeDescription(id, description);
                service.printAllTasks();
                continue;
            }

            if (action == 5) {
                std::cout << "Task id: ";
                const int id = readInt();
                std::cout << "New status (NEW/INPROGRESS/DONE): ";
                const std::string status = toUpper(readLine());
                service.changeStatus(id, status);
                service.printAllTasks();
                continue;
            }

            if (action == 6) {
                std::cout << "Task id: ";
                const int id = readInt();
                std::cout << "New priority (LOW/MEDIUM/HIGH): ";
                const std::string priority = toUpper(readLine());
                service.changePriority(id, priority);
                service.printAllTasks();
                continue;
            }

            if (action == 7) {
                service.sortByPriority();
                service.printAllTasks();
                continue;
            }

            if (action == 8) {
                service.sortByStatus();
                service.printAllTasks();
                continue;
            }

            if (action == 9) {
                service.sortById();
                service.printAllTasks();
                continue;
            }

            if (action == 10) {
                std::cout << "Title fragment: ";
                const std::string title = readLine();
                const std::vector<int> ids = service.findByTitle(title);

                if (ids.empty()) {
                    std::cout << "No matches.\n";
                } else {
                    std::cout << "Found task ids:";
                    for (const int id : ids) {
                        std::cout << ' ' << id;
                    }
                    std::cout << '\n';
                }
                continue;
            }

            if (action == 11) {
                std::cout << "Task id: ";
                const int id = readInt();
                service.deleteTask(id);
                std::cout << "Task deleted.\n";
                service.printAllTasks();
                continue;
            }

            std::cout << "Unknown action.\n";
        } catch (const std::exception& ex) {
            std::cout << "Error: " << ex.what() << '\n';
            if (!std::cin) {
                std::cin.clear();
                std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
            }
        }
    }

    return 0;
}
