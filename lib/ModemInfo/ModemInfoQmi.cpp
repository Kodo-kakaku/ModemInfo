/*
 * -------------------------------------------------------------
 * -- ModemInfo. 3G/LTE dongle information for OpenWrt LuCi.
 * -------------------------------------------------------------
 * -- Copyright 2022-2023 Vladislav Kadulin <spanky@yandex.ru>
 * -- Licensed to the GNU General Public License v3.0
*/

#include "ModemInfoQmi.h"

bool ModemInfoQmi::signals_handler() {
    g_main_loop_quit(loop);
    if (cancellable) {
        if (!g_cancellable_is_cancelled(cancellable)) {
            g_cancellable_cancel(cancellable);
            return true;
        }
    }
    return false;
}

bool ModemInfoQmi::create_connection() {
    g_autofree gchar *id = g_file_get_path(file);
    if (id) {
        /* Describes a local device file. */
        qmi_device_new(file,
                       cancellable,
                       reinterpret_cast<GAsyncReadyCallback>(&device_new_ready),
                       nullptr);

        g_unix_signal_add(SIGHUP, reinterpret_cast<GSourceFunc>(&signals_handler), nullptr);
        g_unix_signal_add(SIGTERM, reinterpret_cast<GSourceFunc>(&signals_handler), nullptr);
        g_main_loop_run(loop);
        return true;
    }
    return false;
}

void ModemInfoQmi::device_new_ready(GObject *unused, GAsyncResult *res) {
    GError *error = nullptr;
    device = qmi_device_new_finish(res, &error);
    if (!device) {
        set_json_field("error", "No such file or directory");
        clear_connection();
        return;
    }
    /* Open the device */
    qmi_device_open(device,
                    QMI_DEVICE_OPEN_FLAGS_PROXY,
                    15,
                    cancellable,
                    reinterpret_cast<GAsyncReadyCallback>(&device_open_ready),
                    nullptr);
}

void ModemInfoQmi::device_open_ready(QmiDevice *dev, GAsyncResult *res) {
    GError *error = nullptr;
    if (!qmi_device_open_finish(device, res, &error)) {
        set_json_field("error", "Couldn't open the QmiDevice: Access denied");
        clear_connection();
        return;
    }

    const int timeout = 20;
    if (MODE == "HARDWARE" || MODE == "ALL") {
        qmi_device_allocate_client(dev,
                                   QMI_SERVICE_DMS,
                                   QMI_CID_NONE,
                                   timeout,
                                   cancellable,
                                   reinterpret_cast<GAsyncReadyCallback>(&allocate_dms_client_ready),
                                   nullptr);
    }

    if (MODE == "NETWORK" || MODE == "ALL") {
        qmi_device_allocate_client(dev,
                                   QMI_SERVICE_NAS,
                                   QMI_CID_NONE,
                                   timeout,
                                   cancellable,
                                   reinterpret_cast<GAsyncReadyCallback>(&allocate_nas_client_ready),
                                   nullptr);
    }
}

void ModemInfoQmi::allocate_dms_client_ready(QmiDevice *dev, GAsyncResult *res) {
    GError *error = nullptr;
    client_dms = qmi_device_allocate_client_finish(dev, res, &error);
    if (!client_dms) {
        set_json_field("error", "Couldn't create client for the DMS service");
        clear_connection();
        return;
    }
    qmicli_dms_run();
}

void ModemInfoQmi::allocate_nas_client_ready(QmiDevice *dev, GAsyncResult *res) {
    GError *error = nullptr;
    client_nas = qmi_device_allocate_client_finish(dev, res, &error);
    if (!client_nas) {
        set_json_field("error", "Couldn't create client for the NAS service");
        clear_connection();
        return;
    }
    qmicli_nas_run();
}

