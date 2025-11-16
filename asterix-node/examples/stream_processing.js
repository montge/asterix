/**
 * Stream Processing Example
 *
 * This example demonstrates real-time processing of ASTERIX data using Node.js streams.
 * This is ideal for processing large files or network streams without loading everything
 * into memory at once.
 */

const asterix = require('asterix-decoder');
const fs = require('fs');
const path = require('path');
const { Transform, Writable } = require('stream');

// Initialize parser
console.log('Initializing ASTERIX decoder...');
asterix.init();

/**
 * Transform stream that parses ASTERIX data
 */
class AsterixParser extends Transform {
    constructor(options = {}) {
        super({ objectMode: true });

        this.buffer = Buffer.alloc(0);
        this.options = options;
        this.recordCount = 0;
        this.bytesProcessed = 0;
    }

    _transform(chunk, encoding, callback) {
        try {
            // Append new data to buffer
            this.buffer = Buffer.concat([this.buffer, chunk]);

            // Try to parse available data
            let offset = 0;
            while (offset < this.buffer.length) {
                const result = asterix.parseWithOffset(
                    this.buffer,
                    offset,
                    10, // Parse 10 blocks at a time
                    this.options
                );

                // Emit parsed records
                for (const record of result.records) {
                    this.recordCount++;
                    this.bytesProcessed += record.length;
                    this.push(record);
                }

                // Update offset
                offset = result.bytesConsumed;

                // Break if no more complete blocks
                if (result.remainingBlocks === 0 || result.bytesConsumed === 0) {
                    break;
                }
            }

            // Keep unparsed data in buffer
            if (offset > 0) {
                this.buffer = this.buffer.slice(offset);
            }

            callback();
        } catch (err) {
            callback(err);
        }
    }

    _flush(callback) {
        // Process any remaining data
        if (this.buffer.length > 0) {
            try {
                const result = asterix.parseWithOffset(
                    this.buffer,
                    0,
                    0, // Parse all remaining
                    this.options
                );

                for (const record of result.records) {
                    this.recordCount++;
                    this.bytesProcessed += record.length;
                    this.push(record);
                }
            } catch (err) {
                // Ignore errors on flush - may be incomplete data
            }
        }

        callback();
    }

    getStats() {
        return {
            records: this.recordCount,
            bytes: this.bytesProcessed
        };
    }
}

/**
 * Writable stream that processes ASTERIX records
 */
class RecordProcessor extends Writable {
    constructor() {
        super({ objectMode: true });

        this.categoryCount = {};
        this.totalRecords = 0;
        this.largestRecord = null;
        this.startTime = Date.now();
    }

    _write(record, encoding, callback) {
        this.totalRecords++;

        // Count by category
        this.categoryCount[record.category] = (this.categoryCount[record.category] || 0) + 1;

        // Track largest record
        if (!this.largestRecord || record.length > this.largestRecord.length) {
            this.largestRecord = record;
        }

        // Progress reporting every 100 records
        if (this.totalRecords % 100 === 0) {
            const elapsed = (Date.now() - this.startTime) / 1000;
            const rps = this.totalRecords / elapsed;
            process.stdout.write(`\rProcessed: ${this.totalRecords} records (${rps.toFixed(0)} rec/s)`);
        }

        callback();
    }

    _final(callback) {
        console.log('\n'); // New line after progress
        callback();
    }

    getStats() {
        return {
            totalRecords: this.totalRecords,
            categoryCount: this.categoryCount,
            largestRecord: this.largestRecord,
            elapsedTime: Date.now() - this.startTime
        };
    }
}

/**
 * Filter stream that only passes specific categories
 */
class CategoryFilter extends Transform {
    constructor(categories) {
        super({ objectMode: true });
        this.categories = new Set(categories);
        this.filtered = 0;
        this.passed = 0;
    }

    _transform(record, encoding, callback) {
        if (this.categories.has(record.category)) {
            this.passed++;
            this.push(record);
        } else {
            this.filtered++;
        }
        callback();
    }

    getStats() {
        return {
            passed: this.passed,
            filtered: this.filtered
        };
    }
}

/**
 * JSON export stream
 */
class JSONExporter extends Writable {
    constructor(outputPath) {
        super({ objectMode: true });
        this.outputPath = outputPath;
        this.records = [];
    }

    _write(record, encoding, callback) {
        this.records.push(record);
        callback();
    }

    _final(callback) {
        const output = {
            metadata: {
                exported_at: new Date().toISOString(),
                total_records: this.records.length
            },
            records: this.records
        };

        fs.writeFileSync(this.outputPath, JSON.stringify(output, null, 2));
        console.log(`Exported ${this.records.length} records to ${this.outputPath}`);
        callback();
    }
}

// Example 1: Basic streaming pipeline
console.log('\n=== Example 1: Basic Streaming Pipeline ===');

const dataPath = path.join(__dirname, '..', '..', 'install', 'share', 'asterix', 'samples', 'cat_062_065.pcap');

if (!fs.existsSync(dataPath)) {
    console.error(`File not found: ${dataPath}`);
    console.log('\nNote: Build and install the C++ executable first.');
    process.exit(1);
}

const parser1 = new AsterixParser({ verbose: false });
const processor1 = new RecordProcessor();

console.log(`Reading from: ${dataPath}`);

fs.createReadStream(dataPath)
    .pipe(parser1)
    .pipe(processor1)
    .on('finish', () => {
        const parserStats = parser1.getStats();
        const processorStats = processor1.getStats();

        console.log('\n=== Streaming Results ===');
        console.log(`Total records: ${processorStats.totalRecords}`);
        console.log(`Bytes processed: ${parserStats.bytes}`);
        console.log(`Parse time: ${processorStats.elapsedTime}ms`);
        console.log(`Throughput: ${(processorStats.totalRecords / processorStats.elapsedTime * 1000).toFixed(0)} rec/s`);

        console.log('\nRecords by category:');
        for (const [cat, count] of Object.entries(processorStats.categoryCount).sort((a, b) => Number(a[0]) - Number(b[0]))) {
            const catNum = Number(cat);
            const desc = asterix.isCategoryDefined(catNum) ? asterix.describe(catNum) : 'Unknown';
            console.log(`  CAT${cat.padStart(3, '0')}: ${count.toString().padStart(4)} - ${desc}`);
        }

        if (processorStats.largestRecord) {
            console.log(`\nLargest record: ${processorStats.largestRecord.length} bytes (CAT${processorStats.largestRecord.category.toString().padStart(3, '0')})`);
        }

        // Example 2: Filtering pipeline
        console.log('\n=== Example 2: Filtering Pipeline (CAT062 only) ===');

        const parser2 = new AsterixParser({ verbose: false });
        const filter = new CategoryFilter([62]);
        const exporter = new JSONExporter(path.join(__dirname, '..', 'cat062_only.json'));

        fs.createReadStream(dataPath)
            .pipe(parser2)
            .pipe(filter)
            .pipe(exporter)
            .on('finish', () => {
                const filterStats = filter.getStats();

                console.log(`\nFilter results:`);
                console.log(`  Passed: ${filterStats.passed} records`);
                console.log(`  Filtered out: ${filterStats.filtered} records`);
                console.log(`  Efficiency: ${(filterStats.passed / (filterStats.passed + filterStats.filtered) * 100).toFixed(1)}% relevant`);

                console.log('\n=== Streaming examples complete ===');
            });
    })
    .on('error', (err) => {
        console.error('\nStreaming error:', err.message);
        process.exit(1);
    });
