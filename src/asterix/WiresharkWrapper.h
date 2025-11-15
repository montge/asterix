/*
 *  Copyright (c) 2013 Croatia Control Ltd. (www.crocontrol.hr)
 *
 *  This file is part of Asterix.
 *
 *  Asterix is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, either version 3 of the License, or
 *  (at your option) any later version.
 *
 *  Asterix is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with Asterix.  If not, see <http://www.gnu.org/licenses/>.
 *
 *
 * AUTHORS: Damir Salantic, Croatia Control Ltd.
 *
 */

/**
 * @file WiresharkWrapper.h
 * @brief Wireshark/Ethereal plugin interface definitions for ASTERIX protocol dissector
 *
 * This header defines the C-compatible FFI boundary between the ASTERIX parser
 * and Wireshark/Ethereal network protocol analyzers. It provides type-safe structures
 * and APIs for exposing ASTERIX parsing capabilities as a Wireshark dissector plugin.
 *
 * @par Plugin Architecture
 * The plugin architecture follows Wireshark's plugin model:
 * 1. Plugin initialization via fulliautomatix_start() loads ASTERIX category definitions
 * 2. Protocol field definitions are exposed via fulliautomatix_get_definitions()
 * 3. Packet dissection via fulliautomatix_parse() produces protocol tree data
 * 4. Wireshark consumes the tree data to render the protocol hierarchy in its UI
 *
 * @par Data Flow
 * @code
 * Wireshark Packet → fulliautomatix_parse() → ASTERIX Parser → fulliautomatix_data tree
 *                                                ↓
 *                                        Category definitions
 *                                                ↓
 *                                     Wireshark Protocol Tree UI
 * @endcode
 *
 * @par Compilation
 * This interface is only active when compiled with WIRESHARK_WRAPPER or ETHEREAL_WRAPPER
 * preprocessor flags:
 * @code
 * # Wireshark plugin build
 * gcc -DWIRESHARK_WRAPPER -shared packet-asterix.c -o asterix.so
 *
 * # Ethereal plugin build (legacy)
 * gcc -DETHEREAL_WRAPPER -DWIRESHARK_WRAPPER -shared packet-asterix.c -o asterix.so
 * @endcode
 *
 * @note This file defines pure C structures and functions for ABI compatibility with
 *       Wireshark's plugin API. The implementation (WiresharkWrapper.cpp) bridges to
 *       the C++ ASTERIX parser core.
 *
 * @see DataBlock::wiresharkData() for parsed data tree generation
 * @see DataItem::getWiresharkData() for individual data item rendering
 * @see Category::getWiresharkDefinitions() for protocol field definitions
 * @see AsterixData for the main parser interface
 */

#ifndef WIRESHARKWRAPPER_H_
#define WIRESHARKWRAPPER_H_

/**
 * @brief Function pointer type for Wireshark's printf-style logging/tracing
 *
 * This type defines a callback function pointer compatible with Wireshark's
 * internal logging mechanisms (e.g., g_warning, proto_tree_add_text). The ASTERIX
 * parser uses this to output diagnostic messages to Wireshark's console.
 *
 * @param format printf-style format string
 * @param ... Variable arguments matching the format string
 *
 * @note The function signature must match Wireshark's expected logging functions
 */
typedef void(*ptExtVoidPrintf)(char const *, ...);

/**
 * @enum PID_NRS
 * @brief Protocol field IDs for standard ASTERIX protocol elements
 *
 * Defines unique identifiers for common ASTERIX protocol fields that appear
 * in every data block. These PIDs are used to register protocol fields with
 * Wireshark and to reference them when building the protocol tree.
 *
 * @note Category-specific data items have dynamically assigned PIDs generated
 *       from their category number and item ID (see Category::getWiresharkDefinitions())
 */
typedef enum {
    PID_CATEGORY = 0,  /**< ASTERIX category number (e.g., 48, 62) */
    PID_LENGTH,        /**< Data block payload length in bytes */
    PID_FSPEC,         /**< Field Specification (FSPEC) bitmap */
    PID_REP,           /**< Repetition factor for repetitive data items */
    PID_LEN,           /**< Data item length for variable-length items */

    PID_LAST           /**< Sentinel value for array bounds */
} PID_NRS;

