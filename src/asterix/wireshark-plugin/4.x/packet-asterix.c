/* packet-asterix.c
 * Wireshark 4.x dissector for ASTERIX protocol
 *
 * Copyright (c) 2013-2025 Croatia Control Ltd. (www.crocontrol.hr)
 * Copyright (c) 2025 ASTERIX Contributors
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 *
 * This file is part of Asterix.
 *
 * Asterix is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * Asterix is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with Asterix.  If not, see <http://www.gnu.org/licenses/>.
 *
 * ASTERIX (All Purpose STructured EUROCONTROL SuRveillance Information EXchange)
 * is a binary data format used in Air Traffic Management for surveillance data.
 *
 * This dissector uses the ASTERIX parser library via the WiresharkWrapper API.
 */

#include "config.h"

#include <epan/packet.h>
#include <epan/prefs.h>
#include <epan/expert.h>
#include <epan/proto_data.h>

/* Include the ASTERIX wrapper API */
#include "WiresharkWrapper.h"

/* Plugin version - should match the library version */
#define ASTERIX_PLUGIN_VERSION "2.9.0"

/* Default UDP port for ASTERIX traffic */
#define ASTERIX_UDP_PORT 8600

/* Protocol handles */
static int proto_asterix = -1;

/* Header field handles for standard ASTERIX fields */
static int hf_asterix_category = -1;
static int hf_asterix_length = -1;
static int hf_asterix_fspec = -1;
static int hf_asterix_data = -1;

/* Expert info handles */
static expert_field ei_asterix_parse_error = EI_INIT;
static expert_field ei_asterix_unknown_category = EI_INIT;

/* Subtree handles */
static gint ett_asterix = -1;
static gint ett_asterix_record = -1;
static gint ett_asterix_item = -1;

/* Preferences */
static guint asterix_udp_port = ASTERIX_UDP_PORT;
static const char *asterix_config_dir = NULL;

/* Forward declarations */
static int dissect_asterix(tvbuff_t *tvb, packet_info *pinfo, proto_tree *tree, void *data);
static void asterix_init(void);
static void asterix_cleanup(void);

/* Wireshark logging callback - forwards to Wireshark's logging system */
static void ws_log_callback(const char *format, ...) {
    va_list args;
    va_start(args, format);
    /* Use Wireshark's debug logging - this is a no-op in release builds */
    /* For production, you might want to use ws_debug or similar */
    va_end(args);
}

/**
 * Protocol registration - called once when Wireshark loads the plugin
 */
void proto_register_asterix(void) {
    /* Header field definitions */
    static hf_register_info hf[] = {
        { &hf_asterix_category,
            { "Category", "asterix.category",
              FT_UINT8, BASE_DEC, NULL, 0x0,
              "ASTERIX Category", HFILL }
        },
        { &hf_asterix_length,
            { "Length", "asterix.length",
              FT_UINT16, BASE_DEC, NULL, 0x0,
              "Data Block Length", HFILL }
        },
        { &hf_asterix_fspec,
            { "FSPEC", "asterix.fspec",
              FT_BYTES, BASE_NONE, NULL, 0x0,
              "Field Specification", HFILL }
        },
        { &hf_asterix_data,
            { "Data", "asterix.data",
              FT_BYTES, BASE_NONE, NULL, 0x0,
              "ASTERIX Data", HFILL }
        }
    };

    /* Subtree array */
    static gint *ett[] = {
        &ett_asterix,
        &ett_asterix_record,
        &ett_asterix_item
    };

    /* Expert info */
    static ei_register_info ei[] = {
        { &ei_asterix_parse_error,
            { "asterix.parse_error", PI_MALFORMED, PI_ERROR,
              "ASTERIX parse error", EXPFILL }
        },
        { &ei_asterix_unknown_category,
            { "asterix.unknown_category", PI_UNDECODED, PI_WARN,
              "Unknown ASTERIX category", EXPFILL }
        }
    };

    expert_module_t *expert_asterix;
    module_t *asterix_module;

    /* Register the protocol */
    proto_asterix = proto_register_protocol(
        "ASTERIX Protocol",     /* Full name */
        "ASTERIX",              /* Short name */
        "asterix"               /* Filter name */
    );

    /* Register header fields */
    proto_register_field_array(proto_asterix, hf, array_length(hf));

    /* Register subtrees */
    proto_register_subtree_array(ett, array_length(ett));

    /* Register expert info */
    expert_asterix = expert_register_protocol(proto_asterix);
    expert_register_field_array(expert_asterix, ei, array_length(ei));

    /* Register preferences */
    asterix_module = prefs_register_protocol(proto_asterix, NULL);

    prefs_register_uint_preference(asterix_module, "udp.port",
        "ASTERIX UDP Port",
        "UDP port for ASTERIX traffic",
        10, &asterix_udp_port);

    prefs_register_directory_preference(asterix_module, "config_dir",
        "Configuration Directory",
        "Directory containing ASTERIX category XML definitions",
        &asterix_config_dir);

    /* Register init and cleanup routines */
    register_init_routine(asterix_init);
    register_cleanup_routine(asterix_cleanup);
}