void ModemInfoQmi::qmicli_dms_run() {
    const int timeout = 10;
    auto dms_client = static_cast<QmiClientDms *>(g_object_ref(client_dms));

    qmi_client_dms_get_ids(dms_client,
                           nullptr,
                           timeout,
                           cancellable,
                           reinterpret_cast<GAsyncReadyCallback>(&get_ids_ready),
                           nullptr);

    qmi_client_dms_get_manufacturer(dms_client,
                                    nullptr,
                                    timeout,
                                    cancellable,
                                    reinterpret_cast<GAsyncReadyCallback>(&get_manufacturer_ready),
                                    nullptr);

    qmi_client_dms_get_model(dms_client,
                             nullptr,
                             timeout,
                             cancellable,
                             reinterpret_cast<GAsyncReadyCallback>(&get_model_ready),
                             nullptr);

    qmi_client_dms_get_software_version(dms_client,
                                        nullptr,
                                        timeout,
                                        cancellable,
                                        reinterpret_cast<GAsyncReadyCallback>(&get_software_version_ready),
                                        nullptr);

    qmi_client_dms_uim_get_iccid(dms_client,
                                 nullptr,
                                 timeout,
                                 cancellable,
                                 reinterpret_cast<GAsyncReadyCallback>(&uim_get_iccid_ready),
                                 nullptr);

    if (MODE != "ALL") { qmicli_async_dms_operation_done(); }
}

void ModemInfoQmi::qmicli_nas_run() {
    const int timeout = 10;
    auto nas_client = static_cast<QmiClientNas *>(g_object_ref(client_nas));

    qmi_client_nas_get_serving_system(nas_client,
                                      nullptr,
                                      timeout,
                                      cancellable,
                                      reinterpret_cast<GAsyncReadyCallback>(&get_serving_system_ready),
                                      nullptr);

    qmi_client_nas_get_signal_info(nas_client,
                                   nullptr,
                                   timeout,
                                   cancellable,
                                   reinterpret_cast<GAsyncReadyCallback>(&get_signal_info_ready),
                                   nullptr);

    qmi_client_nas_get_cell_location_info(nas_client,
                                          nullptr,
                                          timeout,
                                          cancellable,
                                          reinterpret_cast<GAsyncReadyCallback>(&get_cell_location_info_ready),
                                          nullptr);

    qmi_client_nas_get_rf_band_information(nas_client,
                                           nullptr,
                                           timeout,
                                           cancellable,
                                           reinterpret_cast<GAsyncReadyCallback>(&get_rf_band_info_ready),
                                           nullptr);

    qmi_client_nas_get_lte_cphy_ca_info(nas_client,
                                        nullptr,
                                        timeout,
                                        cancellable,
                                        reinterpret_cast<GAsyncReadyCallback>(&get_lte_cphy_ca_info_ready),
                                        nullptr);

    qmicli_async_nas_operation_done();
    if (MODE == "ALL") { qmicli_async_dms_operation_done(); }
}

// DMS
void ModemInfoQmi::get_ids_ready(QmiClientDms *cli, GAsyncResult *res) {
    GError *error = nullptr;
    auto output = qmi_client_dms_get_ids_finish(cli, res, &error);
    if (!output) {
        set_json_field("imei", "-");
        g_error_free(error);
        return;
    }
    if (!qmi_message_dms_get_ids_output_get_result(output, &error)) {
        set_json_field("imei", "-");
        g_error_free(error);
        qmi_message_dms_get_ids_output_unref(output);
        return;
    }

    const gchar *imei = nullptr;
    qmi_message_dms_get_ids_output_get_imei(output, &imei, nullptr);
    set_json_field("imei", imei ? imei : "-");
    qmi_message_dms_get_ids_output_unref(output);
}

void ModemInfoQmi::get_manufacturer_ready(QmiClientDms *cli, GAsyncResult *res) {
    GError *error = nullptr;
    auto output = qmi_client_dms_get_manufacturer_finish(cli, res, &error);
    if (!output) {
        set_json_field("manufacturer", "-");
        g_error_free(error);
        return;
    }
    if (!qmi_message_dms_get_manufacturer_output_get_result(output, &error)) {
        set_json_field("manufacturer", "-");
        g_error_free(error);
        qmi_message_dms_get_manufacturer_output_unref(output);
        return;
    }

    const gchar *str = nullptr;
    qmi_message_dms_get_manufacturer_output_get_manufacturer(output, &str, nullptr);
    set_json_field("manufacturer", str ? str : "-");
    qmi_message_dms_get_manufacturer_output_unref(output);
}

