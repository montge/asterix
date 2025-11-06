/**
 * ASTERIX Decoder for Node.js
 *
 * TypeScript type definitions for the ASTERIX decoder Node.js bindings.
 * Provides a type-safe interface for parsing ASTERIX protocol data.
 *
 * @packageDocumentation
 */

/**
 * A single parsed ASTERIX record
 */
export interface AsterixRecord {
    /** ASTERIX category (1-255) */
    category: number;

    /** Data block length in bytes */
    length: number;

    /** Timestamp in milliseconds since Unix epoch */
    timestamp_ms: number;

    /** CRC checksum */
    crc: number;

    /** Hex representation of raw data */
    hex_data?: string;

    /** Parsed data items (structure depends on category) */
    items: Record<string, any>;
}

/**
 * Result of incremental parsing with parseWithOffset
 */
export interface ParseResult {
    /** Array of parsed ASTERIX records */
    records: AsterixRecord[];

    /** Number of bytes consumed from input */
    bytesConsumed: number;

    /** Estimated number of blocks remaining to parse */
    remainingBlocks: number;
}

/**
 * Options for parsing ASTERIX data
 */
export interface ParseOptions {
    /** Enable verbose output (default: false) */
    verbose?: boolean;

    /** Filter results to specific category (optional) */
    filterCategory?: number;

    /** Maximum number of records to parse (optional) */
    maxRecords?: number;
}

/**
 * Initialize the ASTERIX parser with default or custom configuration
 *
 * This function must be called before any parsing operations.
 * It loads ASTERIX category definitions from XML files.
 *
 * @param configDir - Path to configuration directory (optional, uses default if not provided)
 * @throws {TypeError} If config directory path is invalid
 * @throws {Error} If initialization fails
 *
 * @example
 * ```typescript
 * import * as asterix from 'asterix-decoder';
 *
 * // Initialize with default config
 * asterix.init();
 *
 * // Or with custom config directory
 * asterix.init('/path/to/asterix/config');
 * ```
 */
export function init(configDir?: string): void;

/**
 * Load a specific ASTERIX category definition file
 *
 * @param xmlPath - Path to XML category definition file
 * @throws {TypeError} If XML path is invalid
 * @throws {Error} If loading fails
 *
 * @example
 * ```typescript
 * asterix.loadCategory('./config/asterix_cat062_1_19.xml');
 * ```
 */
export function loadCategory(xmlPath: string): void;

/**
 * Parse ASTERIX data from a Buffer
 *
 * This is the main parsing function. It accepts a Buffer containing
 * raw ASTERIX data and returns an array of parsed records.
 *
 * @param data - Buffer containing ASTERIX data (raw, PCAP, HDLC, etc.)
 * @param options - Optional parsing configuration
 * @returns Array of parsed ASTERIX records
 * @throws {TypeError} If input is not a Buffer or is empty
 * @throws {Error} If parsing fails
 *
 * @example
 * ```typescript
 * import * as fs from 'fs';
 * import * as asterix from 'asterix-decoder';
 *
 * asterix.init();
 *
 * const data = fs.readFileSync('sample.asterix');
 * const records = asterix.parse(data, { verbose: true });
 *
 * for (const record of records) {
 *   console.log(`Category ${record.category}: ${Object.keys(record.items).length} items`);
 * }
 * ```
 */
export function parse(data: Buffer, options?: ParseOptions): AsterixRecord[];

/**
 * Parse ASTERIX data incrementally with offset and block count
 *
 * Useful for processing large files or streams without loading
 * everything into memory at once.
 *
 * @param data - Buffer containing ASTERIX data
 * @param offset - Byte offset to start parsing from
 * @param blocksCount - Maximum number of blocks to parse (0 = all)
 * @param options - Optional parsing configuration
 * @returns ParseResult with records, bytes consumed, and remaining blocks
 * @throws {TypeError} If arguments are invalid
 * @throws {Error} If parsing fails
 *
 * @example
 * ```typescript
 * import * as fs from 'fs';
 * import * as asterix from 'asterix-decoder';
 *
 * asterix.init();
 *
 * const data = fs.readFileSync('large_file.asterix');
 * let offset = 0;
 * const allRecords: asterix.AsterixRecord[] = [];
 *
 * while (offset < data.length) {
 *   const result = asterix.parseWithOffset(data, offset, 100);
 *   allRecords.push(...result.records);
 *   offset = result.bytesConsumed;
 *
 *   if (result.remainingBlocks === 0) {
 *     break;
 *   }
 * }
 * ```
 */
export function parseWithOffset(
    data: Buffer,
    offset: number,
    blocksCount: number,
    options?: ParseOptions
): ParseResult;

/**
 * Get human-readable description for ASTERIX elements
 *
 * @param category - ASTERIX category (1-255)
 * @param item - Item ID (optional, null for category description)
 * @param field - Field name (optional)
 * @param value - Value (optional)
 * @returns Human-readable description
 * @throws {TypeError} If category is invalid (0 or > 255)
 * @throws {Error} If description lookup fails
 *
 * @example
 * ```typescript
 * import * as asterix from 'asterix-decoder';
 *
 * asterix.init();
 *
 * // Get category description
 * const catDesc = asterix.describe(48);
 * console.log(catDesc); // "Monoradar Target Reports"
 *
 * // Get item description
 * const itemDesc = asterix.describe(48, '010');
 * console.log(itemDesc); // "Data Source Identifier"
 *
 * // Get field value description
 * const fieldDesc = asterix.describe(48, '010', 'SAC', '7');
 * console.log(fieldDesc); // "System Area Code: 7"
 * ```
 */
export function describe(
    category: number,
    item?: string | null,
    field?: string | null,
    value?: string | null
): string;

/**
 * Check if an ASTERIX category is defined
 *
 * @param category - ASTERIX category (1-255)
 * @returns true if category is loaded, false otherwise
 *
 * @example
 * ```typescript
 * import * as asterix from 'asterix-decoder';
 *
 * asterix.init();
 *
 * if (asterix.isCategoryDefined(62)) {
 *   console.log('CAT062 is available');
 * }
 * ```
 */
export function isCategoryDefined(category: number): boolean;

/**
 * Version of the asterix-decoder package
 */
export const version: string;
