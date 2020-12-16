/*
 * Copyright (C) 2019,2020 by Sukchan Lee <acetcom@gmail.com>
 *
 * This file is part of Open5GS.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Affero General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <https://www.gnu.org/licenses/>.
 */

#include "npcf-build.h"

ogs_sbi_request_t *amf_npcf_am_policy_control_build_create(
        amf_ue_t *amf_ue, void *data)
{
    ogs_sbi_message_t message;
    ogs_sbi_header_t header;
    ogs_sbi_request_t *request = NULL;
    ogs_sbi_server_t *server = NULL;

    OpenAPI_policy_association_request_t PolicyAssociationRequest;

    int i, j;
    OpenAPI_lnode_t *node = NULL;

    OpenAPI_list_t *AllowedSnssais = NULL;
    OpenAPI_user_location_t ueLocation;
    OpenAPI_ambr_t ueAmbr;

    ogs_assert(amf_ue);
    ogs_assert(amf_ue->supi);

    memset(&message, 0, sizeof(message));
    message.h.method = (char *)OGS_SBI_HTTP_METHOD_POST;
    message.h.service.name =
        (char *)OGS_SBI_SERVICE_NAME_NPCF_AM_POLICY_CONTROL;
    message.h.api.version = (char *)OGS_SBI_API_V1;
    message.h.resource.component[0] = (char *)OGS_SBI_RESOURCE_NAME_POLICIES;

    memset(&PolicyAssociationRequest, 0, sizeof(PolicyAssociationRequest));

    server = ogs_list_first(&ogs_sbi_self()->server_list);
    ogs_assert(server);

    memset(&header, 0, sizeof(header));
    header.service.name = (char *)OGS_SBI_SERVICE_NAME_NAMF_CALLBACK;
    header.api.version = (char *)OGS_SBI_API_V1;
    header.resource.component[0] = amf_ue->supi;
    header.resource.component[1] =
            (char *)OGS_SBI_RESOURCE_NAME_AM_POLICY_NOTIFY;
    PolicyAssociationRequest.notification_uri =
                        ogs_sbi_server_uri(server, &header);
    ogs_assert(PolicyAssociationRequest.notification_uri);

    PolicyAssociationRequest.supi = amf_ue->supi;

    if (amf_ue->num_of_msisdn) {
        if (amf_ue->msisdn[0]) {
            PolicyAssociationRequest.gpsi = ogs_msprintf("%s-%s",
                        OGS_ID_GPSI_TYPE_MSISDN, amf_ue->msisdn[0]);
        }
    }

    PolicyAssociationRequest.access_type = amf_ue->nas.access_type; 
    PolicyAssociationRequest.pei = amf_ue->pei;

    memset(&ueLocation, 0, sizeof(ueLocation));
    ueLocation.nr_location = ogs_sbi_build_nr_location(
            &amf_ue->tai, &amf_ue->nr_cgi);
    ogs_assert(ueLocation.nr_location);
    ueLocation.nr_location->ue_location_timestamp =
        ogs_sbi_gmtime_string(amf_ue->ue_location_timestamp);
    PolicyAssociationRequest.user_loc = &ueLocation;

    PolicyAssociationRequest.time_zone =
        ogs_sbi_timezone_string(ogs_timezone());

    PolicyAssociationRequest.serving_plmn =
        ogs_sbi_build_plmn_id_nid(&amf_ue->tai.plmn_id);

    PolicyAssociationRequest.rat_type = amf_ue_rat_type(amf_ue);

    memset(&ueAmbr, 0, sizeof(ueAmbr));
    if (OGS_SBI_FEATURES_IS_SET(amf_ue->am_policy_control_features,
                OGS_SBI_NPCF_AM_POLICY_CONTROL_UE_AMBR_AUTHORIZATION)) {
        if (amf_ue->subscribed_ue_ambr.uplink) {
            ueAmbr.uplink = ogs_sbi_bitrate_to_string(
                amf_ue->subscribed_ue_ambr.uplink, OGS_SBI_BITRATE_KBPS);
        }
        if (amf_ue->subscribed_ue_ambr.downlink) {
            ueAmbr.downlink = ogs_sbi_bitrate_to_string(
                amf_ue->subscribed_ue_ambr.downlink, OGS_SBI_BITRATE_KBPS);
        }
        if (ueAmbr.downlink || ueAmbr.uplink) {
            PolicyAssociationRequest.ue_ambr = &ueAmbr;
        }
    }

    AllowedSnssais = OpenAPI_list_create();
    ogs_assert(AllowedSnssais);

    for (i = 0; i < amf_self()->num_of_plmn_support; i++) {
        if (memcmp(&amf_ue->tai.plmn_id,
                &amf_self()->plmn_support[i].plmn_id, OGS_PLMN_ID_LEN) != 0)
            continue;
        for (j = 0; j < amf_self()->plmn_support[i].num_of_s_nssai; j++) {
            struct OpenAPI_snssai_s *Snssai = ogs_calloc(1, sizeof(*Snssai));
            ogs_assert(Snssai);

            Snssai->sst = amf_self()->plmn_support[i].s_nssai[j].sst;
            Snssai->sd = ogs_s_nssai_sd_to_string(
                amf_self()->plmn_support[i].s_nssai[j].sd);

            OpenAPI_list_add(AllowedSnssais, Snssai);
        }
    }

    if (AllowedSnssais->count)
        PolicyAssociationRequest.allowed_snssais = AllowedSnssais;
    else
        OpenAPI_list_free(AllowedSnssais);

    PolicyAssociationRequest.guami = ogs_sbi_build_guami(amf_ue->guami);

    PolicyAssociationRequest.service_name =
        (char *)OGS_SBI_SERVICE_NAME_NAMF_CALLBACK;

    PolicyAssociationRequest.supp_feat =
        ogs_uint64_to_string(amf_ue->am_policy_control_features);
    ogs_assert(PolicyAssociationRequest.supp_feat);

    message.PolicyAssociationRequest = &PolicyAssociationRequest;

    request = ogs_sbi_build_request(&message);
    ogs_assert(request);

    ogs_free(PolicyAssociationRequest.notification_uri);
    ogs_free(PolicyAssociationRequest.supp_feat);

    if (PolicyAssociationRequest.gpsi)
        ogs_free(PolicyAssociationRequest.gpsi);

    if (ueLocation.nr_location) {
        if (ueLocation.nr_location->ue_location_timestamp)
            ogs_free(ueLocation.nr_location->ue_location_timestamp);
        ogs_sbi_free_nr_location(ueLocation.nr_location);
    }
    if (PolicyAssociationRequest.time_zone)
        ogs_free(PolicyAssociationRequest.time_zone);

    if (PolicyAssociationRequest.serving_plmn)
        ogs_sbi_free_plmn_id_nid(PolicyAssociationRequest.serving_plmn);

    if (ueAmbr.downlink) ogs_free(ueAmbr.downlink);
    if (ueAmbr.uplink) ogs_free(ueAmbr.uplink);

    OpenAPI_list_for_each(PolicyAssociationRequest.allowed_snssais, node) {
        struct OpenAPI_snssai_s *Snssai = node->data;
        if (Snssai) {
            if (Snssai->sd)
                ogs_free(Snssai->sd);
            ogs_free(Snssai);
        }
    }
    OpenAPI_list_free(PolicyAssociationRequest.allowed_snssais);

    if (PolicyAssociationRequest.guami)
        ogs_sbi_free_guami(PolicyAssociationRequest.guami);

    return request;
}