void ModemInfoQmi::get_model_ready(QmiClientDms *cli, GAsyncResult *res) {
    GError *error = nullptr;
    auto output = qmi_client_dms_get_model_finish(cli, res, &error);
    if (!output) {
        set_json_field("model", "-");
        g_error_free(error);
        return;
    }
    if (!qmi_message_dms_get_model_output_get_result(output, &error)) {
        set_json_field("model", "-");
        g_error_free(error);
        qmi_message_dms_get_model_output_unref(output);
        return;
    }

    const gchar *str = nullptr;
    qmi_message_dms_get_model_output_get_model(output, &str, nullptr);
    set_json_field("model", str ? str : "-");
    qmi_message_dms_get_model_output_unref(output);
}

void ModemInfoQmi::get_software_version_ready(QmiClientDms *cli, GAsyncResult *res) {
    GError *error = nullptr;
    auto output = qmi_client_dms_get_software_version_finish(cli, res, &error);
    if (!output) {
        set_json_field("firmware", "-");
        g_error_free(error);
        return;
    }
    if (!qmi_message_dms_get_software_version_output_get_result(output, &error)) {
        set_json_field("firmware", "-");
        g_error_free(error);
        qmi_message_dms_get_software_version_output_unref(output);
        return;
    }

    const gchar *version = nullptr;
    qmi_message_dms_get_software_version_output_get_version(output, &version, nullptr);
    set_json_field("firmware", version ? version : "-");
    qmi_message_dms_get_software_version_output_unref(output);
}

void ModemInfoQmi::uim_get_iccid_ready(QmiClientDms *cli, GAsyncResult *res) {
    GError *error = nullptr;
    auto output = qmi_client_dms_uim_get_iccid_finish(cli, res, &error);
    if (!output) {
        set_json_field("iccid", "-");
        g_error_free(error);
        return;
    }
    if (!qmi_message_dms_uim_get_iccid_output_get_result(output, &error)) {
        set_json_field("iccid", "-");
        g_error_free(error);
        qmi_message_dms_uim_get_iccid_output_unref(output);
        return;
    }

    const gchar *str = nullptr;
    qmi_message_dms_uim_get_iccid_output_get_iccid(output, &str, nullptr);
    set_json_field("iccid", str ? str : "-");
    qmi_message_dms_uim_get_iccid_output_unref(output);
}

