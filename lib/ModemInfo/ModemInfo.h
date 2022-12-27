/*
 * -------------------------------------------------------------
 * -- ModemInfo. 3G/LTE dongle information for OpenWrt LuCi.
 * -------------------------------------------------------------
 * -- Copyright 2022-2023 Vladislav Kadulin <spanky@yandex.ru>
 * -- Licensed to the GNU General Public License v3.0
*/

#ifndef MODEMINFO_MODEMINFO_H
#define MODEMINFO_MODEMINFO_H

#include <iostream>
#include <vector>

extern "C" {
#include <cjson/cJSON.h>
};

class ModemInfo {
    static inline cJSON *info_json;
protected:
    static inline std::string_view MODE;

    static void set_json_field(const std::string_view &, double);

    static void set_json_field(const std::string_view &, const std::string_view &);

    static void set_json_field(const std::string_view &, const std::vector<double> &);

    static void set_json_field(const std::string_view &, const std::vector<std::string> &);

public:
    ModemInfo() { info_json = cJSON_CreateObject(); };

    virtual ~ModemInfo() { cJSON_Delete(info_json); };

    static void print_json(bool);

    virtual bool create_connection() = 0;
};


#endif //MODEMINFO_MODEMINFO_H
