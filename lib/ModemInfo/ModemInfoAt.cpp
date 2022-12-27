/*
 * -------------------------------------------------------------
 * -- ModemInfo. 3G/LTE dongle information for OpenWrt LuCi.
 * -------------------------------------------------------------
 * -- Copyright 2022-2023 Vladislav Kadulin <spanky@yandex.ru>
 * -- Licensed to the GNU General Public License v3.0
*/

#include "ModemInfoAt.h"

void ModemInfoAt::parse_json_file() {
    const auto json_f = fopen(this->JSON_FILE_PATH.data(), "rb");
    if (!json_f) {
        set_json_field("error", "No such file or directory");
        exit(EXIT_FAILURE);
    }
    while (!feof(json_f)) {
        fread(this->buff.data(), this->buff.size(), 1, json_f);
    }
    fclose(json_f);
}

bool ModemInfoAt::is_final_result(const char *const response) {
#define STARTS_WITH(a, b) ( strncmp((a), (b), strlen(b)) == 0)
    switch (response[0]) {
        case '+':
            if (STARTS_WITH(&response[1], "CME ERROR:")) {
                return true;
            }
            if (STARTS_WITH(&response[1], "CMS ERROR:")) {
                return true;
            }
            return false;
        case 'B':
            if (strcmp(&response[1], "USY\n") == 0) {
                return true;
            }
            return false;
        case 'E':
            if (strcmp(&response[1], "RROR\n") == 0) {
                return true;
            }
            if (strcmp(&response[1], "RROR READING\n") == 0) {
                return true;
            }
            return false;
        case 'N':
            if (strcmp(&response[1], "O ANSWER\n") == 0) {
                return true;
            }
            if (strcmp(&response[1], "O CARRIER\n") == 0) {
                return true;
            }
            if (strcmp(&response[1], "O DIALTONE\n") == 0) {
                return true;
            }
            return false;
        case 'O':
            if (strcmp(&response[1], "K\n") == 0) {
                return true;
            }
            // fallthrough
        default:
            return false;
    }
}

void ModemInfoAt::strip_cr(char *s) {
    char *from, *to;
    from = to = s;
    while (*from != '\0') {
        if (*from == '\r') {
            from++;
            continue;
        }
        *to++ = *from++;
    }
    *to = '\0';
}

void ModemInfoAt::serial_read() {
    fputs((this->tmp + "\r\n").c_str(), this->modem);
    char *resp = fgets(this->buff.data(), this->buff.size(), this->modem);
    strip_cr(resp);

    tmp.push_back('\n');
    if (resp == tmp) {
        tmp.clear();
    } else {
        tmp.clear();
        this->tmp.append(resp);
    }

    do {
        resp = fgets(this->buff.data(), this->buff.size(), this->modem);
        if (resp == nullptr) {
            resp = strdup("ERROR READING\n");
        }
        strip_cr(resp);
        this->tmp.append(resp);
    } while (!is_final_result(resp));

    for (size_t i = 0; i < this->tmp.size(); ++i) {
        if (i + 1 < this->tmp.size() && this->tmp.at(i) == 'O' && this->tmp.at(i + 1) == 'K') {
            this->tmp.erase(i, 2);
            --i;
        } else if (this->tmp.at(i) == '\r' || this->tmp.at(i) == '\n' || this->tmp.at(i) == '\"') {
            this->tmp.erase(i, 1);
            --i;
        }
    }
}

bool ModemInfoAt::create_connection() {
    int begin = 0;
    int end = JSON_KEYS.size();

    if (MODE == "HARDWARE") {
        end = 6;
    } else if (MODE == "NETWORK") {
        begin = 6;
    } else {}

    parse_json_file();
    this->json = cJSON_Parse(this->buff.data());
    this->modem = fopen(DEVICE_PATH.begin(), "r+b");

    if (modem && json) {
        for (size_t i = begin; i < end; ++i) {
            const auto json_obj = cJSON_GetObjectItem(this->json, JSON_KEYS.at(i).data());
            if (cJSON_IsArray(json_obj)) {
                std::vector<std::string> array_fields{};
                for (size_t j = 0; j < cJSON_GetArraySize(json_obj); ++j) {
                    const auto item = cJSON_GetArrayItem(json_obj, j);
                    this->tmp = cJSON_GetStringValue(item);
                    if (!this->tmp.empty()) {
                        serial_read();
                        array_fields.emplace_back(this->tmp.data());
                    }
                }
                set_json_field(JSON_KEYS.at(i).data(), array_fields);
            } else {
                this->tmp = cJSON_GetStringValue(json_obj);
                if (!this->tmp.empty()) {
                    serial_read();
                    set_json_field(JSON_KEYS.at(i).data(), this->tmp.data());
                }
            }
        }
        fclose(modem);
    } else {
        set_json_field("error", "Couldn't open the AT Device: Access denied");
        exit(EXIT_FAILURE);
    }
    return true;
}