// NAS
void ModemInfoQmi::get_serving_system_ready(QmiClientNas *client, GAsyncResult *res) {
    GError *error = nullptr;
    auto output = qmi_client_nas_get_serving_system_finish(client, res, &error);
    if (!output) {
        set_json_field("reg", "-");
        set_json_field("cid", "-");
        set_json_field("lac", "-");
        set_json_field("cops", "-");
        set_json_field("cops_mcc", "-");
        set_json_field("cops_mnc", "-");
        g_error_free(error);
        return;
    }
    if (!qmi_message_nas_get_serving_system_output_get_result(output, &error)) {
        set_json_field("reg", "-");
        set_json_field("cid", "-");
        set_json_field("lac", "-");
        set_json_field("cops", "-");
        set_json_field("cops_mcc", "-");
        set_json_field("cops_mnc", "-");
        g_error_free(error);
        qmi_message_nas_get_serving_system_output_unref(output);
        return;
    }

    QmiNasRegistrationState registration_state;
    QmiNasAttachState cs_attach_state;
    QmiNasAttachState ps_attach_state;
    QmiNasNetworkType selected_network;
    GArray *radio_interfaces = nullptr;
    qmi_message_nas_get_serving_system_output_get_serving_system(
            output,
            &registration_state,
            &cs_attach_state,
            &ps_attach_state,
            &selected_network,
            &radio_interfaces,
            nullptr);
    set_json_field("reg", qmi_nas_registration_state_get_string(registration_state));

    guint16 current_plmn_mcc;
    guint16 current_plmn_mnc;
    const gchar *current_plmn_description = nullptr;
    if (qmi_message_nas_get_serving_system_output_get_current_plmn(
            output,
            &current_plmn_mcc,
            &current_plmn_mnc,
            &current_plmn_description,
            nullptr)) {
        set_json_field("cops", (strcmp(current_plmn_description, "") == 0 ? "-" : current_plmn_description));
        set_json_field("cops_mcc", current_plmn_mcc);
        set_json_field("cops_mnc", current_plmn_mnc);
    } else {
        set_json_field("cops", "-");
        set_json_field("cops_mcc", "-");
        set_json_field("cops_mnc", "-");
    }

    guint32 cid;
    std::string hex;
    if (qmi_message_nas_get_serving_system_output_get_cid_3gpp(output, &cid, nullptr)) {
        sprintf(hex.data(), "%x", cid);
        const auto len = strlen(hex.c_str());
        set_json_field("cid", hex);
        set_json_field("cid_num", cid);

        if (len > 2) {
            set_json_field("cell", &hex[len - 2]);
            hex[len - 2] = '\0';
            set_json_field("enbid", &hex[0]);
        } else {
            set_json_field("cell", "-");
            set_json_field("enbid", "-");
        }
        hex.clear();
    } else {
        set_json_field("cid", "-");
        set_json_field("cell", "-");
        set_json_field("enbid", "-");
        set_json_field("cid_num", "-");
    }

    guint16 lac;
    if (qmi_message_nas_get_serving_system_output_get_lac_3gpp(
            output,
            &lac,
            nullptr)) {
        sprintf(hex.data(), "%x", lac);
        set_json_field("lac", hex);
        set_json_field("lac_num", lac);
    } else {
        set_json_field("lac", "-");
        set_json_field("lac_num", "-");
    }

    qmi_message_nas_get_serving_system_output_unref(output);
}

