/*
 * -------------------------------------------------------------
 * -- ModemInfo. 3G/LTE dongle information for OpenWrt LuCi.
 * -------------------------------------------------------------
 * -- Copyright 2022-2023 Vladislav Kadulin <spanky@yandex.ru>
 * -- Licensed to the GNU General Public License v3.0
*/

#include "ModemInfo.h"

void ModemInfo::set_json_field(const std::string_view &command, const std::string_view &val) {
    cJSON_AddStringToObject(info_json, command.data(), val.data());
}

void ModemInfo::set_json_field(const std::string_view &command, double val) {
    cJSON_AddNumberToObject(info_json, command.data(), val);
}

void ModemInfo::set_json_field(const std::string_view &json_key, const std::vector<double> &json_array) {
    auto commands = cJSON_CreateArray();
    for (const auto &i: json_array) {
        auto command = cJSON_CreateNumber(i);
        cJSON_AddItemToArray(commands, command);
    }
    cJSON_AddItemToObject(info_json, json_key.data(), commands);
}

void ModemInfo::set_json_field(const std::string_view &json_key, const std::vector<std::string> &json_array) {
    auto commands = cJSON_CreateArray();
    for (const auto &i: json_array) {
        auto command = cJSON_CreateString(i.data());
        cJSON_AddItemToArray(commands, command);
    }
    cJSON_AddItemToObject(info_json, json_key.data(), commands);
}

void ModemInfo::print_json(const bool PRETTY) {
    auto json = PRETTY ? cJSON_Print(info_json) : cJSON_PrintUnformatted(info_json);
    std::cout << json << std::endl;
    free(json);
}


