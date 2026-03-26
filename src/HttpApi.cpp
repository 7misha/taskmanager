#include "../include/HttpApi.h"
#include "../include/nlohmann/json.hpp"
#include <algorithm>
#include <cctype>
#include <exception>
#include <string>

using json = nlohmann::json;

namespace {
json toJsonTask(const Task& task) {
    return json{{"id", task.getId()},
                {"title", task.getTitle()},
                {"description", task.getDescription()},
                {"status", SQLiteTaskRepository::toStringStatus(task.getStatus())},
                {"priority", SQLiteTaskRepository::toStringPriority(task.getPriority())},
                {"createdAt", task.getCreatedAt()}};
}
} // namespace

void registerHttpRoutes(httplib::Server& server, TaskService& service, SQLiteTaskRepository& repo) {
    server.Get("/tasks", [&](const httplib::Request&, httplib::Response& res) {
        try {
            const auto& tasks = repo.getAllTasks();
            json out = json::array();
            for (const auto& task : tasks) {
                out.push_back(toJsonTask(task));
            }
            res.status = 200;
            res.set_content(out.dump(), "application/json");
        } catch (const std::exception& ex) {
            res.status = 500;
            res.set_content(json{{"error", ex.what()}}.dump(), "application/json");
        }
    });

    server.Get(R"(/tasks/(\d+))", [&](const httplib::Request& req, httplib::Response& res) {
        try {
            const int id = std::stoi(req.matches[1].str());
            const auto& task = repo.getTaskById(id);
            res.status = 200;
            res.set_content(toJsonTask(task).dump(), "application/json");
        } catch (const std::out_of_range&) {
            res.status = 404;
            res.set_content(json{{"error", "Task not found"}}.dump(), "application/json");
        } catch (const std::exception& ex) {
            res.status = 500;
            res.set_content(json{{"error", ex.what()}}.dump(), "application/json");
        }
    });

    server.Delete(R"(/tasks/(\d+))", [&](const httplib::Request& req, httplib::Response& res) {
        try {
            const int id = std::stoi(req.matches[1].str());
            const Task task = repo.getTaskById(id);
            service.deleteTask(id);

            res.status = 200;
            res.set_content(toJsonTask(task).dump(), "application/json");
        } catch (const std::out_of_range&) {
            res.status = 404;
            res.set_content(json{{"error", "Task not found"}}.dump(), "application/json");
        } catch (const std::exception& ex) {
            res.status = 500;
            res.set_content(json{{"error", ex.what()}}.dump(), "application/json");
        }
    });

    server.Post("/tasks", [&](const httplib::Request& req, httplib::Response& res) {
        try {
            const json body = json::parse(req.body);
            const std::string title = body.at("title").get<std::string>();
            const std::string description = body.at("description").get<std::string>();
            std::string priority = body.at("priority").get<std::string>();
            std::transform(priority.begin(), priority.end(), priority.begin(),
                           [](unsigned char c) { return static_cast<char>(std::toupper(c)); });
            const std::string createdAt = body.at("createdAt").get<std::string>();

            service.addTaskService(title, description, priority, createdAt);

            const auto& tasks = repo.getAllTasks();
            const auto& created = tasks.back();
            res.status = 201;
            res.set_content(toJsonTask(created).dump(), "application/json");
        } catch (const json::parse_error& ex) {
            res.status = 400;
            res.set_content(json{{"error", std::string("Invalid JSON: ") + ex.what()}}.dump(),
                            "application/json");
        } catch (const json::out_of_range& ex) {
            res.status = 400;
            res.set_content(json{{"error", std::string("Missing field: ") + ex.what()}}.dump(),
                            "application/json");
        } catch (const std::invalid_argument& ex) {
            res.status = 400;
            res.set_content(json{{"error", ex.what()}}.dump(), "application/json");
        } catch (const std::exception& ex) {
            res.status = 500;
            res.set_content(json{{"error", ex.what()}}.dump(), "application/json");
        }
    });

    server.Get("/health", [](const httplib::Request&, httplib::Response& res) {
        res.status = 200;
        res.set_content(R"({"status":"ok"})", "application/json");
    });

    server.Patch(R"(/tasks/(\d+))", [&](const httplib::Request& req, httplib::Response& res) {
        try {
            const int id = std::stoi(req.matches[1].str());
            json body = json::parse(req.body);

            auto& task = repo.getTaskById(id);
            if (body.contains("title")) {
                task.updateTitle(body.at("title").get<std::string>());
            }
            if (body.contains("description")) {
                task.updateDescription(body.at("description").get<std::string>());
            }
            if (body.contains("status")) {
                std::string status = body.at("status").get<std::string>();
                std::transform(status.begin(), status.end(), status.begin(),
                               [](unsigned char c) { return static_cast<char>(std::toupper(c)); });

                if (status == "NEW") {
                    task.updateStatus(Status::New);
                } else if (status == "INPROGRESS") {
                    task.updateStatus(Status::InProgress);
                } else if (status == "DONE") {
                    task.updateStatus(Status::Done);
                } else {
                    throw std::invalid_argument("Wrong Status!!!");
                }
            }
            if (body.contains("priority")) {
                std::string priority = body.at("priority").get<std::string>();
                std::transform(priority.begin(), priority.end(), priority.begin(),
                               [](unsigned char c) { return static_cast<char>(std::toupper(c)); });
                if (priority == "LOW") {
                    task.updatePriority(Priority::Low);
                } else if (priority == "MEDIUM") {
                    task.updatePriority(Priority::Medium);
                } else if (priority == "HIGH") {
                    task.updatePriority(Priority::High);
                } else {
                    throw std::invalid_argument("Wrong Priority!!!");
                }
            }
            repo.updateTask(task);

            json out{{"id", task.getId()},
                     {"title", task.getTitle()},
                     {"description", task.getDescription()},
                     {"status", SQLiteTaskRepository::toStringStatus(task.getStatus())},
                     {"priority", SQLiteTaskRepository::toStringPriority(task.getPriority())},
                     {"createdAt", task.getCreatedAt()}};

            res.status = 200;
            res.set_content(out.dump(), "application/json");
        } catch (const json::parse_error& ex) {
            res.status = 400;
            res.set_content(json{{"error", std::string("Invalid JSON: ") + ex.what()}}.dump(),
                            "application/json");
        } catch (const json::out_of_range& ex) {
            res.status = 400;
            res.set_content(
                json{{"error", std::string("Bad field type/value: ") + ex.what()}}.dump(),
                "application/json");
        } catch (const std::out_of_range&) {
            res.status = 404;
            res.set_content(json{{"error", "Task not found"}}.dump(), "application/json");
        } catch (const std::invalid_argument& ex) {
            res.status = 400;
            res.set_content(json{{"error", ex.what()}}.dump(), "application/json");
        } catch (const std::exception& ex) {
            res.status = 500;
            res.set_content(json{{"error", ex.what()}}.dump(), "application/json");
        }
    });
}