#pragma once

#include "SQLiteTaskRepository.h"
#include "TaskService.h"
#include "httplib.h"

void registerHttpRoutes(httplib::Server& server, TaskService& service, SQLiteTaskRepository& repo);