/**
 * @enum FA_FT_Types
 * @brief Wireshark field type constants (from epan/ftypes/ftypes.h)
 *
 * Defines field type constants compatible with Wireshark's protocol field API.
 * These types determine how Wireshark displays and interprets protocol field values.
 * Each type corresponds to a specific C data type and display format.
 *
 * @note These constants mirror Wireshark's ftenum enumeration but use the FA_ prefix
 *       to avoid naming conflicts when building as a plugin.
 *
 * @see fulliautomatix_definitions::type for usage in protocol field definitions
 */
enum {
    FA_FT_NONE,        /**< No value (text labels only) */
    FA_FT_PROTOCOL,    /**< Protocol header */
    FA_FT_BOOLEAN,     /**< Boolean (TRUE/FALSE from glib.h) */
    FA_FT_UINT8,       /**< Unsigned 8-bit integer */
    FA_FT_UINT16,      /**< Unsigned 16-bit integer */
    FA_FT_UINT24,      /**< Unsigned 24-bit integer (stored as UINT32, displayed as 3 hex digits) */
    FA_FT_UINT32,      /**< Unsigned 32-bit integer */
    FA_FT_UINT64,      /**< Unsigned 64-bit integer */
    FA_FT_INT8,        /**< Signed 8-bit integer */
    FA_FT_INT16,       /**< Signed 16-bit integer */
    FA_FT_INT24,       /**< Signed 24-bit integer (stored as INT32) */
    FA_FT_INT32,       /**< Signed 32-bit integer */
    FA_FT_INT64,       /**< Signed 64-bit integer */
    FA_FT_FLOAT,       /**< IEEE 754 single-precision float */
    FA_FT_DOUBLE,      /**< IEEE 754 double-precision float */
    FA_FT_ABSOLUTE_TIME, /**< Absolute timestamp (epoch-based) */
    FA_FT_RELATIVE_TIME, /**< Relative time duration */
    FA_FT_STRING,      /**< Character string (not null-terminated) */
    FA_FT_STRINGZ,     /**< Null-terminated string (for proto_tree_add_item) */
    FA_FT_EBCDIC,      /**< EBCDIC-encoded string */
    FA_FT_UINT_STRING, /**< String prefixed with length (for proto_tree_add_item) */
    /*FA_FT_UCS2_LE, */    /**< Unicode UTF-16LE (2 bytes per char) */
    FA_FT_ETHER,       /**< Ethernet MAC address (6 bytes) */
    FA_FT_BYTES,       /**< Raw byte array */
    FA_FT_UINT_BYTES,  /**< Byte array with length prefix */
    FA_FT_IPv4,        /**< IPv4 address (4 bytes) */
    FA_FT_IPv6,        /**< IPv6 address (16 bytes) */
    FA_FT_IPXNET,      /**< IPX network number */
    FA_FT_FRAMENUM,    /**< Frame number reference (UINT32, clickable to navigate) */
    FA_FT_PCRE,        /**< Compiled Perl-Compatible Regular Expression */
    FA_FT_GUID,        /**< GUID/UUID (128-bit identifier) */
    FA_FT_OID,         /**< ASN.1 Object Identifier */
    FA_FT_NUM_TYPES    /**< Total number of field types (sentinel) */
};

/**
 * @enum fa_base_display_e
 * @brief Display format for numeric protocol fields (from epan/proto.h)
 *
 * Specifies how Wireshark should format and display numeric protocol field values
 * in the dissector pane. The display format is independent of the underlying data type.
 *
 * @note Used in fulliautomatix_definitions::display to control field rendering
 * @see fulliautomatix_definitions
 */
