/*
 * Copyright (C) 2015-2020, Wazuh Inc.
 *
 * This program is free software; you can redistribute it
 * and/or modify it under the terms of the GNU General Public
 * License (version 2) as published by the FSF - Free Software
 * Foundation.
 */

#include <stdarg.h>
#include <stddef.h>
#include <setjmp.h>
#include <cmocka.h>
#include <stdio.h>

#include "../../wrappers/wazuh/shared/debug_op_wrappers.h"
#include "../../wrappers/wazuh/shared/time_op_wrappers.h"

#include "../../wazuh_modules/wmodules.h"
#include "../../wazuh_modules/task_manager/wm_task_manager_parsing.h"
#include "../../headers/shared.h"

const char* wm_task_manager_decode_status(char *status);

// Setup / teardown

static int teardown_json(void **state) {
    if (*state) {
        cJSON *json = *state;
        cJSON_Delete(json);
    }
    return 0;
}

// Tests

void test_wm_task_manager_decode_status_done(void **state)
{
    char *status = "Done";

    const char *ret = wm_task_manager_decode_status(status);

    assert_string_equal(ret, "Updated");
}

void test_wm_task_manager_decode_status_in_progress(void **state)
{
    char *status = "In progress";

    const char *ret = wm_task_manager_decode_status(status);

    assert_string_equal(ret, "Updating");
}

void test_wm_task_manager_decode_status_failed(void **state)
{
    char *status = "Failed";

    const char *ret = wm_task_manager_decode_status(status);

    assert_string_equal(ret, "Error");
}

void test_wm_task_manager_decode_status_timeout(void **state)
{
    char *status = "Timeout";

    const char *ret = wm_task_manager_decode_status(status);

    assert_string_equal(ret, "Timeout reached while waiting for the response from the agent");
}

void test_wm_task_manager_decode_status_legacy(void **state)
{
    char *status = "Legacy";

    const char *ret = wm_task_manager_decode_status(status);

    assert_string_equal(ret, "Legacy upgrade: check the result manually since the agent cannot report the result of the task");
}

void test_wm_task_manager_decode_status_unknown(void **state)
{
    char *status = "No status";

    const char *ret = wm_task_manager_decode_status(status);

    assert_string_equal(ret, "Invalid status");
}

void test_wm_task_manager_parse_data_response(void **state)
{
    int error_code = 0;
    int agent_id = 77;
    int task_id = 124;
    char *status = "In progress";

    cJSON *response = wm_task_manager_parse_data_response(error_code, agent_id, task_id, status);

    *state = response;

    assert_non_null(response);
    assert_non_null(cJSON_GetObjectItem(response, "error"));
    assert_int_equal(cJSON_GetObjectItem(response, "error")->valueint, error_code);
    assert_non_null(cJSON_GetObjectItem(response, "message"));
    assert_string_equal(cJSON_GetObjectItem(response, "message")->valuestring, "Success");
    assert_non_null(cJSON_GetObjectItem(response, "agent"));
    assert_int_equal(cJSON_GetObjectItem(response, "agent")->valueint, agent_id);
    assert_non_null(cJSON_GetObjectItem(response, "task_id"));
    assert_int_equal(cJSON_GetObjectItem(response, "task_id")->valueint, task_id);
    assert_non_null(cJSON_GetObjectItem(response, "status"));
    assert_string_equal(cJSON_GetObjectItem(response, "status")->valuestring, status);
}

void test_wm_task_manager_parse_data_response_no_status(void **state)
{
    int error_code = 0;
    int agent_id = 77;
    int task_id = 124;
    char *status = NULL;

    cJSON *response = wm_task_manager_parse_data_response(error_code, agent_id, task_id, status);

    *state = response;

    assert_non_null(response);
    assert_non_null(cJSON_GetObjectItem(response, "error"));
    assert_int_equal(cJSON_GetObjectItem(response, "error")->valueint, error_code);
    assert_non_null(cJSON_GetObjectItem(response, "message"));
    assert_string_equal(cJSON_GetObjectItem(response, "message")->valuestring, "Success");
    assert_non_null(cJSON_GetObjectItem(response, "agent"));
    assert_int_equal(cJSON_GetObjectItem(response, "agent")->valueint, agent_id);
    assert_non_null(cJSON_GetObjectItem(response, "task_id"));
    assert_int_equal(cJSON_GetObjectItem(response, "task_id")->valueint, task_id);
    assert_null(cJSON_GetObjectItem(response, "status"));
}

