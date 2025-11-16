/* packet-asterix.h
 *
 * Wireshark ASTERIX Dissector Plugin
 * ASTERIX Protocol Dissector Header
 *
 * ASTERIX (All Purpose STructured EUROCONTROL SuRveillance Information EXchange)
 * is a protocol used for Air Traffic Management (ATM) surveillance data exchange.
 *
 * Copyright (C) 2025
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

#ifndef PACKET_ASTERIX_H
#define PACKET_ASTERIX_H

#include <glib.h>
#include <epan/packet.h>
#include <epan/prefs.h>
#include <epan/expert.h>

/* Protocol handle */
extern int proto_asterix;

/* Subtree handles */
extern gint ett_asterix;
extern gint ett_asterix_datablock;
extern gint ett_asterix_record;
extern gint ett_asterix_fspec;
extern gint ett_asterix_item;

/* Header field handles */
extern int hf_asterix_category;
extern int hf_asterix_length;
extern int hf_asterix_fspec;
extern int hf_asterix_record;

/* ASTERIX category definitions */
typedef struct {
    guint8 category;
    const char* description;
} asterix_category_t;

/* Function prototypes */
void proto_register_asterix(void);
void proto_reg_handoff_asterix(void);

/* Dissector functions */
static int dissect_asterix(tvbuff_t *tvb, packet_info *pinfo, proto_tree *tree, void *data);
static gboolean dissect_asterix_heuristic(tvbuff_t *tvb, packet_info *pinfo, proto_tree *tree, void *data);

/* Helper functions */
static guint parse_fspec(tvbuff_t *tvb, guint offset, proto_tree *tree);
static guint dissect_asterix_datablock(tvbuff_t *tvb, packet_info *pinfo, proto_tree *tree, guint offset);

#endif /* PACKET_ASTERIX_H */
