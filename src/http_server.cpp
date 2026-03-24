#include "../include/SQLiteTaskRepository.h"
#include "../include/TaskService.h"
#include "../include/httplib.h"
#include <exception>
#include <iostream>
#include <sstream>

#include <string>

#include "../include/nlohmann/json.hpp"
using json = nlohmann::json;

int main() {
    SQLiteTaskRepository repo("tasks.db");
    TaskService service(repo);
    httplib::Server server;

    server.Get("/tasks", [&](const httplib::Request&, httplib::Response& res) {
        try {
            const auto& tasks = repo.getAllTasks();
            json out = json::array();
            for (const auto& to : tasks) {
                out.push_back(
                    {{"id", to.getId()},
                     {"title", to.getTitle()},
                     {"description", to.getDescription()},
                     {"status", SQLiteTaskRepository::toStringStatus(to.getStatus())},
                     {"priority", SQLiteTaskRepository::toStringPriority(to.getPriority())},
                     {"createdAt", to.getCreatedAt()}});
            }
            res.status = 200;
            res.set_content(out.dump(), "application/json");
        } catch (const std::exception& ex) {
            res.set_content(std::string("{\"error\":\"") + ex.what() + "\"}", "application/json");
            res.status = 500;
        }
    });

    server.Get(R"(/tasks/(\d+))", [&](const httplib::Request& req, httplib::Response& res) {
        try {
            const int id = std::stoi(req.matches[1].str());
            const auto& task = repo.getTaskById(id);

            const json out{{"id", task.getId()},
                           {"title", task.getTitle()},
                           {"description", task.getDescription()},
                           {"status", task.getStatus()},
                           {"priority", task.getCreatedAt()}};
            res.status = 200;
            res.set_content(out.dump(), "application/json");
        } catch (const std::out_of_range&) {
            res.status = 404;
            res.set_content(json{{"error", "Task not found"}}.dump(), "application/json");
        } catch (const std::exception& ex) {
            res.status = 500;
            res.set_content(json{{"error", ex.what()}}.dump(), "application/json");
        }
    });

    server.Get("/health", [](const httplib::Request&, httplib::Response& res) {
        res.set_content(R"({"status":"ok"})", "application/json");
        res.status = 200;
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
            const json out{{"id", created.getId()},
                           {"title", created.getTitle()},
                           {"description", created.getDescription()},
                           {"status", created.getStatus()},
                           {"priority", created.getCreatedAt()}};

            res.status = 201;
            res.set_content(out.dump(), "application/json");
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

    constexpr int kPort = 8080;
    std::cout << "HTTP server is listening on http://localhost:" << kPort << '\n';
    if (!server.listen("0.0.0.0", kPort)) {
        std::cerr << "Failed to start server on port " << kPort << '\n';
        return 1;
    }

    return 0;
}