void test_wm_task_manager_parse_data_response_no_task_id(void **state)
{
    int error_code = 0;
    int agent_id = 77;
    int task_id = OS_INVALID;
    char *status = "In progress";

    cJSON *response = wm_task_manager_parse_data_response(error_code, agent_id, task_id, status);

    *state = response;

    assert_non_null(response);
    assert_non_null(cJSON_GetObjectItem(response, "error"));
    assert_int_equal(cJSON_GetObjectItem(response, "error")->valueint, error_code);
    assert_non_null(cJSON_GetObjectItem(response, "message"));
    assert_string_equal(cJSON_GetObjectItem(response, "message")->valuestring, "Success");
    assert_non_null(cJSON_GetObjectItem(response, "agent"));
    assert_int_equal(cJSON_GetObjectItem(response, "agent")->valueint, agent_id);
    assert_null(cJSON_GetObjectItem(response, "task_id"));
    assert_non_null(cJSON_GetObjectItem(response, "status"));
    assert_string_equal(cJSON_GetObjectItem(response, "status")->valuestring, status);
}

void test_wm_task_manager_parse_data_response_no_agent_id(void **state)
{
    int error_code = 0;
    int agent_id = OS_INVALID;
    int task_id = 124;
    char *status = "In progress";

    cJSON *response = wm_task_manager_parse_data_response(error_code, agent_id, task_id, status);

    *state = response;

    assert_non_null(response);
    assert_non_null(cJSON_GetObjectItem(response, "error"));
    assert_int_equal(cJSON_GetObjectItem(response, "error")->valueint, error_code);
    assert_non_null(cJSON_GetObjectItem(response, "message"));
    assert_string_equal(cJSON_GetObjectItem(response, "message")->valuestring, "Success");
    assert_null(cJSON_GetObjectItem(response, "agent"));
    assert_non_null(cJSON_GetObjectItem(response, "task_id"));
    assert_int_equal(cJSON_GetObjectItem(response, "task_id")->valueint, task_id);
    assert_non_null(cJSON_GetObjectItem(response, "status"));
    assert_string_equal(cJSON_GetObjectItem(response, "status")->valuestring, status);
}

void test_wm_task_manager_parse_data_result(void **state)
{
    cJSON *response = cJSON_CreateObject();

    char *module = "api";
    char *command = "task";
    char *status = "In progress";
    char *error = "Error message";
    int create_time = 123456789;
    char *create_time_timestamp = NULL;
    int last_update = 234567890;
    char *last_update_timestamp = NULL;
    char *req_command = "task_result";

    os_strdup("5/5/20 12:30:55.666", create_time_timestamp);
    os_strdup("5/5/20 12:55:18.789", last_update_timestamp);

    expect_value(__wrap_w_get_timestamp, time, create_time);
    will_return(__wrap_w_get_timestamp, create_time_timestamp);

    expect_value(__wrap_w_get_timestamp, time, last_update);
    will_return(__wrap_w_get_timestamp, last_update_timestamp);

    wm_task_manager_parse_data_result(response, module, command, status, error, create_time, last_update, req_command);

    *state = response;

    assert_non_null(response);
    assert_non_null(cJSON_GetObjectItem(response, "module"));
    assert_string_equal(cJSON_GetObjectItem(response, "module")->valuestring, module);
    assert_non_null(cJSON_GetObjectItem(response, "command"));
    assert_string_equal(cJSON_GetObjectItem(response, "command")->valuestring, command);
    assert_non_null(cJSON_GetObjectItem(response, "status"));
    assert_string_equal(cJSON_GetObjectItem(response, "status")->valuestring, status);
    assert_non_null(cJSON_GetObjectItem(response, "error_msg"));
    assert_string_equal(cJSON_GetObjectItem(response, "error_msg")->valuestring, error);
    assert_non_null(cJSON_GetObjectItem(response, "create_time"));
    assert_string_equal(cJSON_GetObjectItem(response, "create_time")->valuestring, "5/5/20 12:30:55.666");
    assert_non_null(cJSON_GetObjectItem(response, "update_time"));
    assert_string_equal(cJSON_GetObjectItem(response, "update_time")->valuestring, "5/5/20 12:55:18.789");
}