typedef enum {
    FA_BASE_NONE,    /**< No base (for non-numeric types like strings) */
    FA_BASE_DEC,     /**< Decimal format (e.g., 255) */
    FA_BASE_HEX,     /**< Hexadecimal format (e.g., 0xff) */
    FA_BASE_OCT,     /**< Octal format (e.g., 0377) */
    FA_BASE_DEC_HEX, /**< Decimal with hex in parentheses (e.g., 255 (0xff)) */
    FA_BASE_HEX_DEC, /**< Hexadecimal with decimal in parentheses (e.g., 0xff (255)) */
    FA_BASE_CUSTOM   /**< Custom formatter via callback in strings field */
} fa_base_display_e;

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @struct _fulliautomatix_value_string
 * @brief Value-to-string mapping for protocol field enumerations
 *
 * Provides human-readable text descriptions for numeric protocol field values.
 * Used by Wireshark to display enumerated values (e.g., "Target Report" for value 1)
 * instead of raw numbers.
 *
 * @par Usage Example
 * @code
 * // Define value strings for ASTERIX Category 48 Message Type (I048/000)
 * fulliautomatix_value_string msg_type_strings[] = {
 *     {1, "Target Report"},
 *     {2, "Start of Update Cycle"},
 *     {3, "Periodic Status Message"},
 *     {4, "Event-triggered Status Message"},
 *     {0, NULL}  // Null terminator
 * };
 * @endcode
 *
 * @note The array must be null-terminated (strptr == NULL for last entry)
 * @see fulliautomatix_definitions::strings for usage in field definitions
 */
typedef struct _fulliautomatix_value_string {
    unsigned long value;  /**< Numeric value from protocol field */
    char *strptr;         /**< Human-readable description (NULL for array terminator) */
} fulliautomatix_value_string;

/**
 * @struct _fulliautomatix_definitions
 * @brief Protocol field definition for Wireshark dissector registration
 *
 * Defines metadata for a single protocol field to be registered with Wireshark's
 * protocol tree system. Each definition describes how the field should be displayed,
 * parsed, and interpreted in the Wireshark UI.
 *
 * This structure closely mirrors Wireshark's hf_register_info but uses a linked list
 * for dynamic registration of ASTERIX category definitions loaded from XML.
 *
 * @par Usage in Plugin
 * @code
 * // Plugin initialization (proto_register_asterix)
 * fulliautomatix_definitions *defs = fulliautomatix_get_definitions();
 * while (defs) {
 *     hf_register_info hf = {
 *         &defs->pid,
 *         {defs->name, defs->abbrev, defs->type, defs->display,
 *          defs->strings, defs->bitmask, defs->blurb, HFILL}
 *     };
 *     proto_register_field_array(proto_asterix, &hf, 1);
 *     defs = defs->next;
 * }
 * fulliautomatix_destroy_definitions(defs);
 * @endcode
 *
 * @note Memory management: Call fulliautomatix_destroy_definitions() to free the list
 * @see fulliautomatix_get_definitions() for retrieving all field definitions
 * @see fulliautomatix_destroy_definitions() for cleanup
 * @see Category::getWiresharkDefinitions() for category-specific field generation
 */
struct _fulliautomatix_definitions {
    int pid;       /**< Protocol field ID (unique identifier for this field) */
    char *name;    /**< Full display name (e.g., "Target Address") */
    char *abbrev;  /**< Abbreviated filter name (e.g., "asterix.i048.040") */
    int type;      /**< Field type (one of FA_FT_* constants) */
    int display;   /**< Display format (one of FA_BASE_* for numeric types,
                        or bit count for FA_FT_BOOLEAN) */
    fulliautomatix_value_string *strings;
                   /**< Value-to-string mappings for enumerations, or NULL.
                        For FA_FT_PROTOCOL, points to associated protocol_t structure. */
    unsigned long bitmask;
                   /**< Bitmask for extracting field from larger value (0 if N/A) */
    char *blurb;   /**< Brief description shown in status bar */
    struct _fulliautomatix_definitions *next;
                   /**< Pointer to next field definition (NULL if last) */
};

/**
 * @typedef fulliautomatix_definitions
 * @brief Convenience typedef for struct _fulliautomatix_definitions
 */