/**
 * Handoff registration - called after all protocols are registered
 */
void proto_reg_handoff_asterix(void) {
    static dissector_handle_t asterix_handle;
    static gboolean initialized = FALSE;
    static guint current_port = 0;

    if (!initialized) {
        asterix_handle = create_dissector_handle(dissect_asterix, proto_asterix);
        initialized = TRUE;
    } else {
        /* Remove old port binding if changing */
        if (current_port != 0) {
            dissector_delete_uint("udp.port", current_port, asterix_handle);
        }
    }

    /* Register for UDP port */
    current_port = asterix_udp_port;
    dissector_add_uint("udp.port", asterix_udp_port, asterix_handle);
}

/**
 * Initialize ASTERIX parser
 */
static void asterix_init(void) {
    const char *config_path = asterix_config_dir;

    /* Use default config path if not specified */
    if (config_path == NULL || config_path[0] == '\0') {
        /* Try common installation paths */
        config_path = "/usr/share/asterix/config";
    }

    /* Initialize the ASTERIX parser */
    if (fulliautomatix_start(ws_log_callback, config_path) != 0) {
        /* Initialization failed - parser will return NULL for all operations */
        /* This is handled gracefully in dissect_asterix */
    }
}

/**
 * Cleanup ASTERIX parser
 */
static void asterix_cleanup(void) {
    /* No explicit cleanup needed - library handles its own memory */
}

/**
 * Add parsed data to protocol tree
 */
static void add_asterix_data_to_tree(proto_tree *tree, tvbuff_t *tvb,
                                      fulliautomatix_data *data, packet_info *pinfo) {
    proto_item *item;
    proto_tree *subtree;

    while (data != NULL) {
        if (data->tree) {
            /* Tree node - create subtree */
            item = proto_tree_add_item(tree, proto_asterix, tvb,
                                       data->bytenr, data->length, ENC_NA);
            if (data->description) {
                proto_item_set_text(item, "%s", data->description);
            }
            subtree = proto_item_add_subtree(item, ett_asterix_item);

            /* Process children would go here if we had hierarchical structure */
        } else {
            /* Leaf node - add field based on type */
            switch (data->type) {
                case FA_FT_UINT8:
                case FA_FT_UINT16:
                case FA_FT_UINT24:
                case FA_FT_UINT32:
                    item = proto_tree_add_uint_format(tree, hf_asterix_data, tvb,
                        data->bytenr, data->length, (guint32)data->val.ul,
                        "%s: %lu", data->description ? data->description : "Value",
                        data->val.ul);
                    break;

                case FA_FT_INT8:
                case FA_FT_INT16:
                case FA_FT_INT24:
                case FA_FT_INT32:
                    item = proto_tree_add_int_format(tree, hf_asterix_data, tvb,
                        data->bytenr, data->length, (gint32)data->val.sl,
                        "%s: %ld", data->description ? data->description : "Value",
                        data->val.sl);
                    break;

                case FA_FT_STRING:
                case FA_FT_STRINGZ:
                    if (data->val.str) {
                        item = proto_tree_add_string_format(tree, hf_asterix_data, tvb,
                            data->bytenr, data->length, data->val.str,
                            "%s: %s", data->description ? data->description : "Value",
                            data->val.str);
                    }
                    break;

                case FA_FT_BYTES:
                    item = proto_tree_add_bytes_format(tree, hf_asterix_fspec, tvb,
                        data->bytenr, data->length,
                        tvb_get_ptr(tvb, data->bytenr, data->length),
                        "%s", data->description ? data->description : "Bytes");
                    break;

                default:
                    /* Generic text for unknown types */
                    if (data->description) {
                        item = proto_tree_add_item(tree, proto_asterix, tvb,
                            data->bytenr, data->length, ENC_NA);
                        proto_item_set_text(item, "%s", data->description);
                    }
                    break;
            }

            /* Add value description if present */
            if (data->value_description && item) {
                proto_item_append_text(item, " (%s)", data->value_description);
            }

            /* Mark errors/warnings */
            if (data->err == 2) {
                expert_add_info(pinfo, item, &ei_asterix_parse_error);
            } else if (data->err == 1) {
                expert_add_info(pinfo, item, &ei_asterix_unknown_category);
            }
        }

        data = data->next;
    }
}