void test_wm_task_manager_parse_data_result_last_update_0(void **state)
{
    cJSON *response = cJSON_CreateObject();

    char *module = "api";
    char *command = "task";
    char *status = "In progress";
    int create_time = 123456789;
    char *create_time_timestamp = NULL;
    int last_update = 0;
    char *req_command = "task_result";

    os_strdup("5/5/20 12:30:55.666", create_time_timestamp);

    expect_value(__wrap_w_get_timestamp, time, create_time);
    will_return(__wrap_w_get_timestamp, create_time_timestamp);

    wm_task_manager_parse_data_result(response, module, command, status, NULL, create_time, last_update, req_command);

    *state = response;

    assert_non_null(response);
    assert_non_null(cJSON_GetObjectItem(response, "module"));
    assert_string_equal(cJSON_GetObjectItem(response, "module")->valuestring, module);
    assert_non_null(cJSON_GetObjectItem(response, "command"));
    assert_string_equal(cJSON_GetObjectItem(response, "command")->valuestring, command);
    assert_non_null(cJSON_GetObjectItem(response, "status"));
    assert_string_equal(cJSON_GetObjectItem(response, "status")->valuestring, status);
    assert_null(cJSON_GetObjectItem(response, "error_msg"));
    assert_non_null(cJSON_GetObjectItem(response, "create_time"));
    assert_string_equal(cJSON_GetObjectItem(response, "create_time")->valuestring, "5/5/20 12:30:55.666");
    assert_non_null(cJSON_GetObjectItem(response, "update_time"));
    assert_string_equal(cJSON_GetObjectItem(response, "update_time")->valuestring, "0");
}

void test_wm_task_manager_parse_data_result_no_last_update(void **state)
{
    cJSON *response = cJSON_CreateObject();

    char *module = "api";
    char *command = "task";
    char *status = "In progress";
    int create_time = 123456789;
    char *create_time_timestamp = NULL;
    int last_update = OS_INVALID;
    char *req_command = "task_result";

    os_strdup("5/5/20 12:30:55.666", create_time_timestamp);

    expect_value(__wrap_w_get_timestamp, time, create_time);
    will_return(__wrap_w_get_timestamp, create_time_timestamp);

    wm_task_manager_parse_data_result(response, module, command, status, NULL, create_time, last_update, req_command);

    *state = response;

    assert_non_null(response);
    assert_non_null(cJSON_GetObjectItem(response, "module"));
    assert_string_equal(cJSON_GetObjectItem(response, "module")->valuestring, module);
    assert_non_null(cJSON_GetObjectItem(response, "command"));
    assert_string_equal(cJSON_GetObjectItem(response, "command")->valuestring, command);
    assert_non_null(cJSON_GetObjectItem(response, "status"));
    assert_string_equal(cJSON_GetObjectItem(response, "status")->valuestring, status);
    assert_null(cJSON_GetObjectItem(response, "error_msg"));
    assert_non_null(cJSON_GetObjectItem(response, "create_time"));
    assert_string_equal(cJSON_GetObjectItem(response, "create_time")->valuestring, "5/5/20 12:30:55.666");
    assert_null(cJSON_GetObjectItem(response, "update_time"));
}

void test_wm_task_manager_parse_data_result_no_create_time(void **state)
{
    cJSON *response = cJSON_CreateObject();

    char *module = "api";
    char *command = "task";
    char *status = "In progress";
    int create_time = OS_INVALID;
    int last_update = 234567890;
    char *last_update_timestamp = NULL;
    char *req_command = "task_result";

    os_strdup("5/5/20 12:55:18.789", last_update_timestamp);

    expect_value(__wrap_w_get_timestamp, time, last_update);
    will_return(__wrap_w_get_timestamp, last_update_timestamp);

    wm_task_manager_parse_data_result(response, module, command, status, NULL, create_time, last_update, req_command);

    *state = response;

    assert_non_null(response);
    assert_non_null(cJSON_GetObjectItem(response, "module"));
    assert_string_equal(cJSON_GetObjectItem(response, "module")->valuestring, module);
    assert_non_null(cJSON_GetObjectItem(response, "command"));
    assert_string_equal(cJSON_GetObjectItem(response, "command")->valuestring, command);
    assert_non_null(cJSON_GetObjectItem(response, "status"));
    assert_string_equal(cJSON_GetObjectItem(response, "status")->valuestring, status);
    assert_null(cJSON_GetObjectItem(response, "error_msg"));
    assert_null(cJSON_GetObjectItem(response, "create_time"));
    assert_non_null(cJSON_GetObjectItem(response, "update_time"));
    assert_string_equal(cJSON_GetObjectItem(response, "update_time")->valuestring, "5/5/20 12:55:18.789");
}

