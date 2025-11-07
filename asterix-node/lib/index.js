/**
 * ASTERIX Decoder for Node.js
 *
 * This module provides a Node.js interface to the ASTERIX decoder.
 * It wraps the native N-API addon with idiomatic JavaScript APIs.
 *
 * @module asterix-decoder
 */

'use strict';

const path = require('path');
const native = require('../build/Release/asterix.node');

// Auto-initialize on module load with default config
let initialized = false;

/**
 * Initialize the ASTERIX parser
 *
 * @param {string} [configDir] - Path to configuration directory (optional)
 * @throws {TypeError} If config directory is invalid
 * @throws {Error} If initialization fails
 */
function init(configDir) {
    if (arguments.length === 0) {
        // Default: use config from install directory
        const defaultConfig = path.join(__dirname, '..', '..', 'install', 'share', 'asterix', 'config');
        native.init(defaultConfig);
    } else {
        // Type validation: must be string
        if (typeof configDir !== 'string') {
            throw new TypeError('Config directory must be a string');
        }
        native.init(configDir);
    }
    initialized = true;
}

/**
 * Load a specific ASTERIX category definition file
 *
 * @param {string} xmlPath - Path to XML category definition file
 * @throws {TypeError} If XML path is invalid
 * @throws {Error} If loading fails
 */
function loadCategory(xmlPath) {
    ensureInitialized();
    native.loadCategory(xmlPath);
}

/**
 * Parse ASTERIX data from a Buffer
 *
 * @param {Buffer} data - Buffer containing ASTERIX data
 * @param {Object} [options] - Optional parsing configuration
 * @param {boolean} [options.verbose=false] - Enable verbose output
 * @param {number} [options.filterCategory] - Filter to specific category
 * @param {number} [options.maxRecords] - Maximum records to parse
 * @returns {Array<Object>} Array of parsed ASTERIX records
 * @throws {TypeError} If input is not a Buffer or is empty
 * @throws {Error} If parsing fails
 */
function parse(data, options = {}) {
    ensureInitialized();

    if (!Buffer.isBuffer(data)) {
        throw new TypeError('First argument must be a Buffer');
    }

    if (data.length === 0) {
        throw new TypeError('Empty input data');
    }

    const records = native.parse(data, {
        verbose: options.verbose || false
    });

    // Apply JavaScript-side filters
    let filtered = records;

    if (options.filterCategory !== undefined) {
        filtered = filtered.filter(r => r.category === options.filterCategory);
    }

    if (options.maxRecords !== undefined) {
        filtered = filtered.slice(0, options.maxRecords);
    }

    return filtered;
}

/**
 * Parse ASTERIX data incrementally with offset and block count
 *
 * @param {Buffer} data - Buffer containing ASTERIX data
 * @param {number} offset - Byte offset to start parsing from
 * @param {number} blocksCount - Maximum number of blocks to parse (0 = all)
 * @param {Object} [options] - Optional parsing configuration
 * @param {boolean} [options.verbose=false] - Enable verbose output
 * @returns {Object} ParseResult with records, bytesConsumed, remainingBlocks
 * @throws {TypeError} If arguments are invalid
 * @throws {Error} If parsing fails
 */
function parseWithOffset(data, offset, blocksCount, options = {}) {
    ensureInitialized();

    if (!Buffer.isBuffer(data)) {
        throw new TypeError('First argument must be a Buffer');
    }

    if (typeof offset !== 'number' || offset < 0) {
        throw new TypeError('Offset must be a non-negative number');
    }

    if (typeof blocksCount !== 'number' || blocksCount < 0) {
        throw new TypeError('blocksCount must be a non-negative number');
    }

    return native.parseWithOffset(data, offset, blocksCount, {
        verbose: options.verbose || false
    });
}

/**
 * Get human-readable description for ASTERIX elements
 *
 * @param {number} category - ASTERIX category (1-255)
 * @param {string} [item] - Item ID (optional)
 * @param {string} [field] - Field name (optional)
 * @param {string} [value] - Value (optional)
 * @returns {string} Human-readable description
 * @throws {TypeError} If category is invalid
 * @throws {Error} If description lookup fails
 */
function describe(category, item, field, value) {
    ensureInitialized();

    if (typeof category !== 'number' || category < 1 || category > 255) {
        throw new TypeError('Category must be a number between 1 and 255');
    }

    return native.describe(category, item, field, value);
}

/**
 * Check if an ASTERIX category is defined
 *
 * @param {number} category - ASTERIX category (1-255)
 * @returns {boolean} true if category is loaded, false otherwise
 */
function isCategoryDefined(category) {
    if (!initialized) {
        return false; // Not initialized yet
    }

    if (typeof category !== 'number' || category < 1 || category > 255) {
        return false;
    }

    return native.isCategoryDefined(category);
}

/**
 * Ensure parser is initialized before use
 *
 * @private
 * @throws {Error} If parser is not initialized
 */
function ensureInitialized() {
    if (!initialized) {
        // Try auto-initialization
        try {
            init();
        } catch (e) {
            throw new Error(
                'ASTERIX parser not initialized. Call asterix.init() first. ' +
                'Auto-initialization failed: ' + e.message
            );
        }
    }
}

// Try auto-initialization on module load (fail silently)
try {
    init();
} catch (e) {
    // Will be initialized on first use
}

module.exports = {
    init,
    loadCategory,
    parse,
    parseWithOffset,
    describe,
    isCategoryDefined,
    version: native.version
};
