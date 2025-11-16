/**
 * File Watcher Example
 *
 * This example demonstrates real-time monitoring of a directory for new ASTERIX files.
 * When a new file is detected, it is automatically parsed and results are saved.
 *
 * This is useful for:
 * - Automated processing of incoming ASTERIX data
 * - Real-time surveillance data pipelines
 * - Integration with data collection systems
 * - Batch processing workflows
 */

const asterix = require('asterix-decoder');
const chokidar = require('chokidar');
const fs = require('fs');
const path = require('path');

// Initialize ASTERIX decoder
console.log('Initializing ASTERIX decoder...');
try {
    asterix.init();
    console.log('ASTERIX decoder initialized');
} catch (err) {
    console.error('Failed to initialize ASTERIX decoder:', err.message);
    process.exit(1);
}

// Configuration
const WATCH_DIR = process.argv[2] || path.join(__dirname, '..', 'watch');
const OUTPUT_DIR = process.argv[3] || path.join(__dirname, '..', 'output');
const WATCH_PATTERN = '*.{asterix,pcap,ast,raw}';

// Create directories if they don't exist
if (!fs.existsSync(WATCH_DIR)) {
    fs.mkdirSync(WATCH_DIR, { recursive: true });
    console.log(`Created watch directory: ${WATCH_DIR}`);
}

if (!fs.existsSync(OUTPUT_DIR)) {
    fs.mkdirSync(OUTPUT_DIR, { recursive: true });
    console.log(`Created output directory: ${OUTPUT_DIR}`);
}

// Statistics
const stats = {
    filesProcessed: 0,
    totalRecords: 0,
    totalBytes: 0,
    errors: 0,
    startTime: Date.now()
};

/**
 * Process a single ASTERIX file
 */
function processFile(filePath) {
    const filename = path.basename(filePath);
    const startTime = Date.now();

    console.log(`\n=== Processing: ${filename} ===`);

    try {
        // Read file
        const data = fs.readFileSync(filePath);
        console.log(`  Size: ${data.length} bytes (${(data.length / 1024).toFixed(2)} KB)`);

        // Parse ASTERIX data
        const parseStart = Date.now();
        const records = asterix.parse(data, {
            verbose: false
        });
        const parseTime = Date.now() - parseStart;

        console.log(`  Parsed: ${records.length} records in ${parseTime}ms`);

        // Analyze records
        const categoryCount = {};
        for (const record of records) {
            categoryCount[record.category] = (categoryCount[record.category] || 0) + 1;
        }

        console.log(`  Categories: ${Object.keys(categoryCount).join(', ')}`);

        // Create output filename (preserve original name + timestamp)
        const timestamp = new Date().toISOString().replace(/[:.]/g, '-');
        const outputFilename = `${path.parse(filename).name}_${timestamp}.json`;
        const outputPath = path.join(OUTPUT_DIR, outputFilename);

        // Export to JSON
        const output = {
            metadata: {
                source_file: filename,
                source_path: filePath,
                processed_at: new Date().toISOString(),
                file_size: data.length,
                total_records: records.length,
                parse_time_ms: parseTime,
                categories: categoryCount
            },
            records: records
        };

        fs.writeFileSync(outputPath, JSON.stringify(output, null, 2));
        console.log(`  Exported: ${outputPath}`);

        // Update statistics
        stats.filesProcessed++;
        stats.totalRecords += records.length;
        stats.totalBytes += data.length;

        const totalTime = Date.now() - startTime;
        console.log(`  Total time: ${totalTime}ms`);
        console.log(`  ✓ Success`);

        // Optional: Delete source file after processing
        // Uncomment to enable auto-deletion
        // fs.unlinkSync(filePath);
        // console.log(`  Deleted: ${filename}`);

    } catch (err) {
        console.error(`  ✗ Error: ${err.message}`);
        stats.errors++;

        // Log error to file
        const errorLogPath = path.join(OUTPUT_DIR, 'errors.log');
        const errorEntry = `${new Date().toISOString()} - ${filename}: ${err.message}\n`;
        fs.appendFileSync(errorLogPath, errorEntry);
    }
}

/**
 * Display statistics
 */
function displayStats() {
    const uptime = Math.floor((Date.now() - stats.startTime) / 1000);
    const avgRecordsPerFile = stats.filesProcessed > 0 ? (stats.totalRecords / stats.filesProcessed).toFixed(1) : 0;
    const avgBytesPerFile = stats.filesProcessed > 0 ? (stats.totalBytes / stats.filesProcessed / 1024).toFixed(2) : 0;

    console.log('\n=== Statistics ===');
    console.log(`  Uptime: ${uptime}s`);
    console.log(`  Files processed: ${stats.filesProcessed}`);
    console.log(`  Total records: ${stats.totalRecords}`);
    console.log(`  Total bytes: ${(stats.totalBytes / 1024 / 1024).toFixed(2)} MB`);
    console.log(`  Errors: ${stats.errors}`);
    console.log(`  Average: ${avgRecordsPerFile} records/file, ${avgBytesPerFile} KB/file`);
    console.log('');
}

/**
 * Setup file watcher
 */
console.log(`\n=== ASTERIX File Watcher ===`);
console.log(`Watching: ${WATCH_DIR}`);
console.log(`Pattern: ${WATCH_PATTERN}`);
console.log(`Output: ${OUTPUT_DIR}`);
console.log(`\nWaiting for files...\n`);

const watcher = chokidar.watch(path.join(WATCH_DIR, WATCH_PATTERN), {
    persistent: true,
    ignoreInitial: false,
    awaitWriteFinish: {
        stabilityThreshold: 2000,
        pollInterval: 100
    }
});

watcher
    .on('add', (filePath) => {
        processFile(filePath);
    })
    .on('change', (filePath) => {
        console.log(`\nFile changed: ${path.basename(filePath)}`);
        console.log('  (skipping - only processing new files)');
    })
    .on('unlink', (filePath) => {
        console.log(`\nFile removed: ${path.basename(filePath)}`);
    })
    .on('error', (error) => {
        console.error(`\nWatcher error: ${error.message}`);
    });

// Display statistics every 60 seconds
setInterval(displayStats, 60000);

// Handle graceful shutdown
process.on('SIGINT', () => {
    console.log('\n\n=== Shutting down ===');
    displayStats();

    watcher.close().then(() => {
        console.log('Watcher closed');
        process.exit(0);
    });
});

process.on('SIGTERM', () => {
    console.log('\n\n=== Shutting down ===');
    displayStats();

    watcher.close().then(() => {
        console.log('Watcher closed');
        process.exit(0);
    });
});

// Display usage instructions
console.log('Usage:');
console.log(`  node file_watcher.js [watch_dir] [output_dir]`);
console.log(`\nDefault directories:`);
console.log(`  Watch: ${WATCH_DIR}`);
console.log(`  Output: ${OUTPUT_DIR}`);
console.log(`\nTo test:`);
console.log(`  1. Copy an ASTERIX or PCAP file to the watch directory`);
console.log(`  2. Check the output directory for JSON results`);
console.log(`\nPress Ctrl+C to stop\n`);