/**
 * Main dissector function
 */
static int dissect_asterix(tvbuff_t *tvb, packet_info *pinfo, proto_tree *tree, void *data _U_) {
    proto_item *ti;
    proto_tree *asterix_tree;
    guint offset = 0;
    guint length = tvb_reported_length(tvb);
    guint8 category;
    guint16 block_length;
    fulliautomatix_data *parsed_data;
    const guint8 *packet_data;

    /* Minimum ASTERIX block is 3 bytes: CAT (1) + LEN (2) */
    if (length < 3) {
        return 0;
    }

    /* Set protocol column */
    col_set_str(pinfo->cinfo, COL_PROTOCOL, "ASTERIX");
    col_clear(pinfo->cinfo, COL_INFO);

    /* Get first category for info column */
    category = tvb_get_guint8(tvb, 0);
    col_add_fstr(pinfo->cinfo, COL_INFO, "Category %d", category);

    /* Create protocol tree */
    ti = proto_tree_add_item(tree, proto_asterix, tvb, 0, -1, ENC_NA);
    asterix_tree = proto_item_add_subtree(ti, ett_asterix);

    /* Process all data blocks in the packet */
    while (offset < length) {
        /* Check minimum length for header */
        if (offset + 3 > length) {
            proto_tree_add_expert(asterix_tree, pinfo, &ei_asterix_parse_error,
                                  tvb, offset, length - offset);
            break;
        }

        /* Read block header */
        category = tvb_get_guint8(tvb, offset);
        block_length = tvb_get_ntohs(tvb, offset + 1);

        /* Validate block length */
        if (block_length < 3 || offset + block_length > length) {
            proto_tree_add_expert_format(asterix_tree, pinfo, &ei_asterix_parse_error,
                tvb, offset, length - offset,
                "Invalid block length: %u (remaining: %u)",
                block_length, length - offset);
            break;
        }

        /* Add category field */
        proto_tree_add_uint(asterix_tree, hf_asterix_category, tvb, offset, 1, category);

        /* Add length field */
        proto_tree_add_uint(asterix_tree, hf_asterix_length, tvb, offset + 1, 2, block_length);

        /* Parse this block using the ASTERIX library */
        packet_data = tvb_get_ptr(tvb, offset, block_length);
        parsed_data = fulliautomatix_parse(packet_data, block_length);

        if (parsed_data != NULL) {
            /* Add parsed data to tree */
            add_asterix_data_to_tree(asterix_tree, tvb, parsed_data, pinfo);

            /* Free parsed data */
            fulliautomatix_data_destroy(parsed_data);
        } else {
            /* Parse failed - add raw data */
            proto_tree_add_item(asterix_tree, hf_asterix_data, tvb,
                               offset + 3, block_length - 3, ENC_NA);
            expert_add_info(pinfo, ti, &ei_asterix_parse_error);
        }

        offset += block_length;
    }

    return length;
}

/*
 * Plugin version information (required for Wireshark 4.x plugins)
 *
 * Note: For Wireshark 4.x, plugins export version symbols and a plugin_register
 * function that registers the protocol. The WS_DLL_PUBLIC_DEF macro is used
 * to export symbols from the shared library.
 */
#ifdef HAVE_PLUGINS

/* Define required macros if not provided by Wireshark headers */
#ifndef WS_DLL_PUBLIC_DEF
#if defined(_WIN32)
#define WS_DLL_PUBLIC_DEF __declspec(dllexport)
#else
#define WS_DLL_PUBLIC_DEF __attribute__((visibility("default")))
#endif
#endif

/* Plugin version information exported symbols */
WS_DLL_PUBLIC_DEF const gchar plugin_version[] = ASTERIX_PLUGIN_VERSION;
WS_DLL_PUBLIC_DEF const int plugin_want_major = WIRESHARK_VERSION_MAJOR;
WS_DLL_PUBLIC_DEF const int plugin_want_minor = WIRESHARK_VERSION_MINOR;

WS_DLL_PUBLIC_DEF void plugin_register(void);

/**
 * Plugin registration function
 * Called by Wireshark when loading the plugin
 */
void plugin_register(void) {
    static proto_plugin plug;

    plug.register_protoinfo = proto_register_asterix;
    plug.register_handoff = proto_reg_handoff_asterix;
    proto_register_plugin(&plug);
}
#endif