void ModemInfoQmi::get_signal_info_ready(QmiClientNas *client, GAsyncResult *res) {
    GError *error = nullptr;
    auto output = qmi_client_nas_get_signal_info_finish(client, res, &error);
    if (!output) {
        set_json_field("mode", "-");
        g_error_free(error);
        return;
    }
    if (!qmi_message_nas_get_signal_info_output_get_result(output, &error)) {
        set_json_field("mode", "-");
        g_error_free(error);
        qmi_message_nas_get_signal_info_output_unref(output);
        return;
    }

    const auto csq_per = [](int rssi) {
        return 100 * (1 - (-50 - rssi) / (-50 - -120));
    };

    /* CDMA... */
    gint8 rssi;
    gint16 ecio;
    if (qmi_message_nas_get_signal_info_output_get_cdma_signal_strength(output,
                                                                        &rssi,
                                                                        &ecio,
                                                                        nullptr)) {
        set_json_field("mode", "CDMA");
        set_json_field("rssi", rssi);
        set_json_field("csq_per", csq_per(rssi));
        set_json_field("ecio", -0.5 * ecio);

    }

    /* HDR... */
    gint32 io;
    QmiNasEvdoSinrLevel sinr_level;
    if (qmi_message_nas_get_signal_info_output_get_hdr_signal_strength(output,
                                                                       &rssi,
                                                                       &ecio,
                                                                       &sinr_level,
                                                                       &io,
                                                                       nullptr)) {

        set_json_field("mode", "HDR");
        set_json_field("rssi", rssi);
        set_json_field("csq_per", csq_per(rssi));
        set_json_field("esio", -0.5 * ecio);
        set_json_field("io", io);
        set_json_field("sinr", sinr_level);
    }

    /* GSM */
    if (qmi_message_nas_get_signal_info_output_get_gsm_signal_strength(output,
                                                                       &rssi,
                                                                       nullptr)) {
        set_json_field("mode", "GSM");
        set_json_field("rssi", rssi);
        set_json_field("csq_per", csq_per(rssi));
    }

    /* WCDMA... */
    if (qmi_message_nas_get_signal_info_output_get_wcdma_signal_strength(output,
                                                                         &rssi,
                                                                         &ecio,
                                                                         nullptr)) {
        set_json_field("mode", "WCDMA");
        set_json_field("rssi", rssi);
        set_json_field("csq_per", csq_per(rssi));
        set_json_field("ecio", static_cast<int>(-0.5 * ecio));
    }

    /* LTE... */
    gint16 snr;
    gint8 rsrq;
    gint16 rsrp;
    if (qmi_message_nas_get_signal_info_output_get_lte_signal_strength(output,
                                                                       &rssi,
                                                                       &rsrq,
                                                                       &rsrp,
                                                                       &snr,
                                                                       nullptr)) {

        set_json_field("mode", "LTE");
        set_json_field("rssi", rssi);
        set_json_field("csq_per", csq_per(rssi));
        set_json_field("rsrq", rsrq);
        set_json_field("rsrp", rsrp);
        set_json_field("sinr", static_cast<int>(0.1 * snr));
    }

    /* TDMA */
    gint8 rscp;
    if (qmi_message_nas_get_signal_info_output_get_tdma_signal_strength(output,
                                                                        &rscp,
                                                                        nullptr)) {
        set_json_field("mode", "TDMA");
        set_json_field("rscp", rscp);
    }

    /* TDMA extended */
    gint32 rssi_tdma;
    gint32 rscp_tdma;
    gint32 ecio_tdma;
    gint32 sinr_tdma;
    if (qmi_message_nas_get_signal_info_output_get_tdma_signal_strength_extended
            (output,
             &rssi_tdma,
             &rscp_tdma,
             &ecio_tdma,
             &sinr_tdma,
             nullptr)) {
        set_json_field("mode", "TDMA extended");
        set_json_field("rssi", rssi_tdma);
        set_json_field("csq_per", csq_per(rssi_tdma));
        set_json_field("rscp", rscp_tdma);
        set_json_field("ecio", ecio_tdma);
        set_json_field("sinr", sinr_tdma);
    }

    /* 5G, values of -32768 in EN-DC mode indicate the modem is not connected... */
    if (qmi_message_nas_get_signal_info_output_get_5g_signal_strength(output,
                                                                      &rsrp,
                                                                      &snr,
                                                                      nullptr)) {
        set_json_field("mode", "5G");
        rsrp == (gint16) (0x8000) ? set_json_field("rsrp", "-") : set_json_field("rsrp", rsrp);
        snr == (gint16) (0x8000) ? set_json_field("snr", "-") : set_json_field("snr", static_cast<int>(0.1 * snr));
    }

    /* 5G extended... */
    gint16 rsrq_5g;
    if (qmi_message_nas_get_signal_info_output_get_5g_signal_strength_extended(output,
                                                                               &rsrq_5g,
                                                                               nullptr)) {
        set_json_field("mode", "5G");
        rsrq_5g == (gint16) (0x8000) ? set_json_field("rsrq", "-") : set_json_field("rsrq", rsrq_5g);
    }
    qmi_message_nas_get_signal_info_output_unref(output);
}

