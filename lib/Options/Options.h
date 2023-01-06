/*
 * -------------------------------------------------------------
 * -- ModemInfo. 3G/LTE dongle information for OpenWrt LuCi.
 * -------------------------------------------------------------
 * -- Copyright 2022-2023 Vladislav Kadulin <spanky@yandex.ru>
 * -- Licensed to the GNU General Public License v3.0
*/

#ifndef MODEMINFO_OPTIONS_H
#define MODEMINFO_OPTIONS_H

#include <iostream>
#include "config.h"

extern "C" {
#include <glib-unix.h>
}

class Options {
    GOptionContext *context{};
    GError *error;
    static inline char *device = nullptr;
    static inline char *file = nullptr;

    static inline bool at = false;
    static inline bool qmi = false;
    static inline bool network = false;
    static inline bool hardware = false;
    static inline bool pretty = false;
    static inline bool ver = false;

    constexpr static GOptionEntry entries[] = {
            {"device",   'd', 0, G_OPTION_ARG_STRING, &device,   "Specify device path",                  "[PATH]"},
            {"file",     'f', 0, G_OPTION_ARG_STRING, &file,     "Specify json file path (only for AT)", "[PATH]"},
            {"at",       'a', 0, G_OPTION_ARG_NONE,   &at,       "Set AT mode",                          nullptr},
            {"qmi",      'q', 0, G_OPTION_ARG_NONE,   &qmi,      "Set QMI mode",                         nullptr},
            {"network",  'n', 0, G_OPTION_ARG_NONE,   &network,  "Get NetWork status",                   nullptr},
            {"hardware", 'h', 0, G_OPTION_ARG_NONE,   &hardware, "Get Hardware info",                    nullptr},
            {"pretty",   'p', 0, G_OPTION_ARG_NONE,   &pretty,   "Pretty print json output",             nullptr},
            {"version",  'v', 0, G_OPTION_ARG_NONE,   &ver,      "Print version",                        nullptr},
            {nullptr}
    };

    static void version();

    static void parse_actions();

public:
    Options(int argc, char **argv) :
            context(g_option_context_new("- Modem Information")),
            error(nullptr) {
        g_option_context_add_main_entries(context, entries, nullptr);
        if (!g_option_context_parse(context, &argc, &argv, &error)) {
            std::cerr << "error: " << error->message << std::endl;
            exit(EXIT_FAILURE);
        }
        parse_actions();
    }

    ~Options() {
        g_option_context_free(context);
    }

    static bool is_pretty();

    static std::string_view get_mode();

    static std::string_view get_file();

    static std::string_view get_device();

    static std::string_view get_option();
};


#endif //MODEMINFO_OPTIONS_H
