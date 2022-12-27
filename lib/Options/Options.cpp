/*
 * -------------------------------------------------------------
 * -- ModemInfo. 3G/LTE dongle information for OpenWrt LuCi.
 * -------------------------------------------------------------
 * -- Copyright 2022-2023 Vladislav Kadulin <spanky@yandex.ru>
 * -- Licensed to the GNU General Public License v3.0
*/

#include "Options.h"

void Options::parse_actions() {
    if (ver) {
        version();
    }
    if (!device) {
        std::cerr << "no device path specified \n";
        exit(EXIT_FAILURE);
    }
    if (at && !file) {
        std::cerr << "json file must be used for AT mode (--file)\n";
        exit(EXIT_FAILURE);
    }

    if (!at && !qmi || at && qmi) {
        std::cerr << "operating mode is not specified (at or qmi)\n";
        exit(EXIT_FAILURE);
    }
}

void Options::version() {
    std::cout << "modeminfo " << VERSION << '\n'
              << "Copyright (C) 2022 Vladislav Kadulin\n"
              << "License GPLv3+: GNU GPL version 3 or later <http://gnu.org/licenses/lgpl-3.0.html>\n"
              << "This is free software: you are free to change and redistribute it.\n"
              << "There is NO WARRANTY, to the extent permitted by law.\n\n";
    exit(EXIT_SUCCESS);
}

std::string_view Options::get_device() {
    return {device};
}


std::string_view Options::get_file() {
    return {file};
}


std::string_view Options::get_mode() {
    return qmi ? "QMI" : "AT";
}

std::string_view Options::get_option() {
    std::string_view opt;
    if (!hardware && !network || hardware && network) {
        opt = "ALL";
    } else if (hardware) {
        opt = "HARDWARE";
    } else if (network) {
        opt = "NETWORK";
    }
    return opt;
}

bool Options::is_pretty() {
    return pretty;
}

