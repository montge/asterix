/* config.h - Wireshark plugin configuration for out-of-tree build
 *
 * This minimal config.h is used when building the ASTERIX plugin
 * outside the Wireshark source tree.
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

#ifndef ASTERIX_WIRESHARK_CONFIG_H
#define ASTERIX_WIRESHARK_CONFIG_H

/* Plugin is built as external module */
#define HAVE_PLUGINS 1

/* Enable ASTERIX wrapper support */
#define WIRESHARK_WRAPPER 1

#endif /* ASTERIX_WIRESHARK_CONFIG_H */