void ModemInfoQmi::get_cell_location_info_ready(QmiClientNas *client, GAsyncResult *res) {
    GError *error = nullptr;
    auto *output = qmi_client_nas_get_cell_location_info_finish(client, res, &error);
    if (!output) {
        set_json_field("pci", "-");
        set_json_field("distance", "-");
        set_json_field("latitude", "-");
        set_json_field("longitude", "-");
        g_error_free(error);
        return;
    }
    if (!qmi_message_nas_get_cell_location_info_output_get_result(output, &error)) {
        set_json_field("pci", "-");
        set_json_field("distance", "-");
        set_json_field("latitude", "-");
        set_json_field("longitude", "-");
        g_error_free(error);
        qmi_message_nas_get_cell_location_info_output_unref(output);
        return;
    }

    GArray *array = nullptr;
    GArray *oper = nullptr;
    guint16 absolute_rf_channel_number;
    gboolean ue_in_idle;
    guint16 tracking_area_code;
    guint32 global_cell_id;
    guint16 serving_cell_id;
    guint8 cell_reselection_priority;
    guint8 s_non_intra_search_threshold;
    guint8 serving_cell_low_threshold;
    guint8 s_intra_search_threshold;
    qmi_message_nas_get_cell_location_info_output_get_intrafrequency_lte_info_v2(
            output,
            &ue_in_idle,
            &oper,
            &tracking_area_code,
            &global_cell_id,
            &absolute_rf_channel_number,
            &serving_cell_id,
            &cell_reselection_priority,
            &s_non_intra_search_threshold,
            &serving_cell_low_threshold,
            &s_intra_search_threshold,
            &array, nullptr) ?
    set_json_field("pci", serving_cell_id) : set_json_field("pci", "-");


    guint32 lte_timing_advance;
    if (qmi_message_nas_get_cell_location_info_output_get_lte_info_timing_advance(
            output,
            &lte_timing_advance,
            nullptr)) {
        lte_timing_advance == 0xFFFFFFFF ?
        set_json_field("distance", "-") :
        set_json_field("distance",
                       std::to_string((299792458 * (lte_timing_advance * pow(10, -6)) / 1000) / 2) + " Km");

    }

    guint16 system_id;
    guint16 network_id;
    guint16 base_station_id;
    guint16 reference_pn;
    guint32 latitude;
    guint32 longitude;
    if (qmi_message_nas_get_cell_location_info_output_get_cdma_info(
            output,
            &system_id,
            &network_id,
            &base_station_id,
            &reference_pn,
            &latitude,
            &longitude,
            nullptr)) {
        set_json_field("latitude", (static_cast<double>(latitude) * 0.25) / 3600.0);
        set_json_field("longitude", (static_cast<double>(longitude) * 0.25) / 3600.0);
    } else {
        set_json_field("latitude", "-");
        set_json_field("longitude", "-");
    }

    qmi_message_nas_get_cell_location_info_output_unref(output);
}

void ModemInfoQmi::get_rf_band_info_ready(QmiClientNas *client, GAsyncResult *res) {
    GError *error = nullptr;
    std::vector<double> arfcn;
    auto output = qmi_client_nas_get_rf_band_information_finish(client, res, &error);
    if (!output) {
        set_json_field("arfcn", arfcn);
        g_error_free(error);
        return;
    }
    if (!qmi_message_nas_get_rf_band_information_output_get_result(output, &error)) {
        set_json_field("arfcn", arfcn);
        g_error_free(error);
        qmi_message_nas_get_rf_band_information_output_unref(output);
        return;
    }

    GArray *band_array = nullptr;
    if (!qmi_message_nas_get_rf_band_information_output_get_list(output, &band_array, &error)) {
        set_json_field("arfcn", arfcn);
        g_error_free(error);
        qmi_message_nas_get_rf_band_information_output_unref(output);
        return;
    }

    for (size_t i = 0; band_array && i < band_array->len; ++i) {
        auto info = &g_array_index
        (band_array, QmiMessageNasGetRfBandInformationOutputListElement, i);
        arfcn.push_back(info->active_channel);
    }
    set_json_field("arfcn", arfcn);
    qmi_message_nas_get_rf_band_information_output_unref(output);
}

