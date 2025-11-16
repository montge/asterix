/* packet-asterix.c
 *
 * Wireshark ASTERIX Dissector Plugin
 * ASTERIX Protocol Dissector Implementation
 *
 * ASTERIX (All Purpose STructured EUROCONTROL SuRveillance Information EXchange)
 * is a binary protocol used for Air Traffic Management (ATM) surveillance data.
 *
 * This dissector supports Wireshark 4.0.0+
 *
 * Copyright (C) 2025
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

#include "config.h"
#include <epan/packet.h>
#include <epan/prefs.h>
#include <epan/expert.h>
#include <epan/proto_data.h>
#include <wsutil/str_util.h>

#include "packet-asterix.h"

/* Plugin version */
#define ASTERIX_PLUGIN_VERSION "1.0.0"

/* Protocol handles */
static int proto_asterix = -1;
static dissector_handle_t asterix_handle;

/* Subtree handles */
static gint ett_asterix = -1;
static gint ett_asterix_datablock = -1;
static gint ett_asterix_record = -1;
static gint ett_asterix_fspec = -1;
static gint ett_asterix_item = -1;

/* Header field handles */
static int hf_asterix_category = -1;
static int hf_asterix_length = -1;
static int hf_asterix_fspec = -1;
static int hf_asterix_fspec_byte = -1;
static int hf_asterix_record = -1;

/* Expert info handles */
static expert_field ei_asterix_invalid_length = EI_INIT;
static expert_field ei_asterix_invalid_category = EI_INIT;
static expert_field ei_asterix_truncated = EI_INIT;

/* Preferences */
static gboolean pref_heuristic_enabled = TRUE;
static gboolean pref_verbose_info = TRUE;

/* ASTERIX category value_string */
static const value_string asterix_category_vals[] = {
    { 1,   "CAT001 - Monoradar Target Reports" },
    { 2,   "CAT002 - Monoradar Target Messages" },
    { 4,   "CAT004 - Safety Net Messages" },
    { 8,   "CAT008 - Monoradar Derived Weather Information" },
    { 10,  "CAT010 - Monoradar Service Messages" },
    { 11,  "CAT011 - Monoradar Alert Messages" },
    { 15,  "CAT015 - INCS Target Reports" },
    { 19,  "CAT019 - Multilateration System Status" },
    { 20,  "CAT020 - Multilateration Target Reports" },
    { 21,  "CAT021 - ADS-B Target Reports" },
    { 23,  "CAT023 - CNS/ATM Ground Station Status" },
    { 25,  "CAT025 - CNS/ATM Service Status Reports" },
    { 30,  "CAT030 - ARTAS FPSDI" },
    { 31,  "CAT031 - ARTAS Picture Integrity" },
    { 32,  "CAT032 - ARTAS Miniplan" },
    { 34,  "CAT034 - Monoradar Service Messages" },
    { 48,  "CAT048 - Monoradar Target Reports" },
    { 62,  "CAT062 - System Track Data" },
    { 63,  "CAT063 - Sensor Status Messages" },
    { 65,  "CAT065 - SDPS Service Status Messages" },
    { 205, "CAT205 - Area Proximity Warnings" },
    { 240, "CAT240 - Radar Video Transmission" },
    { 247, "CAT247 - Fixed Transponder Information" },
    { 252, "CAT252 - ARTAS Operational Service Configuration" },
    { 0,   NULL }
};

/**
 * Parse FSPEC (Field Specification)
 *
 * FSPEC indicates which data items are present in the record.
 * Each bit represents a data item (1 = present, 0 = absent).
 * If bit 1 (LSB) of a byte is set, another FSPEC byte follows.
 *
 * @param tvb    The packet data buffer
 * @param offset Current offset in the buffer
 * @param tree   Protocol tree to add FSPEC to
 * @return       Number of FSPEC bytes parsed
 */