typedef struct _fulliautomatix_definitions fulliautomatix_definitions;

/**
 * @struct _fulliautomatix_data
 * @brief Parsed protocol tree data for Wireshark dissector rendering
 *
 * Represents a single node in the hierarchical protocol tree produced by parsing
 * an ASTERIX data block. Each node can be either a tree node (container for child nodes)
 * or a leaf item (actual protocol field value).
 *
 * The linked list structure allows dynamic construction of the protocol tree during
 * parsing, which Wireshark then renders in its protocol dissector pane.
 *
 * @par Tree Structure
 * @code
 * // Example protocol tree for ASTERIX Category 48:
 * fulliautomatix_data (tree=1, description="Data Record")
 *   ├── fulliautomatix_data (tree=0, pid=PID_CATEGORY, val.ul=48)
 *   ├── fulliautomatix_data (tree=0, pid=PID_FSPEC, val.str="0xC0...")
 *   ├── fulliautomatix_data (tree=1, description="I048/010 Data Source Identifier")
 *   │   ├── fulliautomatix_data (tree=0, pid=..., val.ul=5)  // SAC
 *   │   └── fulliautomatix_data (tree=0, pid=..., val.ul=12) // SIC
 *   └── fulliautomatix_data (tree=1, description="I048/040 Measured Position")
 *       ├── fulliautomatix_data (tree=0, pid=..., val.sl=12345) // RHO
 *       └── fulliautomatix_data (tree=0, pid=..., val.sl=6789)  // THETA
 * @endcode
 *
 * @par Usage in Plugin
 * @code
 * // Wireshark dissector function (dissect_asterix)
 * fulliautomatix_data *data = fulliautomatix_parse(tvb_get_ptr(tvb, 0, -1), len);
 * while (data) {
 *     if (data->tree) {
 *         // Create subtree
 *         proto_item *ti = proto_tree_add_text(tree, tvb, data->bytenr, data->length,
 *                                               "%s", data->description);
 *         proto_tree *subtree = proto_item_add_subtree(ti, ett_asterix);
 *     } else {
 *         // Add leaf item
 *         proto_tree_add_uint(tree, data->pid, tvb, data->bytenr, data->length, data->val.ul);
 *     }
 *     data = data->next;
 * }
 * fulliautomatix_data_destroy(data);
 * @endcode
 *
 * @note Memory management: Call fulliautomatix_data_destroy() to free the list
 * @see fulliautomatix_parse() for generating the tree
 * @see fulliautomatix_data_destroy() for cleanup
 * @see DataBlock::wiresharkData() for tree generation implementation
 * @see newDataTree(), newDataUL(), newDataSL(), newDataString() for node constructors
 */
struct _fulliautomatix_data {
    int tree;       /**< Node type: 1 = tree/container node, 0 = leaf/item node */
    int pid;        /**< Protocol field ID (references fulliautomatix_definitions::pid) */
    int bytenr;     /**< Byte offset of this field in the packet */
    int length;     /**< Length of this field in bytes */
    char *description;
                    /**< Human-readable description for tree nodes */
    int type;       /**< Data type (one of FA_FT_* constants) */
    union {
        char *str;          /**< String value (for FA_FT_STRING, FA_FT_BYTES, etc.) */
        signed long sl;     /**< Signed integer value (for FA_FT_INT*) */
        unsigned long ul;   /**< Unsigned integer value (for FA_FT_UINT*) */
    } val;          /**< Field value (interpretation depends on type) */
    char *value_description;
                    /**< Additional text appended after value (e.g., units, meaning) */
    int err;        /**< Packet correctness indicator: 0=OK, 1=Warning, 2=Error */
    struct _fulliautomatix_data *next;
                    /**< Pointer to next node in tree (NULL if last) */
};

/**
 * @typedef fulliautomatix_data
 * @brief Convenience typedef for struct _fulliautomatix_data
 */
typedef struct _fulliautomatix_data fulliautomatix_data;

