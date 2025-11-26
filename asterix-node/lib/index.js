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
const { Transform } = require('stream');
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
 * Asynchronously parse ASTERIX data from a Buffer
 *
 * Recommended for large files to avoid blocking the event loop.
 * Uses setImmediate to yield to the event loop between parsing chunks.
 *
 * @param {Buffer} data - Buffer containing ASTERIX data
 * @param {Object} [options] - Optional parsing configuration
 * @param {boolean} [options.verbose=false] - Enable verbose output
 * @param {number} [options.filterCategory] - Filter to specific category
 * @param {number} [options.maxRecords] - Maximum records to parse
 * @param {number} [options.chunkSize=100] - Records to parse per iteration
 * @returns {Promise<Array<Object>>} Promise resolving to array of parsed records
 * @throws {TypeError} If input is not a Buffer or is empty
 * @throws {Error} If parsing fails
 */
async function parseAsync(data, options = {}) {
    ensureInitialized();

    if (!Buffer.isBuffer(data)) {
        throw new TypeError('First argument must be a Buffer');
    }

    if (data.length === 0) {
        throw new TypeError('Empty input data');
    }

    const chunkSize = options.chunkSize || 100;
    const allRecords = [];
    let offset = 0;

    return new Promise((resolve, reject) => {
        const parseChunk = () => {
            try {
                if (offset >= data.length) {
                    // Apply filters
                    let filtered = allRecords;
                    if (options.filterCategory !== undefined) {
                        filtered = filtered.filter(r => r.category === options.filterCategory);
                    }
                    if (options.maxRecords !== undefined) {
                        filtered = filtered.slice(0, options.maxRecords);
                    }
                    resolve(filtered);
                    return;
                }

                const result = native.parseWithOffset(data, offset, chunkSize, {
                    verbose: options.verbose || false
                });

                allRecords.push(...result.records);

                if (result.bytesConsumed <= offset || result.remainingBlocks === 0) {
                    // No progress or done
                    let filtered = allRecords;
                    if (options.filterCategory !== undefined) {
                        filtered = filtered.filter(r => r.category === options.filterCategory);
                    }
                    if (options.maxRecords !== undefined) {
                        filtered = filtered.slice(0, options.maxRecords);
                    }
                    resolve(filtered);
                    return;
                }

                offset = result.bytesConsumed;

                // Yield to event loop
                setImmediate(parseChunk);
            } catch (err) {
                reject(err);
            }
        };

        setImmediate(parseChunk);
    });
}

/**
 * Create a Transform stream for parsing ASTERIX data
 *
 * Processes ASTERIX data as a Node.js stream, emitting parsed records.
 * Useful for processing large files or network streams.
 *
 * @param {Object} [options] - Stream options
 * @param {boolean} [options.verbose=false] - Enable verbose output
 * @param {number} [options.filterCategory] - Filter to specific category
 * @param {boolean} [options.objectMode=true] - Emit objects (default) or JSON strings
 * @returns {Transform} Transform stream that emits parsed ASTERIX records
 *
 * @example
 * const fs = require('fs');
 * const asterix = require('asterix-decoder');
 *
 * fs.createReadStream('data.asterix')
 *   .pipe(asterix.createParseStream({ verbose: true }))
 *   .on('data', (record) => console.log(record))
 *   .on('error', (err) => console.error(err));
 */
function createParseStream(options = {}) {
    ensureInitialized();

    const verbose = options.verbose || false;
    const filterCategory = options.filterCategory;
    const objectMode = options.objectMode !== false; // Default true

    let buffer = Buffer.alloc(0);

    return new Transform({
        objectMode: objectMode,
        transform(chunk, encoding, callback) {
            try {
                // Accumulate data
                buffer = Buffer.concat([buffer, chunk]);

                // Try to parse accumulated data
                if (buffer.length >= 3) { // Minimum ASTERIX block: CAT(1) + LEN(2)
                    let offset = 0;

                    while (offset < buffer.length - 2) {
                        // Check if we have enough data for the next block
                        const blockLen = (buffer[offset + 1] << 8) | buffer[offset + 2];

                        if (blockLen < 3 || offset + blockLen > buffer.length) {
                            // Incomplete block, wait for more data
                            break;
                        }

                        // Extract and parse the block
                        const blockData = buffer.slice(offset, offset + blockLen);

                        try {
                            const records = native.parse(blockData, { verbose });

                            for (const record of records) {
                                // Apply filter
                                if (filterCategory !== undefined && record.category !== filterCategory) {
                                    continue;
                                }

                                if (objectMode) {
                                    this.push(record);
                                } else {
                                    this.push(JSON.stringify(record) + '\n');
                                }
                            }
                        } catch (parseErr) {
                            // Skip malformed block but continue
                            this.emit('warning', `Parse error at offset ${offset}: ${parseErr.message}`);
                        }

                        offset += blockLen;
                    }

                    // Keep unparsed data for next chunk
                    buffer = buffer.slice(offset);
                }

                callback();
            } catch (err) {
                callback(err);
            }
        },
        flush(callback) {
            // Try to parse any remaining data
            if (buffer.length > 0) {
                try {
                    const records = native.parse(buffer, { verbose });
                    for (const record of records) {
                        if (filterCategory !== undefined && record.category !== filterCategory) {
                            continue;
                        }
                        if (objectMode) {
                            this.push(record);
                        } else {
                            this.push(JSON.stringify(record) + '\n');
                        }
                    }
                } catch (err) {
                    this.emit('warning', `Final parse error: ${err.message}`);
                }
            }
            callback();
        }
    });
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
    parseAsync,
    parseWithOffset,
    createParseStream,
    describe,
    isCategoryDefined,
    version: native.version
};
