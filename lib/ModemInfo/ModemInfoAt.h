/*
 * -------------------------------------------------------------
 * -- ModemInfo. 3G/LTE dongle information for OpenWrt LuCi.
 * -------------------------------------------------------------
 * -- Copyright 2022-2023 Vladislav Kadulin <spanky@yandex.ru>
 * -- Licensed to the GNU General Public License v3.0
*/

#ifndef MODEMINFO_MODEMINFOAT_H
#define MODEMINFO_MODEMINFOAT_H

#include <cstring>
#include <array>

#include "ModemInfo.h"

class ModemInfoAt : public ModemInfo {
    FILE *modem;
    cJSON *json;
    std::string tmp;
    std::array<char, 1024> buff{};
    const std::string_view DEVICE_PATH;
    const std::string_view JSON_FILE_PATH;
    static constexpr std::array<std::string_view, 12> JSON_KEYS{
            "device", "imei", "imsi", "iccid", "firmware", "chiptemp",
            "csq", "cops", "creg", "cereg", "cgreg", "any"
    };

    static inline bool is_final_result(const char *);

    static inline void strip_cr(char *);

    void parse_json_file();

    void serial_read();

public:
    ModemInfoAt(const std::string_view &PATH, const std::string_view &FILE,
                const std::string_view &OPTIONS) :
            modem(nullptr),
            json(nullptr),
            DEVICE_PATH(PATH),
            JSON_FILE_PATH(FILE) {
        MODE = OPTIONS;
        tmp.reserve(1024);
    }

    ~ModemInfoAt() override {
        if (this->json != nullptr) {
            cJSON_Delete(this->json);
        }
    };

    bool create_connection() override;
};


#endif //MODEMINFO_MODEMINFOAT_H