/**
 * @brief Initialize the ASTERIX parser with Wireshark logging and category definitions
 *
 * Initializes the ASTERIX parser subsystem, loads category definitions from XML files
 * listed in asterix.ini, and registers the logging callback for diagnostic output.
 *
 * This function must be called during Wireshark plugin initialization (proto_register_asterix)
 * before any parsing or definition retrieval operations.
 *
 * @param pPrintFunc Callback function for diagnostic logging (e.g., g_warning)
 * @param ini_file_path Directory path containing asterix.ini and category XML files
 *
 * @return 0 on success, non-zero on failure (e.g., missing asterix.ini, XML parse error)
 *
 * @par Example
 * @code
 * // Wireshark plugin initialization
 * void proto_register_asterix(void) {
 *     const char *config_dir = get_plugins_pers_dir();
 *     if (fulliautomatix_start(g_warning, config_dir) != 0) {
 *         g_error("Failed to initialize ASTERIX parser");
 *         return;
 *     }
 *     // Register protocol fields...
 * }
 * @endcode
 *
 * @note Thread safety: Not thread-safe. Must be called once during plugin init.
 * @see fulliautomatix_get_definitions() for retrieving field definitions after init
 * @see AsterixDefinition for internal category management
 */
int fulliautomatix_start(ptExtVoidPrintf pPrintFunc, const char *ini_file_path);

/**
 * @brief Set or update the diagnostic logging callback
 *
 * Updates the logging callback used for trace and error messages. Useful for
 * runtime log level changes or redirecting output.
 *
 * @param pPrintFunc New callback function for diagnostic logging
 *
 * @note The callback must remain valid for the lifetime of the parser
 * @see Tracer::Configure() for internal implementation
 */
void fulliautomatix_set_tracer(ptExtVoidPrintf pPrintFunc);

/**
 * @brief Retrieve protocol field definitions for all loaded ASTERIX categories
 *
 * Generates a linked list of protocol field definitions for registration with
 * Wireshark. Includes standard ASTERIX fields (category, length, FSPEC) plus
 * category-specific data items loaded from XML.
 *
 * @return Pointer to head of definitions linked list, or NULL on error
 *
 * @par Usage Pattern
 * @code
 * // Proto registration phase
 * fulliautomatix_definitions *defs = fulliautomatix_get_definitions();
 * fulliautomatix_definitions *current = defs;
 *
 * while (current) {
 *     // Register each field with Wireshark
 *     hf_register_info hf = {...};  // Convert to Wireshark format
 *     proto_register_field_array(proto_asterix, &hf, 1);
 *     current = current->next;
 * }
 *
 * // Clean up
 * fulliautomatix_destroy_definitions(defs);
 * @endcode
 *
 * @note Caller owns the returned list and must call fulliautomatix_destroy_definitions()
 * @warning Do not modify the list structure (read-only)
 * @see fulliautomatix_destroy_definitions() for cleanup
 * @see Category::getWiresharkDefinitions() for category-specific definitions
 */
fulliautomatix_definitions *fulliautomatix_get_definitions(void);

/**
 * @brief Free memory allocated for protocol field definitions
 *
 * Recursively frees all nodes in the definitions linked list, including
 * dynamically allocated strings (name, abbrev, blurb) and value_string arrays.
 *
 * @param pDef Pointer to head of definitions list (may be NULL)
 *
 * @note Safe to call with NULL pointer (no-op)
 * @note After calling, pDef is invalid (dangling pointer)
 * @see fulliautomatix_get_definitions() for allocation
 */
void fulliautomatix_destroy_definitions(fulliautomatix_definitions *pDef);

