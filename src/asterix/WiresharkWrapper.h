/**
 * @file WiresharkWrapper.h
 * @brief C API for legacy Wireshark plugin integration
 *
 * This header defines the FFI (Foreign Function Interface) boundary between
 * the C++ ASTERIX parser and legacy Wireshark dissector plugins. It provides
 * C-compatible structures and functions for dissecting ASTERIX data in Wireshark.
 *
 * ## Status: Legacy API
 *
 * **IMPORTANT:** This interface is for legacy Wireshark plugins that were
 * removed in Issue #22. A modern Wireshark 4.x dissector will use a different
 * API and architecture. This header remains for reference and potential
 * future Wireshark 4.x integration.
 *
 * ## Architecture
 *
 * The wrapper provides two main components:
 * 1. **fulliautomatix_definitions**: Field definitions for Wireshark dissector tree
 * 2. **fulliautomatix_data**: Parsed ASTERIX data in Wireshark-compatible format
 *
 * ## Usage Pattern (Legacy)
 *
 * ```c
 * // Initialize ASTERIX parser with configuration
 * fulliautomatix_start(logger_func, "/path/to/asterix.ini");
 *
 * // Get field definitions for Wireshark registration
 * fulliautomatix_definitions* defs = fulliautomatix_get_definitions();
 *
 * // Parse ASTERIX packet
 * fulliautomatix_data* data = fulliautomatix_parse(buffer, length);
 *
 * // Traverse tree to populate Wireshark dissector
 * for (fulliautomatix_data* node = data; node != NULL; node = node->next) {
 *     if (node->tree) {
 *         // Add subtree
 *     } else {
 *         // Add field item
 *     }
 * }
 *
 * // Cleanup
 * fulliautomatix_data_destroy(data);
 * fulliautomatix_destroy_definitions(defs);
 * ```
 *
 * ## Data Structures
 *
 * - **fulliautomatix_definitions**: Field registry (name, type, display format)
 * - **fulliautomatix_data**: Tree-structured parsed data (values, byte offsets)
 * - **fulliautomatix_value_string**: Enumerated value descriptions
 *
 * ## Field Types
 *
 * Matches Wireshark's ftypes.h:
 * - FA_FT_UINT8, FA_FT_UINT16, FA_FT_UINT32, FA_FT_UINT64: Unsigned integers
 * - FA_FT_STRING: Text strings
 * - FA_FT_BYTES: Byte arrays
 * - FA_FT_BOOLEAN: True/false values
 * - FA_FT_PROTOCOL: Protocol tree nodes
 *
 * ## Display Formats
 *
 * - FA_BASE_DEC: Decimal notation
 * - FA_BASE_HEX: Hexadecimal notation
 * - FA_BASE_DEC_HEX: Decimal with hex in parentheses
 *
 * ## Memory Management
 *
 * - Caller must free definitions with fulliautomatix_destroy_definitions()
 * - Caller must free data with fulliautomatix_data_destroy()
 * - All string pointers are owned by the wrapper and freed automatically
 *
 * ## Future Migration
 *
 * For Wireshark 4.x dissector development, this API will likely be replaced
 * with modern Wireshark 4.x plugin architecture. See Issue #22 for details.
 *
 * @note C-compatible API (extern "C" linkage)
 * @note Not thread-safe - uses global ASTERIX parser state
 * @note Deprecated - legacy plugin support only
 *
 * @see Issue #22 (Wireshark 4.x modernization)
 * @see BREAKING_CHANGES.md (legacy plugin removal)
 */

#ifndef WIRESHARKWRAPPER_H_
#define WIRESHARKWRAPPER_H_

typedef void(*ptExtVoidPrintf)(char const *, ...);

typedef enum {
    PID_CATEGORY = 0,
    PID_LENGTH,
    PID_FSPEC,
    PID_REP,
    PID_LEN,

    PID_LAST
} PID_NRS;