void test_wm_task_manager_parse_data_result_status_upgrade_result(void **state)
{
    cJSON *response = cJSON_CreateObject();

    char *module = "upgrade_module";
    char *command = "upgrade";
    char *status = "Legacy";
    int create_time = 123456789;
    char *create_time_timestamp = NULL;
    int last_update = 234567890;
    char *last_update_timestamp = NULL;
    char *req_command = "upgrade_result";

    os_strdup("5/5/20 12:30:55.666", create_time_timestamp);
    os_strdup("5/5/20 12:55:18.789", last_update_timestamp);

    expect_value(__wrap_w_get_timestamp, time, create_time);
    will_return(__wrap_w_get_timestamp, create_time_timestamp);

    expect_value(__wrap_w_get_timestamp, time, last_update);
    will_return(__wrap_w_get_timestamp, last_update_timestamp);

    wm_task_manager_parse_data_result(response, module, command, status, NULL, create_time, last_update, req_command);

    *state = response;

    assert_non_null(response);
    assert_non_null(cJSON_GetObjectItem(response, "module"));
    assert_string_equal(cJSON_GetObjectItem(response, "module")->valuestring, module);
    assert_non_null(cJSON_GetObjectItem(response, "command"));
    assert_string_equal(cJSON_GetObjectItem(response, "command")->valuestring, command);
    assert_non_null(cJSON_GetObjectItem(response, "status"));
    assert_string_equal(cJSON_GetObjectItem(response, "status")->valuestring, "Legacy upgrade: check the result manually since the agent cannot report the result of the task");
    assert_null(cJSON_GetObjectItem(response, "error_msg"));
    assert_non_null(cJSON_GetObjectItem(response, "create_time"));
    assert_string_equal(cJSON_GetObjectItem(response, "create_time")->valuestring, "5/5/20 12:30:55.666");
    assert_non_null(cJSON_GetObjectItem(response, "update_time"));
    assert_string_equal(cJSON_GetObjectItem(response, "update_time")->valuestring, "5/5/20 12:55:18.789");
}

void test_wm_task_manager_parse_data_result_no_status(void **state)
{
    cJSON *response = cJSON_CreateObject();

    char *module = "upgrade_module";
    char *command = "upgrade";
    char *status = NULL;
    int create_time = 123456789;
    char *create_time_timestamp = NULL;
    int last_update = 234567890;
    char *last_update_timestamp = NULL;
    char *req_command = "upgrade_result";

    os_strdup("5/5/20 12:30:55.666", create_time_timestamp);
    os_strdup("5/5/20 12:55:18.789", last_update_timestamp);

    expect_value(__wrap_w_get_timestamp, time, create_time);
    will_return(__wrap_w_get_timestamp, create_time_timestamp);

    expect_value(__wrap_w_get_timestamp, time, last_update);
    will_return(__wrap_w_get_timestamp, last_update_timestamp);

    wm_task_manager_parse_data_result(response, module, command, status, NULL, create_time, last_update, req_command);

    *state = response;

    assert_non_null(response);
    assert_non_null(cJSON_GetObjectItem(response, "module"));
    assert_string_equal(cJSON_GetObjectItem(response, "module")->valuestring, module);
    assert_non_null(cJSON_GetObjectItem(response, "command"));
    assert_string_equal(cJSON_GetObjectItem(response, "command")->valuestring, command);
    assert_null(cJSON_GetObjectItem(response, "status"));
    assert_null(cJSON_GetObjectItem(response, "error_msg"));
    assert_non_null(cJSON_GetObjectItem(response, "create_time"));
    assert_string_equal(cJSON_GetObjectItem(response, "create_time")->valuestring, "5/5/20 12:30:55.666");
    assert_non_null(cJSON_GetObjectItem(response, "update_time"));
    assert_string_equal(cJSON_GetObjectItem(response, "update_time")->valuestring, "5/5/20 12:55:18.789");
}