static guint
parse_fspec(tvbuff_t *tvb, guint offset, proto_tree *tree)
{
    proto_tree *fspec_tree;
    proto_item *ti;
    guint fspec_len = 0;
    guint8 fspec_byte;
    gboolean fx_bit;

    ti = proto_tree_add_item(tree, hf_asterix_fspec, tvb, offset, -1, ENC_NA);
    fspec_tree = proto_item_add_subtree(ti, ett_asterix_fspec);

    do {
        /* Check if we have enough data */
        if (!tvb_bytes_exist(tvb, offset + fspec_len, 1)) {
            break;
        }

        fspec_byte = tvb_get_guint8(tvb, offset + fspec_len);
        fx_bit = (fspec_byte & 0x01) != 0;

        proto_tree_add_uint_format(fspec_tree, hf_asterix_fspec_byte, tvb,
                                   offset + fspec_len, 1, fspec_byte,
                                   "FSPEC Byte %d: 0x%02x %s",
                                   fspec_len + 1, fspec_byte,
                                   fx_bit ? "(FX=1, more bytes follow)" : "(FX=0, last byte)");

        fspec_len++;
    } while (fx_bit && fspec_len < 32); /* Safety limit */

    proto_item_set_len(ti, fspec_len);
    return fspec_len;
}

/**
 * Dissect ASTERIX Data Block
 *
 * Data Block Structure:
 *   - Category (1 byte)
 *   - Length (2 bytes, big-endian)
 *   - Data Records (variable)
 *
 * @param tvb    The packet data buffer
 * @param pinfo  Packet info structure
 * @param tree   Protocol tree
 * @param offset Current offset in the buffer
 * @return       Offset after this data block
 */
static guint
dissect_asterix_datablock(tvbuff_t *tvb, packet_info *pinfo, proto_tree *tree, guint offset)
{
    proto_tree *datablock_tree;
    proto_tree *record_tree;
    proto_item *ti;
    proto_item *length_item;
    guint8 category;
    guint16 length;
    guint block_end;
    guint record_num = 1;

    /* Check minimum size (category + length = 3 bytes) */
    if (!tvb_bytes_exist(tvb, offset, 3)) {
        return offset;
    }

    /* Parse category */
    category = tvb_get_guint8(tvb, offset);

    /* Parse length (includes category and length fields) */
    length = tvb_get_ntohs(tvb, offset + 1);

    /* Validate length */
    if (length < 3) {
        expert_add_info_format(pinfo, tree, &ei_asterix_invalid_length,
                              "Invalid data block length: %u (minimum is 3)", length);
        return offset + 3;
    }

    /* Check if we have the complete data block */
    if (!tvb_bytes_exist(tvb, offset, length)) {
        expert_add_info_format(pinfo, tree, &ei_asterix_truncated,
                              "Truncated data block: expected %u bytes, only %u available",
                              length, tvb_captured_length_remaining(tvb, offset));
        return offset + tvb_captured_length_remaining(tvb, offset);
    }

    /* Create data block subtree */
    ti = proto_tree_add_item(tree, proto_asterix, tvb, offset, length, ENC_NA);
    proto_item_append_text(ti, ", Category %u, %u bytes", category, length);
    datablock_tree = proto_item_add_subtree(ti, ett_asterix_datablock);

    /* Add category field */
    proto_tree_add_uint(datablock_tree, hf_asterix_category, tvb, offset, 1, category);

    /* Add length field */
    length_item = proto_tree_add_uint(datablock_tree, hf_asterix_length, tvb, offset + 1, 2, length);

    /* Validate category */
    if (category == 0 || category > 255) {
        expert_add_info_format(pinfo, length_item, &ei_asterix_invalid_category,
                              "Invalid ASTERIX category: %u", category);
    }

    /* Update info column */
    if (pref_verbose_info) {
        col_append_fstr(pinfo->cinfo, COL_INFO, " CAT%03u(%ub)", category, length);
    }

    block_end = offset + length;
    offset += 3; /* Skip category and length */

    /* Parse data records */
    while (offset < block_end) {
        guint record_start = offset;
        guint fspec_len;

        /* Check for minimum record size (at least 1 FSPEC byte) */
        if (!tvb_bytes_exist(tvb, offset, 1)) {
            break;
        }

        /* Create record subtree */
        ti = proto_tree_add_item(datablock_tree, hf_asterix_record, tvb, offset, -1, ENC_NA);
        proto_item_append_text(ti, " #%u", record_num);
        record_tree = proto_item_add_subtree(ti, ett_asterix_record);

        /* Parse FSPEC */
        fspec_len = parse_fspec(tvb, offset, record_tree);
        offset += fspec_len;

        /* TODO: Parse data items based on FSPEC */
        /* For MVP, we just skip to the next record boundary */
        /* This will be implemented in Phase 2 */

        /* For now, consume remaining bytes in this data block as "Data Items" */
        guint remaining = block_end - offset;
        if (remaining > 0) {
            proto_tree_add_item(record_tree, hf_asterix_record, tvb, offset, remaining, ENC_NA);
            offset += remaining;
        }

        proto_item_set_len(ti, offset - record_start);
        record_num++;

        /* Safety check to prevent infinite loops */
        if (offset <= record_start) {
            break;
        }
    }

    return block_end;
}