/* field types (epan/ftypes/from ftypes.h) */
enum {
    FA_FT_NONE,        /* used for text labels with no value */
    FA_FT_PROTOCOL,
    FA_FT_BOOLEAN,     /* TRUE and FALSE come from <glib.h> */
    FA_FT_UINT8,
    FA_FT_UINT16,
    FA_FT_UINT24,      /* really a UINT32, but displayed as 3 hex-digits if FD_HEX*/
    FA_FT_UINT32,
    FA_FT_UINT64,
    FA_FT_INT8,
    FA_FT_INT16,
    FA_FT_INT24,       /* same as for UINT24 */
    FA_FT_INT32,
    FA_FT_INT64,
    FA_FT_FLOAT,
    FA_FT_DOUBLE,
    FA_FT_ABSOLUTE_TIME,
    FA_FT_RELATIVE_TIME,
    FA_FT_STRING,
    FA_FT_STRINGZ,     /* for use with proto_tree_add_item() */
    FA_FT_EBCDIC,      /* for use with proto_tree_add_item() */
    FA_FT_UINT_STRING, /* for use with proto_tree_add_item() */
    /*FA_FT_UCS2_LE, */    /* Unicode, 2 byte, Little Endian     */
    FA_FT_ETHER,
    FA_FT_BYTES,
    FA_FT_UINT_BYTES,
    FA_FT_IPv4,
    FA_FT_IPv6,
    FA_FT_IPXNET,
    FA_FT_FRAMENUM,    /* a UINT32, but if selected lets you go to frame with that numbe */
    FA_FT_PCRE,                /* a compiled Perl-Compatible Regular Expression object */
    FA_FT_GUID,                /* GUID, UUID */
    FA_FT_OID,                 /* OBJECT IDENTIFIER */
    FA_FT_NUM_TYPES /* last item number plus one */
};

/* from (epan/proto.h) */
typedef enum {
    FA_BASE_NONE,    /**< none */
    FA_BASE_DEC,    /**< decimal */
    FA_BASE_HEX,    /**< hexadecimal */
    FA_BASE_OCT,    /**< octal */
    FA_BASE_DEC_HEX,    /**< decimal (hexadecimal) */
    FA_BASE_HEX_DEC,    /**< hexadecimal (decimal) */
    FA_BASE_CUSTOM    /**< call custom routine (in ->strings) to format */
} fa_base_display_e;

#ifdef __cplusplus
extern "C" {
#endif

typedef struct _fulliautomatix_value_string {
    unsigned long value;
    char *strptr;
} fulliautomatix_value_string;

struct _fulliautomatix_definitions {
    int pid;       /** pid */
    char *name;      /**< full name of this field */
    char *abbrev;    /**< abbreviated name of this field */
    int type;       /**< field type, one of FA_FT_ (from ftypes.h) */
    int display;    /**< one of BASE_, or number of field bits for FA_FT_BOOLEAN */
    fulliautomatix_value_string *strings;   /**< value_string, range_string or true_false_string,
                                               typically converted by VALS(), RVALS() or TFS().
                                               If this is an FA_FT_PROTOCOL then it points to the
                                               associated protocol_t structure */
    unsigned long bitmask;    /**< bitmask of interesting bits */
    char *blurb;     /**< Brief description of field */
    struct _fulliautomatix_definitions *next;      /**< Pointer to next definition in linked list */
};

typedef struct _fulliautomatix_definitions fulliautomatix_definitions;

struct _fulliautomatix_data {
    int tree;                              /**< if 1 this is tree, otherwise it is item */
    int pid;                               /** pid */
    int bytenr;                            /**< byte number */
    int length;                            /**< length */
    char *description;                     /** description */
    int type;                              /**< data type */
    union {
        char *str;                           /** string value */
        signed long sl;                      /** signed long value */
        unsigned long ul;                    /** unsigned long value */
    } val;
    char *value_description;               /** additional string to append after the value */
    int err;                               /** packet correctness : 0-OK, 1-Warning, 2-Error */
    struct _fulliautomatix_data *next;     /** pointer to next data in linked list */
};

typedef struct _fulliautomatix_data fulliautomatix_data;

int fulliautomatix_start(ptExtVoidPrintf pPrintFunc, const char *ini_file_path);
void fulliautomatix_set_tracer(ptExtVoidPrintf pPrintFunc);
fulliautomatix_definitions *fulliautomatix_get_definitions(void);
void fulliautomatix_destroy_definitions(fulliautomatix_definitions *pDef);
fulliautomatix_data *fulliautomatix_parse(const unsigned char *pBuf, unsigned int len);
void fulliautomatix_data_destroy(fulliautomatix_data *pData);

fulliautomatix_data *newDataTree(fulliautomatix_data *prev, int bytenr, int length, char *description);
fulliautomatix_data *newDataTreeEnd(fulliautomatix_data *prev, int offset);
fulliautomatix_data *newDataUL(fulliautomatix_data *prev, int pid, int bytenr, int length, unsigned long val);
fulliautomatix_data *newDataSL(fulliautomatix_data *prev, int pid, int bytenr, int length, signed long val);
fulliautomatix_data *newDataString(fulliautomatix_data *prev, int pid, int bytenr, int length, char *val);
fulliautomatix_data *newDataBytes(fulliautomatix_data *prev, int pid, int bytenr, int length, unsigned char *val);
fulliautomatix_data *newDataMessage(fulliautomatix_data *prev, int bytenr, int length, int err, char *val);

#ifdef __cplusplus
}
#endif

#endif /* WIRESHARKWRAPPER_H_ */