void ModemInfoQmi::get_lte_cphy_ca_info_ready(QmiClientNas *client, GAsyncResult *res) {
    GError *error = nullptr;
    auto output = qmi_client_nas_get_lte_cphy_ca_info_finish(client, res, &error);
    if (!output) {
        set_json_field("bwdl", "-");
        set_json_field("bwca", "-");
        set_json_field("scc",  "-");
        g_error_free(error);
        return;
    }
    if (!qmi_message_nas_get_lte_cphy_ca_info_output_get_result(output, &error)) {
        set_json_field("bwdl", "-");
        set_json_field("bwca", "-");
        set_json_field("scc",  "-");
        g_error_free(error);
        qmi_message_nas_get_lte_cphy_ca_info_output_unref(output);
        return;
    }

    guint16 pci;
    guint16 channel;
    QmiNasDLBandwidth dl_bandwidth;
    QmiNasActiveBand band;
    QmiNasScellState state;
    GArray *array;

    std::vector<short> bwca;
    std::vector<std::string> s_band;

    const auto qmi_nas_dl_bandwidth = [](QmiNasDLBandwidth dl_bandwidth) {
        short band = 0;
        switch (dl_bandwidth) {
            case QMI_NAS_DL_BANDWIDTH_3:
                band = 3;
                break;
            case QMI_NAS_DL_BANDWIDTH_5:
                band = 5;
                break;
            case QMI_NAS_DL_BANDWIDTH_10:
                band = 10;
                break;
            case QMI_NAS_DL_BANDWIDTH_15:
                band = 15;
                break;
            case QMI_NAS_DL_BANDWIDTH_20:
                band = 20;
                break;
            case QMI_NAS_DL_BANDWIDTH_1_4:
                break;
            case QMI_NAS_DL_BANDWIDTH_INVALID:
                break;
            case QMI_NAS_DL_BANDWIDTH_UNKNOWN:
                break;
            default:
                break;
        }
        return band;
    };

    if (qmi_message_nas_get_lte_cphy_ca_info_output_get_phy_ca_agg_pcell_info(
            output,
            &pci,
            &channel,
            &dl_bandwidth,
            &band,
            nullptr)) {
        dl_bandwidth != 6 && dl_bandwidth != 255 ? set_json_field("bwdl", dl_bandwidth) : set_json_field("bwdl", "-");
        bwca.push_back(qmi_nas_dl_bandwidth(dl_bandwidth));
    }

    if (qmi_message_nas_get_lte_cphy_ca_info_output_get_phy_ca_agg_secondary_cells(
            output,
            &array,
            nullptr)) {
        for (size_t i = 0; i < array->len; ++i) {
            QmiMessageNasGetLteCphyCaInfoOutputPhyCaAggSecondaryCellsSsc *e;
            e = &g_array_index (array, QmiMessageNasGetLteCphyCaInfoOutputPhyCaAggSecondaryCellsSsc, i);
            s_band.emplace_back(qmi_nas_active_band_get_string(e->lte_band));
            bwca.push_back(qmi_nas_dl_bandwidth(dl_bandwidth));
        }
    } else {
        if (qmi_message_nas_get_lte_cphy_ca_info_output_get_phy_ca_agg_scell_info(
                        output,
                        &pci,
                        &channel,
                        &dl_bandwidth,
                        &band,
                        &state,
                        nullptr)) {
            s_band.emplace_back(qmi_nas_active_band_get_string(band));
            bwca.push_back(qmi_nas_dl_bandwidth(dl_bandwidth));
        }
    }

    !s_band.empty() ? set_json_field("scc", s_band) : set_json_field("scc", "-");
    !bwca.empty() ? set_json_field("bwca", std::reduce(bwca.begin(), bwca.end())) : set_json_field("bwca", "-");

    set_json_field("lteca", s_band.size());
    qmi_message_nas_get_lte_cphy_ca_info_output_unref(output);
}


void ModemInfoQmi::clear_connection() {
    if (cancellable) { g_object_unref(cancellable); }
    if (client_nas) { g_object_unref(client_nas); }
    if (client_dms) { g_object_unref(client_dms); }
    if (device) { g_object_unref(device); }
    if (file) { g_object_unref(file); }
    if (loop) {
        g_main_loop_unref(loop);
        g_main_loop_quit(loop);
    }
}

void ModemInfoQmi::qmicli_async_dms_operation_done() {
    qmi_device_release_client(device,
                              client_dms,
                              QMI_DEVICE_RELEASE_CLIENT_FLAGS_RELEASE_CID,
                              10,
                              nullptr,
                              (GAsyncReadyCallback) release_client_ready,
                              nullptr);
}

void ModemInfoQmi::qmicli_async_nas_operation_done() {
    qmi_device_release_client(device,
                              client_nas,
                              QMI_DEVICE_RELEASE_CLIENT_FLAGS_RELEASE_CID,
                              10,
                              nullptr,
                              (GAsyncReadyCallback) release_client_ready,
                              nullptr);
}

void ModemInfoQmi::release_client_ready(QmiDevice *dev, GAsyncResult *res) {
    GError *error = nullptr;
    if (!qmi_device_release_client_finish(dev, res, &error)) { g_error_free(error); }
    clear_connection();
}


