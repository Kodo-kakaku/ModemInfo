/*
 * -------------------------------------------------------------
 * -- ModemInfo. 3G/LTE dongle information for OpenWrt LuCi.
 * -------------------------------------------------------------
 * -- Copyright 2022-2023 Vladislav Kadulin <spanky@yandex.ru>
 * -- Licensed to the GNU General Public License v3.0
*/

#include <memory>

#include "../lib/Options/Options.h"
#include "../lib/ModemInfo/ModemInfo.h"
#include "../lib/ModemInfo/ModemInfoAt.h"
#include "../lib/ModemInfo/ModemInfoQmi.h"


int main(int argc, char **argv) {
    const Options options(argc, argv);
    const std::unique_ptr<ModemInfo> modem{Options::get_mode() == "QMI" ?
                                     static_cast<ModemInfo *>(new ModemInfoQmi(Options::get_device(),
                                                                               Options::get_option())) :
                                     static_cast<ModemInfo *>(new ModemInfoAt(Options::get_device(),
                                                                              Options::get_file(),
                                                                              Options::get_option()))
    };

    modem->create_connection();
    modem->print_json(Options::is_pretty());
    return 0;
}