/**
 * @brief Parse ASTERIX binary data and generate protocol tree
 *
 * Parses raw ASTERIX data blocks and constructs a hierarchical protocol tree
 * suitable for rendering in Wireshark's dissector pane. Automatically detects
 * category from data block headers and applies corresponding XML definitions.
 *
 * @param pBuf Pointer to raw ASTERIX binary data
 * @param len Length of data buffer in bytes
 *
 * @return Pointer to head of protocol tree (linked list), or NULL on parse error
 *
 * @par Example
 * @code
 * // Wireshark dissector callback
 * static void dissect_asterix(tvbuff_t *tvb, packet_info *pinfo, proto_tree *tree) {
 *     guint len = tvb_reported_length(tvb);
 *     const guint8 *data = tvb_get_ptr(tvb, 0, len);
 *
 *     fulliautomatix_data *parsed = fulliautomatix_parse(data, len);
 *     if (!parsed) {
 *         proto_tree_add_text(tree, tvb, 0, len, "Parse error");
 *         return;
 *     }
 *
 *     // Render protocol tree in Wireshark UI
 *     while (parsed) {
 *         if (parsed->tree) {
 *             // Add subtree node
 *         } else {
 *             // Add leaf item
 *         }
 *         parsed = parsed->next;
 *     }
 *
 *     fulliautomatix_data_destroy(parsed);
 * }
 * @endcode
 *
 * @note Caller owns the returned tree and must call fulliautomatix_data_destroy()
 * @warning Input buffer must remain valid until tree is destroyed
 * @see fulliautomatix_data_destroy() for cleanup
 * @see DataBlock::wiresharkData() for internal tree generation
 * @see AsterixData for main parser entry point
 */
fulliautomatix_data *fulliautomatix_parse(const unsigned char *pBuf, unsigned int len);

/**
 * @brief Free memory allocated for parsed protocol tree
 *
 * Recursively frees all nodes in the protocol tree linked list, including
 * dynamically allocated strings (description, value_description, string values).
 *
 * @param pData Pointer to head of protocol tree (may be NULL)
 *
 * @note Safe to call with NULL pointer (no-op)
 * @note After calling, pData is invalid (dangling pointer)
 * @see fulliautomatix_parse() for allocation
 */
void fulliautomatix_data_destroy(fulliautomatix_data *pData);

/**
 * @brief Create a protocol tree container node
 *
 * Allocates and initializes a tree node (container) in the protocol tree.
 * Tree nodes represent hierarchical groupings (e.g., "Data Record", "I048/010")
 * and can contain child nodes.
 *
 * @param prev Pointer to previous node in list (will set prev->next), or NULL if first
 * @param bytenr Byte offset of this tree node in the packet
 * @param length Total length in bytes covered by this tree node
 * @param description Human-readable label for the tree node
 *
 * @return Pointer to newly allocated tree node, or NULL on allocation failure
 *
 * @note Internal function used by DataItem::getWiresharkData() implementations
 * @note Caller must eventually free via fulliautomatix_data_destroy()
 * @see newDataTreeEnd() for closing multi-level tree structures
 */
fulliautomatix_data *newDataTree(fulliautomatix_data *prev, int bytenr, int length, char *description);

/**
 * @brief Mark the end of a tree node at a specific offset
 *
 * Updates a tree node's length based on the ending byte offset. Used for
 * variable-length tree nodes where final length is not known until parsing completes.
 *
 * @param prev Pointer to tree node to update
 * @param offset Ending byte offset (exclusive)
 *
 * @return The prev pointer (unchanged)
 *
 * @note Internal function used by compound/repetitive item parsers
 * @see DataItemFormatCompound::getWiresharkData() for usage
 */
fulliautomatix_data *newDataTreeEnd(fulliautomatix_data *prev, int offset);

/**
 * @brief Create a leaf node with unsigned integer value
 *
 * Allocates and initializes a leaf item node containing an unsigned integer value.
 * Used for protocol fields like counters, identifiers, bitmasks.
 *
 * @param prev Pointer to previous node in list (will set prev->next), or NULL if first
 * @param pid Protocol field ID (from PID_NRS or category-specific IDs)
 * @param bytenr Byte offset of this field in the packet
 * @param length Length of this field in bytes
 * @param val Unsigned integer value
 *
 * @return Pointer to newly allocated leaf node, or NULL on allocation failure
 *
 * @note Sets type=FA_FT_UINT32, tree=0, val.ul=val
 * @note Internal function used by DataItemBits::getWiresharkValue()
 * @see newDataSL() for signed integers
 */