/**
 * Main ASTERIX Dissector
 *
 * @param tvb  The packet data buffer
 * @param pinfo Packet info structure
 * @param tree Protocol tree
 * @param data User data (unused)
 * @return     Number of bytes dissected
 */
static int
dissect_asterix(tvbuff_t *tvb, packet_info *pinfo, proto_tree *tree, void *data _U_)
{
    guint offset = 0;
    guint tvb_len = tvb_captured_length(tvb);

    /* Set protocol column */
    col_set_str(pinfo->cinfo, COL_PROTOCOL, "ASTERIX");
    col_clear(pinfo->cinfo, COL_INFO);

    /* Parse all data blocks in this packet */
    while (offset < tvb_len) {
        guint block_start = offset;
        offset = dissect_asterix_datablock(tvb, pinfo, tree, offset);

        /* Safety check */
        if (offset <= block_start) {
            break;
        }
    }

    return tvb_len;
}

/**
 * Heuristic Dissector
 *
 * Attempts to identify ASTERIX packets without relying on port numbers.
 *
 * @param tvb  The packet data buffer
 * @param pinfo Packet info structure
 * @param tree Protocol tree
 * @param data User data (unused)
 * @return     TRUE if packet is ASTERIX, FALSE otherwise
 */
static gboolean
dissect_asterix_heuristic(tvbuff_t *tvb, packet_info *pinfo, proto_tree *tree, void *data)
{
    guint8 category;
    guint16 length;

    /* Check minimum size (category + length) */
    if (tvb_captured_length(tvb) < 3) {
        return FALSE;
    }

    /* Check if first byte is valid ASTERIX category (1-255, not 0) */
    category = tvb_get_guint8(tvb, 0);
    if (category == 0) {
        return FALSE;
    }

    /* Check if length field is reasonable */
    length = tvb_get_ntohs(tvb, 1);
    if (length < 3 || length > tvb_captured_length(tvb)) {
        return FALSE;
    }

    /* Check if category is in our known list (increases confidence) */
    if (try_val_to_str(category, asterix_category_vals) == NULL) {
        /* Unknown category - might still be ASTERIX, but less confident */
        /* Accept it anyway for now */
    }

    /* High confidence - call full dissector */
    dissect_asterix(tvb, pinfo, tree, data);
    return TRUE;
}

/**
 * Register Protocol
 */