void test_wm_task_manager_parse_data_result_no_command(void **state)
{
    cJSON *response = cJSON_CreateObject();

    char *module = "api";
    char *command = NULL;
    char *status = "In progress";
    int create_time = 123456789;
    char *create_time_timestamp = NULL;
    int last_update = 234567890;
    char *last_update_timestamp = NULL;
    char *req_command = "task_result";

    os_strdup("5/5/20 12:30:55.666", create_time_timestamp);
    os_strdup("5/5/20 12:55:18.789", last_update_timestamp);

    expect_value(__wrap_w_get_timestamp, time, create_time);
    will_return(__wrap_w_get_timestamp, create_time_timestamp);

    expect_value(__wrap_w_get_timestamp, time, last_update);
    will_return(__wrap_w_get_timestamp, last_update_timestamp);

    wm_task_manager_parse_data_result(response, module, command, status, NULL, create_time, last_update, req_command);

    *state = response;

    assert_non_null(response);
    assert_non_null(cJSON_GetObjectItem(response, "module"));
    assert_string_equal(cJSON_GetObjectItem(response, "module")->valuestring, module);
    assert_null(cJSON_GetObjectItem(response, "command"));
    assert_non_null(cJSON_GetObjectItem(response, "status"));
    assert_string_equal(cJSON_GetObjectItem(response, "status")->valuestring, status);
    assert_null(cJSON_GetObjectItem(response, "error_msg"));
    assert_non_null(cJSON_GetObjectItem(response, "create_time"));
    assert_string_equal(cJSON_GetObjectItem(response, "create_time")->valuestring, "5/5/20 12:30:55.666");
    assert_non_null(cJSON_GetObjectItem(response, "update_time"));
    assert_string_equal(cJSON_GetObjectItem(response, "update_time")->valuestring, "5/5/20 12:55:18.789");
}

void test_wm_task_manager_parse_data_result_no_module(void **state)
{
    cJSON *response = cJSON_CreateObject();

    char *module = NULL;
    char *command = "task";
    char *status = "In progress";
    int create_time = 123456789;
    char *create_time_timestamp = NULL;
    int last_update = 234567890;
    char *last_update_timestamp = NULL;
    char *req_command = "task_result";

    os_strdup("5/5/20 12:30:55.666", create_time_timestamp);
    os_strdup("5/5/20 12:55:18.789", last_update_timestamp);

    expect_value(__wrap_w_get_timestamp, time, create_time);
    will_return(__wrap_w_get_timestamp, create_time_timestamp);

    expect_value(__wrap_w_get_timestamp, time, last_update);
    will_return(__wrap_w_get_timestamp, last_update_timestamp);

    wm_task_manager_parse_data_result(response, module, command, status, NULL, create_time, last_update, req_command);

    *state = response;

    assert_non_null(response);
    assert_null(cJSON_GetObjectItem(response, "module"));
    assert_non_null(cJSON_GetObjectItem(response, "command"));
    assert_string_equal(cJSON_GetObjectItem(response, "command")->valuestring, command);
    assert_non_null(cJSON_GetObjectItem(response, "status"));
    assert_string_equal(cJSON_GetObjectItem(response, "status")->valuestring, status);
    assert_null(cJSON_GetObjectItem(response, "error_msg"));
    assert_non_null(cJSON_GetObjectItem(response, "create_time"));
    assert_string_equal(cJSON_GetObjectItem(response, "create_time")->valuestring, "5/5/20 12:30:55.666");
    assert_non_null(cJSON_GetObjectItem(response, "update_time"));
    assert_string_equal(cJSON_GetObjectItem(response, "update_time")->valuestring, "5/5/20 12:55:18.789");
}

void test_wm_task_manager_parse_message(void **state)
{
    char *message = "{"
                    "  \"origin\": {"
                    "      \"module\": \"upgrade_module\""
                    "   },"
                    "  \"command\": \"upgrade\","
                    "  \"parameters\": {"
                    "      \"agents\": [1, 2]"
                    "   }"
                    "}";

    cJSON *response = wm_task_manager_parse_message(message);

    *state = response;

    assert_non_null(response);
    assert_int_equal(cJSON_GetArraySize(response), 2);
    cJSON *agent1 = cJSON_GetArrayItem(response, 0);
    assert_non_null(cJSON_GetObjectItem(agent1, "module"));
    assert_string_equal(cJSON_GetObjectItem(agent1, "module")->valuestring, "upgrade_module");
    assert_non_null(cJSON_GetObjectItem(agent1, "command"));
    assert_string_equal(cJSON_GetObjectItem(agent1, "command")->valuestring, "upgrade");
    assert_non_null(cJSON_GetObjectItem(agent1, "agent"));
    assert_int_equal(cJSON_GetObjectItem(agent1, "agent")->valueint, 1);
    cJSON *agent2 = cJSON_GetArrayItem(response, 1);
    assert_non_null(cJSON_GetObjectItem(agent2, "module"));
    assert_string_equal(cJSON_GetObjectItem(agent2, "module")->valuestring, "upgrade_module");
    assert_non_null(cJSON_GetObjectItem(agent2, "command"));
    assert_string_equal(cJSON_GetObjectItem(agent2, "command")->valuestring, "upgrade");
    assert_non_null(cJSON_GetObjectItem(agent2, "agent"));
    assert_int_equal(cJSON_GetObjectItem(agent2, "agent")->valueint, 2);
}

