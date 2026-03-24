cmake_minimum_required(VERSION 3.16)

project(taskmanager LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

find_package(SQLite3 REQUIRED)

add_library(taskmanager_core
    src/Task.cpp
    src/TaskRepository.cpp
    src/TaskService.cpp
    src/SQLiteTaskRepository.cpp
)

target_include_directories(taskmanager_core
    PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include
)
target_link_libraries(taskmanager_core PUBLIC SQLite::SQLite3)

add_executable(taskmanager_cli src/main.cpp)
target_link_libraries(taskmanager_cli PRIVATE taskmanager_core)
set_target_properties(taskmanager_cli PROPERTIES OUTPUT_NAME taskmanager)

add_executable(taskmanager_http_server src/http_server.cpp)
target_include_directories(taskmanager_http_server PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/include)
target_link_libraries(taskmanager_http_server PRIVATE taskmanager_core)

include(CTest)

if(BUILD_TESTING)
    option(TASKMANAGER_FETCH_GTEST "Download GoogleTest if it is not installed" OFF)

    if(TASKMANAGER_FETCH_GTEST)
        include(FetchContent)
        FetchContent_Declare(
            googletest
            URL https://github.com/google/googletest/archive/refs/tags/v1.14.0.zip
        )
        set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
        FetchContent_MakeAvailable(googletest)
    endif()

    find_package(GTest QUIET)

    set(_gtest_target "")
    set(_gtest_main_target "")

    if(TARGET GTest::gtest)
        set(_gtest_target GTest::gtest)
    elseif(TARGET GTest::GTest)
        set(_gtest_target GTest::GTest)
    endif()

    if(TARGET GTest::gtest_main)
        set(_gtest_main_target GTest::gtest_main)
    elseif(TARGET GTest::Main)
        set(_gtest_main_target GTest::Main)
    endif()

    if(_gtest_target AND _gtest_main_target)
        add_executable(taskmanager_tests
            tests/task_test.cpp
            tests/task_repository_test.cpp
            tests/sqlite_task_repository_test.cpp
            tests/task_service_sqlite_test.cpp
            tests/task_service_test.cpp
        )
        target_link_libraries(taskmanager_tests
            PRIVATE
                taskmanager_core
                ${_gtest_target}
                ${_gtest_main_target}
        )

        if(GTest_DIR)
            get_filename_component(_gtest_cmake_parent "${GTest_DIR}" DIRECTORY)
            get_filename_component(_gtest_lib_dir "${_gtest_cmake_parent}" DIRECTORY)
            set_property(TARGET taskmanager_tests APPEND PROPERTY BUILD_RPATH "${_gtest_lib_dir}")
        endif()

        add_test(NAME taskmanager_tests COMMAND taskmanager_tests)
    else()
        message(WARNING "GoogleTest not found. Tests target will not be generated.")
    endif()
endif()
