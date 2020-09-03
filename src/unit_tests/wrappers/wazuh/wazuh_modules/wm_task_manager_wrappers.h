/* Copyright (C) 2015-2020, Wazuh Inc.
 * All rights reserved.
 *
 * This program is free software; you can redistribute it
 * and/or modify it under the terms of the GNU General Public
 * License (version 2) as published by the FSF - Free Software
 * Foundation
 */


#ifndef WM_TASK_MANAGER_WRAPPERS_H
#define WM_TASK_MANAGER_WRAPPERS_H

#include "headers/shared.h"
#include "wazuh_modules/wmodules.h"

int __wrap_wm_task_manager_check_db();

cJSON* __wrap_wm_task_manager_parse_message(const char *msg);

cJSON* __wrap_wm_task_manager_analyze_task(const cJSON *task_object, int *error_code);

cJSON* __wrap_wm_task_manager_parse_response(int error_code, int agent_id, int task_id, char *status);

int __wrap_wm_task_manager_get_task_by_agent_id_and_module(int agent_id, const char *module, char **command, char **status, char **error, int *create_time, int *last_update_time);

int __wrap_wm_task_manager_get_task_by_task_id(int task_id, char **module, char **command, char **status, char **error, int *create_time, int *last_update_time);

void __wrap_wm_task_manager_parse_response_result(cJSON *response, const char *module, const char *command, char *status, char *error, int create_time, int last_update_time, char *request_command);

int __wrap_wm_task_manager_insert_task(int agent_id, const char *module, const char *command);

int __wrap_wm_task_manager_get_task_status(int agent_id, const char *module, char **status);

int __wrap_wm_task_manager_update_task_status(int agent_id, const char *module, const char *status, const char *error);

#endif