void test_wm_task_manager_parse_message_command_err(void **state)
{
    char *message = "{"
                    "  \"origin\": {"
                    "      \"module\": \"upgrade_module\""
                    "   },"
                    "  \"parameters\": {"
                    "      \"agents\": [1, 2]"
                    "   }"
                    "}";

    expect_string(__wrap__mterror, tag, "wazuh-modulesd:task-manager");
    expect_string(__wrap__mterror, formatted_msg, "(8259): Invalid message. 'command' not found at index '0'");

    cJSON *response = wm_task_manager_parse_message(message);

    assert_null(response);
}

void test_wm_task_manager_parse_message_module_err(void **state)
{
    char *message = "{"
                    "  \"origin\": {"
                    "   },"
                    "  \"command\": \"upgrade\","
                    "  \"parameters\": {"
                    "      \"agents\": [1, 2]"
                    "   }"
                    "}";

    expect_string(__wrap__mterror, tag, "wazuh-modulesd:task-manager");
    expect_string(__wrap__mterror, formatted_msg, "(8259): Invalid message. 'module' not found at index '0'");

    cJSON *response = wm_task_manager_parse_message(message);

    assert_null(response);
}

void test_wm_task_manager_parse_message_invalid_json_err(void **state)
{
    char *message = "unknown json";

    expect_string(__wrap__mterror, tag, "wazuh-modulesd:task-manager");
    expect_string(__wrap__mterror, formatted_msg, "(8257): Error parsing JSON event: 'unknown json'");

    cJSON *response = wm_task_manager_parse_message(message);

    assert_null(response);
}

int main(void) {
    const struct CMUnitTest tests[] = {
        // wm_task_manager_decode_status
        cmocka_unit_test(test_wm_task_manager_decode_status_done),
        cmocka_unit_test(test_wm_task_manager_decode_status_in_progress),
        cmocka_unit_test(test_wm_task_manager_decode_status_failed),
        cmocka_unit_test(test_wm_task_manager_decode_status_timeout),
        cmocka_unit_test(test_wm_task_manager_decode_status_legacy),
        cmocka_unit_test(test_wm_task_manager_decode_status_unknown),
        // wm_task_manager_parse_data_response
        cmocka_unit_test_teardown(test_wm_task_manager_parse_data_response, teardown_json),
        cmocka_unit_test_teardown(test_wm_task_manager_parse_data_response_no_status, teardown_json),
        cmocka_unit_test_teardown(test_wm_task_manager_parse_data_response_no_task_id, teardown_json),
        cmocka_unit_test_teardown(test_wm_task_manager_parse_data_response_no_agent_id, teardown_json),
        // wm_task_manager_parse_data_result
        cmocka_unit_test_teardown(test_wm_task_manager_parse_data_result, teardown_json),
        cmocka_unit_test_teardown(test_wm_task_manager_parse_data_result_last_update_0, teardown_json),
        cmocka_unit_test_teardown(test_wm_task_manager_parse_data_result_no_last_update, teardown_json),
        cmocka_unit_test_teardown(test_wm_task_manager_parse_data_result_no_create_time, teardown_json),
        cmocka_unit_test_teardown(test_wm_task_manager_parse_data_result_status_upgrade_result, teardown_json),
        cmocka_unit_test_teardown(test_wm_task_manager_parse_data_result_no_status, teardown_json),
        cmocka_unit_test_teardown(test_wm_task_manager_parse_data_result_no_command, teardown_json),
        cmocka_unit_test_teardown(test_wm_task_manager_parse_data_result_no_module, teardown_json),
        // wm_task_manager_parse_message
        cmocka_unit_test_teardown(test_wm_task_manager_parse_message, teardown_json),
        cmocka_unit_test(test_wm_task_manager_parse_message_command_err),
        cmocka_unit_test(test_wm_task_manager_parse_message_module_err),
        cmocka_unit_test(test_wm_task_manager_parse_message_invalid_json_err)
    };
    return cmocka_run_group_tests(tests, NULL, NULL);
}