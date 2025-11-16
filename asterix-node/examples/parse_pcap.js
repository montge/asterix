/**
 * PCAP File Parsing Example
 *
 * This example demonstrates parsing ASTERIX data from PCAP network capture files.
 * PCAP files are commonly created by Wireshark, tcpdump, or other network capture tools.
 */

const asterix = require('asterix-decoder');
const fs = require('fs');
const path = require('path');

// Initialize parser
console.log('Initializing ASTERIX decoder...');
asterix.init();

// Example PCAP file path (relative to repository root)
const pcapPath = path.join(__dirname, '..', '..', 'install', 'share', 'asterix', 'samples', 'cat_062_065.pcap');

console.log(`\nParsing PCAP file: ${pcapPath}`);

// Check if file exists
if (!fs.existsSync(pcapPath)) {
    console.error(`PCAP file not found: ${pcapPath}`);
    console.log('\nNote: Build and install the C++ executable first:');
    console.log('  cmake -B build -DCMAKE_BUILD_TYPE=Release');
    console.log('  cmake --build build --parallel');
    console.log('  cmake --install build');
    process.exit(1);
}

try {
    // Read PCAP file
    const pcapData = fs.readFileSync(pcapPath);
    console.log(`Read ${pcapData.length} bytes from PCAP file`);

    // Parse with verbose output
    const startTime = Date.now();
    const records = asterix.parse(pcapData, {
        verbose: true
    });
    const parseTime = Date.now() - startTime;

    console.log(`\nParsed ${records.length} records in ${parseTime}ms`);
    console.log(`Average: ${(parseTime / records.length).toFixed(2)}ms per record`);

    // Count records by category
    const categoryCount = {};
    for (const record of records) {
        categoryCount[record.category] = (categoryCount[record.category] || 0) + 1;
    }

    console.log('\nRecords by category:');
    for (const [cat, count] of Object.entries(categoryCount).sort((a, b) => Number(a[0]) - Number(b[0]))) {
        const catNum = Number(cat);
        const desc = asterix.isCategoryDefined(catNum) ? asterix.describe(catNum) : 'Unknown';
        console.log(`  CAT${cat.padStart(3, '0')}: ${count.toString().padStart(4)} records - ${desc}`);
    }

    // Display first record in detail
    if (records.length > 0) {
        console.log('\n=== First Record Details ===');
        const record = records[0];

        console.log(`Category: ${record.category}`);
        console.log(`Length: ${record.length} bytes`);
        console.log(`Timestamp: ${new Date(record.timestamp_ms).toISOString()}`);
        console.log(`CRC: 0x${record.crc.toString(16).padStart(8, '0')}`);

        if (record.hex_data) {
            console.log(`Hex data (first 128 chars): ${record.hex_data.substring(0, 128)}...`);
        }

        console.log(`\nData items (${Object.keys(record.items).length}):`);
        for (const [itemId, itemData] of Object.entries(record.items)) {
            console.log(`  ${itemId}:`);
            const jsonStr = JSON.stringify(itemData, null, 4);
            const lines = jsonStr.split('\n');
            for (const line of lines.slice(0, 10)) {
                console.log(`    ${line}`);
            }
            if (lines.length > 10) {
                console.log(`    ... (${lines.length - 10} more lines)`);
            }
        }
    }

    // Display statistics
    console.log('\n=== Statistics ===');
    const totalBytes = records.reduce((sum, r) => sum + r.length, 0);
    console.log(`Total data: ${totalBytes} bytes`);
    console.log(`Average record size: ${(totalBytes / records.length).toFixed(2)} bytes`);
    console.log(`Throughput: ${(totalBytes / parseTime * 1000 / 1024).toFixed(2)} KB/s`);

    // Find records with specific characteristics
    console.log('\n=== Record Analysis ===');

    // Records with most data items
    const recordsByItemCount = records
        .map((r, idx) => ({ index: idx, count: Object.keys(r.items).length, record: r }))
        .sort((a, b) => b.count - a.count);

    if (recordsByItemCount.length > 0) {
        const maxItems = recordsByItemCount[0];
        console.log(`Record with most items: #${maxItems.index + 1} (${maxItems.count} items, CAT${maxItems.record.category.toString().padStart(3, '0')})`);
    }

    // Largest record
    const recordsBySize = records
        .map((r, idx) => ({ index: idx, size: r.length, record: r }))
        .sort((a, b) => b.size - a.size);

    if (recordsBySize.length > 0) {
        const largest = recordsBySize[0];
        console.log(`Largest record: #${largest.index + 1} (${largest.size} bytes, CAT${largest.record.category.toString().padStart(3, '0')})`);
    }

    // Export to JSON
    const jsonOutputPath = path.join(__dirname, '..', 'output.json');
    console.log(`\nExporting to JSON: ${jsonOutputPath}`);

    const jsonOutput = {
        metadata: {
            source: pcapPath,
            parsed_at: new Date().toISOString(),
            total_records: records.length,
            total_bytes: totalBytes,
            parse_time_ms: parseTime,
            categories: categoryCount
        },
        records: records
    };

    fs.writeFileSync(jsonOutputPath, JSON.stringify(jsonOutput, null, 2));
    console.log(`Exported ${records.length} records to JSON`);

} catch (err) {
    console.error('\nError parsing PCAP file:', err.message);
    console.error('Stack:', err.stack);
    process.exit(1);
}

console.log('\n=== PCAP parsing complete ===');
