#include "../include/HttpApi.h"
#include <iostream>

int main() {
    try {
        SQLiteTaskRepository repo("tasks.db");
        TaskService service(repo);
        httplib::Server server;
        registerHttpRoutes(server, service, repo);

        constexpr int kPort = 8080;
        std::cout << "HTTP server is listening on http://localhost:" << kPort << '\n';
        if (!server.listen("0.0.0.0", kPort)) {
            std::cerr << "Failed to start server on port " << kPort << '\n';
            return 1;
        }

        return 0;
    } catch (const std::exception& ex) {
        std::cerr << "Fatal error: " << ex.what() << '\n';
        return 1;
    }
}
