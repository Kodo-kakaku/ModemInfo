/*
 * -------------------------------------------------------------
 * -- ModemInfo. 3G/LTE dongle information for OpenWrt LuCi.
 * -------------------------------------------------------------
 * -- Copyright 2022-2023 Vladislav Kadulin <spanky@yandex.ru>
 * -- Licensed to the GNU General Public License v3.0
*/

#ifndef MODEMINFO_MODEMINFOQMI_H
#define MODEMINFO_MODEMINFOQMI_H

#include "ModemInfo.h"
#include <cmath>

extern "C" {
#include <libqmi-glib/libqmi-glib.h>
#include <glib-unix.h>
#include <gio/gio.h>
}

class ModemInfoQmi : public ModemInfo {
    static inline GFile *file;
    static inline GMainLoop *loop;
    static inline GCancellable *cancellable;

    static inline QmiDevice *device;
    static inline QmiClient *client_dms;
    static inline QmiClient *client_nas;

    static bool signals_handler();

    static void device_new_ready(GObject *, GAsyncResult *);

    static void device_open_ready(QmiDevice *, GAsyncResult *);

    static void allocate_dms_client_ready(QmiDevice *dev, GAsyncResult *res);

    static void allocate_nas_client_ready(QmiDevice *dev, GAsyncResult *res);

    static void qmicli_dms_run();

    static void qmicli_nas_run();

    // DMS
    static void get_ids_ready(QmiClientDms *, GAsyncResult *);

    static void get_model_ready(QmiClientDms *, GAsyncResult *);

    static void uim_get_iccid_ready(QmiClientDms *, GAsyncResult *);

    static void get_manufacturer_ready(QmiClientDms *, GAsyncResult *);

    static void get_software_version_ready(QmiClientDms *, GAsyncResult *);

    // NAS
    static void get_signal_info_ready(QmiClientNas *, GAsyncResult *);

    static void get_rf_band_info_ready(QmiClientNas *, GAsyncResult *);

    static void get_serving_system_ready(QmiClientNas *, GAsyncResult *);

    static void get_lte_cphy_ca_info_ready(QmiClientNas *, GAsyncResult *);

    static void get_cell_location_info_ready(QmiClientNas *, GAsyncResult *);

    // END CONNECTION
    static void clear_connection();

    static void qmicli_async_nas_operation_done();

    static void qmicli_async_dms_operation_done();

    static void release_client_ready(QmiDevice *dev, GAsyncResult *res);


public:
    explicit ModemInfoQmi(const std::string_view &PATH, const std::string_view &OPTIONS) {
        loop = g_main_loop_new(nullptr, false);
        file = g_file_new_for_path(PATH.data());
        MODE = OPTIONS;
        cancellable = g_cancellable_new();

        device = nullptr;
        client_dms = nullptr;
        client_nas = nullptr;
    }

    ~ModemInfoQmi() override = default;

    bool create_connection() override;
};

#endif //MODEMINFO_MODEMINFOQMI_H