void
proto_register_asterix(void)
{
    static hf_register_info hf[] = {
        { &hf_asterix_category,
          { "Category", "asterix.category",
            FT_UINT8, BASE_DEC, VALS(asterix_category_vals), 0x0,
            "ASTERIX Category", HFILL }
        },
        { &hf_asterix_length,
          { "Length", "asterix.length",
            FT_UINT16, BASE_DEC, NULL, 0x0,
            "Data Block Length (bytes)", HFILL }
        },
        { &hf_asterix_fspec,
          { "FSPEC", "asterix.fspec",
            FT_NONE, BASE_NONE, NULL, 0x0,
            "Field Specification", HFILL }
        },
        { &hf_asterix_fspec_byte,
          { "FSPEC Byte", "asterix.fspec.byte",
            FT_UINT8, BASE_HEX, NULL, 0x0,
            "Field Specification Byte", HFILL }
        },
        { &hf_asterix_record,
          { "Data Record", "asterix.record",
            FT_BYTES, BASE_NONE, NULL, 0x0,
            "ASTERIX Data Record", HFILL }
        }
    };

    static gint *ett[] = {
        &ett_asterix,
        &ett_asterix_datablock,
        &ett_asterix_record,
        &ett_asterix_fspec,
        &ett_asterix_item
    };

    static ei_register_info ei[] = {
        { &ei_asterix_invalid_length,
          { "asterix.invalid_length", PI_MALFORMED, PI_ERROR,
            "Invalid data block length", EXPFILL }
        },
        { &ei_asterix_invalid_category,
          { "asterix.invalid_category", PI_MALFORMED, PI_WARN,
            "Invalid or unknown ASTERIX category", EXPFILL }
        },
        { &ei_asterix_truncated,
          { "asterix.truncated", PI_MALFORMED, PI_WARN,
            "Truncated data block", EXPFILL }
        }
    };

    module_t *asterix_module;
    expert_module_t *expert_asterix;

    /* Register protocol */
    proto_asterix = proto_register_protocol(
        "ASTERIX - All Purpose STructured EUROCONTROL SuRveillance Information EXchange",
        "ASTERIX",
        "asterix"
    );

    /* Register fields and subtrees */
    proto_register_field_array(proto_asterix, hf, array_length(hf));
    proto_register_subtree_array(ett, array_length(ett));

    /* Register expert info */
    expert_asterix = expert_register_protocol(proto_asterix);
    expert_register_field_array(expert_asterix, ei, array_length(ei));

    /* Register preferences */
    asterix_module = prefs_register_protocol(proto_asterix, NULL);

    prefs_register_bool_preference(asterix_module, "heuristic_enabled",
        "Enable heuristic dissection",
        "Attempt to identify ASTERIX packets without relying on port numbers",
        &pref_heuristic_enabled);

    prefs_register_bool_preference(asterix_module, "verbose_info",
        "Verbose info column",
        "Display detailed information in the Info column",
        &pref_verbose_info);

    /* Register dissector */
    asterix_handle = register_dissector("asterix", dissect_asterix, proto_asterix);
}

/**
 * Register Handoff
 */
void
proto_reg_handoff_asterix(void)
{
    /* Register heuristic dissector for UDP */
    if (pref_heuristic_enabled) {
        heur_dissector_add("udp", dissect_asterix_heuristic,
                          "ASTERIX over UDP", "asterix_udp",
                          proto_asterix, HEURISTIC_ENABLE);
    }

    /* Register on common ASTERIX UDP ports */
    dissector_add_uint("udp.port", 8600, asterix_handle);
    dissector_add_uint("udp.port", 21112, asterix_handle);
}

/*
 * Editor modelines  -  https://www.wireshark.org/tools/modelines.html
 *
 * Local variables:
 * c-basic-offset: 4
 * tab-width: 8
 * indent-tabs-mode: nil
 * End:
 *
 * vi: set shiftwidth=4 tabstop=8 expandtab:
 * :indentSize=4:tabSize=8:noTabs=true:
 */
