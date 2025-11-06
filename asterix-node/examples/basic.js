/**
 * Basic ASTERIX parsing example
 *
 * This example demonstrates the simplest usage of the asterix-decoder package.
 */

const asterix = require('asterix-decoder');
const fs = require('fs');
const path = require('path');

// Initialize parser with default configuration
console.log('Initializing ASTERIX decoder...');
asterix.init();

// Check which categories are available
console.log('\nAvailable categories:');
const commonCategories = [1, 2, 8, 10, 19, 20, 21, 23, 25, 34, 48, 62, 63, 65];
for (const cat of commonCategories) {
    if (asterix.isCategoryDefined(cat)) {
        const desc = asterix.describe(cat);
        console.log(`  CAT${cat.toString().padStart(3, '0')}: ${desc}`);
    }
}

// Example 1: Parse from file
console.log('\n=== Example 1: Parse from file ===');

// Create sample data (CAT048 minimal data block)
const sampleData = Buffer.from([
    0x30, // Category 48
    0x00, 0x0E, // Length: 14 bytes
    0xF0, // FSPEC: I048/010, I048/020, I048/040, I048/070 present
    // I048/010 (Data Source Identifier)
    0x12, 0x34, // SAC=0x12, SIC=0x34
    // I048/020 (Target Report Descriptor)
    0x01, 0x02, 0x03,
    // I048/040 (Measured Position)
    0x00, 0x00, 0x00, 0x00
]);

console.log(`Sample data: ${sampleData.toString('hex')}`);

try {
    const records = asterix.parse(sampleData, { verbose: true });

    console.log(`Parsed ${records.length} record(s):`);

    for (const record of records) {
        console.log(`\nRecord ${records.indexOf(record) + 1}:`);
        console.log(`  Category: ${record.category}`);
        console.log(`  Length: ${record.length} bytes`);
        console.log(`  Timestamp: ${new Date(record.timestamp_ms).toISOString()}`);
        console.log(`  CRC: 0x${record.crc.toString(16).padStart(8, '0')}`);

        if (record.hex_data) {
            console.log(`  Hex data: ${record.hex_data.substring(0, 64)}...`);
        }

        console.log(`  Data items (${Object.keys(record.items).length}):`);
        for (const [itemId, itemData] of Object.entries(record.items)) {
            console.log(`    ${itemId}:`, JSON.stringify(itemData, null, 2).substring(0, 100));
        }
    }
} catch (err) {
    console.error('Parsing error:', err.message);
    if (err.message.includes('not defined')) {
        console.log('\nNote: CAT048 may not be loaded. Try initializing with config directory:');
        console.log('  asterix.init("/path/to/asterix/config")');
    }
}

// Example 2: Get descriptions
console.log('\n=== Example 2: Get descriptions ===');

if (asterix.isCategoryDefined(48)) {
    try {
        // Category description
        const catDesc = asterix.describe(48);
        console.log(`CAT048: ${catDesc}`);

        // Item description
        const itemDesc = asterix.describe(48, '010');
        console.log(`I048/010: ${itemDesc}`);

        // Field description
        const fieldDesc = asterix.describe(48, '010', 'SAC');
        console.log(`I048/010/SAC: ${fieldDesc}`);

        // Field value description
        const valueDesc = asterix.describe(48, '010', 'SAC', '18');
        console.log(`I048/010/SAC=18: ${valueDesc}`);
    } catch (err) {
        console.error('Description error:', err.message);
    }
} else {
    console.log('CAT048 not loaded');
}

// Example 3: Parse options
console.log('\n=== Example 3: Parse with options ===');

try {
    // Filter to specific category
    const cat48Records = asterix.parse(sampleData, {
        filterCategory: 48,
        maxRecords: 10,
        verbose: false
    });

    console.log(`Filtered records (CAT048 only): ${cat48Records.length}`);

    // Verify all records are CAT048
    const allCat48 = cat48Records.every(r => r.category === 48);
    console.log(`All records are CAT048: ${allCat48}`);
} catch (err) {
    console.error('Parsing error:', err.message);
}

// Example 4: Error handling
console.log('\n=== Example 4: Error handling ===');

// Empty buffer
try {
    asterix.parse(Buffer.alloc(0));
} catch (err) {
    console.log(`Empty buffer error (expected): ${err.message}`);
}

// Invalid type
try {
    asterix.parse('not a buffer');
} catch (err) {
    console.log(`Type error (expected): ${err.message}`);
}

// Oversized data
try {
    const hugeBuffer = Buffer.alloc(65537);
    asterix.parse(hugeBuffer);
} catch (err) {
    console.log(`Oversized data error (expected): ${err.message}`);
}

// Invalid category
try {
    asterix.describe(0); // Category 0 is invalid
} catch (err) {
    console.log(`Invalid category error (expected): ${err.message}`);
}

console.log('\n=== Basic example complete ===');
console.log(`asterix-decoder version: ${asterix.version}`);
