#include "../include/HttpApi.h"
#include "../include/nlohmann/json.hpp"
#include <chrono>
#include <filesystem>
#include <gtest/gtest.h>
#include <string>
#include <thread>

using json = nlohmann::json;

namespace {
std::filesystem::path uniqueDbPath(const std::string& tag) {
    const auto now = std::chrono::steady_clock::now().time_since_epoch().count();
    return std::filesystem::temp_directory_path() /
           ("taskmanager_http_api_" + tag + "_" + std::to_string(now) + ".db");
}

class TestHttpServer final {
public:
    explicit TestHttpServer(const std::filesystem::path& dbPath)
        : dbPath_(dbPath), repo_(dbPath.string()), service_(repo_) {
        registerHttpRoutes(server_, service_, repo_);
        port_ = server_.bind_to_any_port("127.0.0.1");
        if (port_ <= 0) {
            throw std::runtime_error("Failed to bind server port");
        }
        thread_ = std::thread([this] { server_.listen_after_bind(); });
        waitUntilReady();
    }

    ~TestHttpServer() {
        server_.stop();
        if (thread_.joinable()) {
            thread_.join();
        }
        std::error_code ec;
        std::filesystem::remove(dbPath_, ec);
    }

    int port() const {
        return port_;
    }

private:
    void waitUntilReady() const {
        httplib::Client client("127.0.0.1", port_);
        for (int i = 0; i < 50; ++i) {
            if (auto res = client.Get("/health")) {
                if (res->status == 200) {
                    return;
                }
            }
            std::this_thread::sleep_for(std::chrono::milliseconds(20));
        }
        throw std::runtime_error("Server did not become ready in time");
    }

    std::filesystem::path dbPath_;
    SQLiteTaskRepository repo_;
    TaskService service_;
    httplib::Server server_;
    int port_{-1};
    std::thread thread_;
};
} // namespace

TEST(HttpApiTest, HealthEndpointWorks) {
    TestHttpServer server(uniqueDbPath("health"));
    httplib::Client client("127.0.0.1", server.port());

    const auto res = client.Get("/health");
    ASSERT_TRUE(res);
    EXPECT_EQ(res->status, 200);
    EXPECT_EQ(json::parse(res->body).at("status").get<std::string>(), "ok");
}

TEST(HttpApiTest, CreateAndReadTaskFlowWorks) {
    TestHttpServer server(uniqueDbPath("flow"));
    httplib::Client client("127.0.0.1", server.port());

    const json payload{
        {"title", "Buy milk"},
        {"description", "2L"},
        {"priority", "LOW"},
        {"createdAt", "2026-03-25"},
    };
    const auto postRes = client.Post("/tasks", payload.dump(), "application/json");
    ASSERT_TRUE(postRes);
    ASSERT_EQ(postRes->status, 201);

    const json created = json::parse(postRes->body);
    EXPECT_EQ(created.at("title").get<std::string>(), "Buy milk");
    EXPECT_EQ(created.at("status").get<std::string>(), "NEW");
    ASSERT_TRUE(created.contains("id"));
    const int id = created.at("id").get<int>();

    const auto listRes = client.Get("/tasks");
    ASSERT_TRUE(listRes);
    ASSERT_EQ(listRes->status, 200);
    const json list = json::parse(listRes->body);
    ASSERT_TRUE(list.is_array());
    ASSERT_FALSE(list.empty());

    const auto byIdRes = client.Get("/tasks/" + std::to_string(id));
    ASSERT_TRUE(byIdRes);
    ASSERT_EQ(byIdRes->status, 200);
    const json byId = json::parse(byIdRes->body);
    EXPECT_EQ(byId.at("id").get<int>(), id);
    EXPECT_EQ(byId.at("priority").get<std::string>(), "LOW");
}

TEST(HttpApiTest, InvalidPostRequestReturnsBadRequest) {
    TestHttpServer server(uniqueDbPath("invalid_post"));
    httplib::Client client("127.0.0.1", server.port());

    const auto badJsonRes = client.Post("/tasks", "{bad json", "application/json");
    ASSERT_TRUE(badJsonRes);
    EXPECT_EQ(badJsonRes->status, 400);

    const json missingField{{"title", "Only title"}};
    const auto missingFieldRes = client.Post("/tasks", missingField.dump(), "application/json");
    ASSERT_TRUE(missingFieldRes);
    EXPECT_EQ(missingFieldRes->status, 400);
}

TEST(HttpApiTest, DeleteEndpointRemovesTask) {
    TestHttpServer server(uniqueDbPath("delete"));
    httplib::Client client("127.0.0.1", server.port());

    const json payload{
        {"title", "Temp"},
        {"description", "To delete"},
        {"priority", "LOW"},
        {"createdAt", "2026-03-25"},
    };
    const auto postRes = client.Post("/tasks", payload.dump(), "application/json");
    ASSERT_TRUE(postRes);
    ASSERT_EQ(postRes->status, 201);
    const int id = json::parse(postRes->body).at("id").get<int>();

    const auto delRes = client.Delete("/tasks/" + std::to_string(id));
    ASSERT_TRUE(delRes);
    EXPECT_EQ(delRes->status, 200);

    const auto byIdRes = client.Get("/tasks/" + std::to_string(id));
    ASSERT_TRUE(byIdRes);
    EXPECT_EQ(byIdRes->status, 404);
}
