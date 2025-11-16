/**
 * Incremental Parsing Example
 *
 * This example demonstrates incremental parsing for large ASTERIX files
 * that don't fit in memory, or for processing data in chunks.
 *
 * Incremental parsing is useful for:
 * - Large files (> 1GB)
 * - Streaming data from network
 * - Memory-constrained environments
 * - Real-time processing with progress reporting
 */

const asterix = require('asterix-decoder');
const fs = require('fs');
const path = require('path');

// Initialize parser
console.log('Initializing ASTERIX decoder...');
asterix.init();

// Example file path
const dataPath = path.join(__dirname, '..', '..', 'install', 'share', 'asterix', 'samples', 'cat_062_065.pcap');

console.log(`\nIncremental parsing: ${dataPath}`);

// Check if file exists
if (!fs.existsSync(dataPath)) {
    console.error(`File not found: ${dataPath}`);
    console.log('\nNote: Build and install the C++ executable first:');
    console.log('  cmake -B build -DCMAKE_BUILD_TYPE=Release');
    console.log('  cmake --build build --parallel');
    console.log('  cmake --install build');
    process.exit(1);
}

try {
    // Read entire file
    const allData = fs.readFileSync(dataPath);
    console.log(`File size: ${allData.length} bytes (${(allData.length / 1024).toFixed(2)} KB)`);

    // Configuration
    const CHUNK_SIZE = 100; // Parse 100 data blocks at a time
    const PROGRESS_INTERVAL = 500; // Report progress every 500 records

    // Statistics
    let totalRecords = 0;
    let offset = 0;
    let iteration = 0;
    const startTime = Date.now();
    const categoryCount = {};
    const itemCount = {};

    console.log(`\nParsing in chunks of ${CHUNK_SIZE} blocks...`);
    console.log('Progress: |' + '='.repeat(50) + '|');
    console.log('          |', { end: '' });

    let lastProgressChars = 0;

    // Incremental parsing loop
    while (offset < allData.length) {
        iteration++;

        // Parse next chunk
        const result = asterix.parseWithOffset(allData, offset, CHUNK_SIZE, {
            verbose: false // Disable verbose for performance
        });

        // Update offset
        offset = result.bytesConsumed;

        // Process records from this chunk
        for (const record of result.records) {
            totalRecords++;

            // Count by category
            categoryCount[record.category] = (categoryCount[record.category] || 0) + 1;

            // Count data items
            for (const itemId of Object.keys(record.items)) {
                itemCount[itemId] = (itemCount[itemId] || 0) + 1;
            }

            // Progress reporting
            if (totalRecords % PROGRESS_INTERVAL === 0) {
                const progressChars = Math.floor((offset / allData.length) * 50);
                if (progressChars > lastProgressChars) {
                    process.stdout.write('='.repeat(progressChars - lastProgressChars));
                    lastProgressChars = progressChars;
                }
            }
        }

        // Check if done
        if (result.remainingBlocks === 0) {
            console.log('='.repeat(50 - lastProgressChars) + '|');
            break;
        }

        // Safety check to prevent infinite loops
        if (iteration > 10000) {
            console.error('\nToo many iterations - possible infinite loop');
            break;
        }
    }

    const elapsedTime = Date.now() - startTime;

    // Display results
    console.log(`\n\n=== Incremental Parsing Complete ===`);
    console.log(`Total records: ${totalRecords}`);
    console.log(`Iterations: ${iteration}`);
    console.log(`Average chunk size: ${(totalRecords / iteration).toFixed(2)} records`);
    console.log(`Parse time: ${elapsedTime}ms`);
    console.log(`Throughput: ${(totalRecords / elapsedTime * 1000).toFixed(0)} records/sec`);
    console.log(`Bytes processed: ${offset} / ${allData.length}`);

    // Category distribution
    console.log(`\n=== Records by Category ===`);
    const sortedCategories = Object.entries(categoryCount).sort((a, b) => Number(a[0]) - Number(b[0]));
    for (const [cat, count] of sortedCategories) {
        const catNum = Number(cat);
        const desc = asterix.isCategoryDefined(catNum) ? asterix.describe(catNum) : 'Unknown';
        const percentage = (count / totalRecords * 100).toFixed(1);
        console.log(`  CAT${cat.padStart(3, '0')}: ${count.toString().padStart(5)} (${percentage}%) - ${desc}`);
    }

    // Most common data items
    console.log(`\n=== Most Common Data Items ===`);
    const sortedItems = Object.entries(itemCount).sort((a, b) => b[1] - a[1]);
    for (const [itemId, count] of sortedItems.slice(0, 10)) {
        const percentage = (count / totalRecords * 100).toFixed(1);
        console.log(`  ${itemId}: ${count.toString().padStart(5)} (${percentage}%)`);
    }

    // Memory efficiency comparison
    console.log(`\n=== Memory Efficiency ===`);
    console.log(`Full parse would require: ${(allData.length / 1024 / 1024).toFixed(2)} MB`);
    console.log(`Incremental parsing chunk: ~${(CHUNK_SIZE * 100 / 1024).toFixed(2)} KB estimated`);
    console.log(`Memory savings: ~${(100 - (CHUNK_SIZE * 100 / allData.length * 100)).toFixed(1)}%`);

    // Example: Filter records by category during incremental parsing
    console.log(`\n=== Filtered Incremental Parsing (CAT062 only) ===`);
    offset = 0;
    let cat062Count = 0;
    const cat062StartTime = Date.now();

    while (offset < allData.length) {
        const result = asterix.parseWithOffset(allData, offset, CHUNK_SIZE, {
            filterCategory: 62,
            verbose: false
        });

        cat062Count += result.records.length;
        offset = result.bytesConsumed;

        if (result.remainingBlocks === 0) {
            break;
        }
    }

    const cat062ParseTime = Date.now() - cat062StartTime;
    console.log(`CAT062 records: ${cat062Count}`);
    console.log(`Parse time: ${cat062ParseTime}ms`);
    console.log(`Speedup vs full parse: ${(elapsedTime / cat062ParseTime).toFixed(2)}x`);

    // Export summary
    const summaryPath = path.join(__dirname, '..', 'incremental_summary.json');
    const summary = {
        metadata: {
            source: dataPath,
            parsed_at: new Date().toISOString(),
            chunk_size: CHUNK_SIZE,
            total_records: totalRecords,
            total_bytes: offset,
            parse_time_ms: elapsedTime,
            iterations: iteration
        },
        categories: categoryCount,
        top_items: Object.fromEntries(sortedItems.slice(0, 20))
    };

    fs.writeFileSync(summaryPath, JSON.stringify(summary, null, 2));
    console.log(`\nSummary exported to: ${summaryPath}`);

} catch (err) {
    console.error('\nError during incremental parsing:', err.message);
    console.error('Stack:', err.stack);
    process.exit(1);
}

console.log('\n=== Example complete ===');