fulliautomatix_data *newDataUL(fulliautomatix_data *prev, int pid, int bytenr, int length, unsigned long val);

/**
 * @brief Create a leaf node with signed integer value
 *
 * Allocates and initializes a leaf item node containing a signed integer value.
 * Used for protocol fields representing positions, velocities, measurements.
 *
 * @param prev Pointer to previous node in list (will set prev->next), or NULL if first
 * @param pid Protocol field ID (from PID_NRS or category-specific IDs)
 * @param bytenr Byte offset of this field in the packet
 * @param length Length of this field in bytes
 * @param val Signed integer value
 *
 * @return Pointer to newly allocated leaf node, or NULL on allocation failure
 *
 * @note Sets type=FA_FT_INT32, tree=0, val.sl=val
 * @note Internal function used by DataItemBits::getWiresharkValue()
 * @see newDataUL() for unsigned integers
 */
fulliautomatix_data *newDataSL(fulliautomatix_data *prev, int pid, int bytenr, int length, signed long val);

/**
 * @brief Create a leaf node with null-terminated string value
 *
 * Allocates and initializes a leaf item node containing a string value.
 * The string is duplicated (caller retains ownership of original).
 *
 * @param prev Pointer to previous node in list (will set prev->next), or NULL if first
 * @param pid Protocol field ID (from PID_NRS or category-specific IDs)
 * @param bytenr Byte offset of this field in the packet
 * @param length Length of this field in bytes
 * @param val Null-terminated string value (will be copied)
 *
 * @return Pointer to newly allocated leaf node, or NULL on allocation failure
 *
 * @note Sets type=FA_FT_STRINGZ, tree=0, val.str=strdup(val)
 * @note Internal function used by DataItemFormatFixed::getWiresharkData()
 * @see newDataBytes() for binary data
 */
fulliautomatix_data *newDataString(fulliautomatix_data *prev, int pid, int bytenr, int length, char *val);

/**
 * @brief Create a leaf node with raw byte array value
 *
 * Allocates and initializes a leaf item node containing binary data (e.g., FSPEC bytes).
 * The byte array is duplicated (caller retains ownership of original).
 *
 * @param prev Pointer to previous node in list (will set prev->next), or NULL if first
 * @param pid Protocol field ID (from PID_NRS or category-specific IDs)
 * @param bytenr Byte offset of this field in the packet
 * @param length Length of this field in bytes
 * @param val Pointer to byte array (will be copied)
 *
 * @return Pointer to newly allocated leaf node, or NULL on allocation failure
 *
 * @note Sets type=FA_FT_BYTES, tree=0, val.str=(copy of byte array)
 * @note Internal function used by DataRecord::getWiresharkData() for FSPEC
 * @see newDataString() for text strings
 */
fulliautomatix_data *newDataBytes(fulliautomatix_data *prev, int pid, int bytenr, int length, unsigned char *val);

/**
 * @brief Create a diagnostic message node (error/warning)
 *
 * Allocates and initializes a special message node for displaying parse errors
 * or warnings in the protocol tree. Wireshark can highlight these with appropriate
 * severity colors.
 *
 * @param prev Pointer to previous node in list (will set prev->next), or NULL if first
 * @param bytenr Byte offset where error occurred
 * @param length Length of problematic data
 * @param err Severity level: 0=OK, 1=Warning, 2=Error
 * @param val Error/warning message text (will be copied)
 *
 * @return Pointer to newly allocated message node, or NULL on allocation failure
 *
 * @note Sets type=FA_FT_NONE, tree=0, err=err, val.str=strdup(val)
 * @note Internal function used by parsing error handlers
 * @see fulliautomatix_data::err for severity interpretation
 */
fulliautomatix_data *newDataMessage(fulliautomatix_data *prev, int bytenr, int length, int err, char *val);

#ifdef __cplusplus
}
#endif

#endif /* WIRESHARKWRAPPER_H_ */
