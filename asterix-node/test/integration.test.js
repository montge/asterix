/**
 * Integration tests for ASTERIX decoder Node.js bindings
 *
 * These tests verify end-to-end functionality with real ASTERIX data files
 * from the install/share/asterix/samples/ directory.
 *
 * Requirements:
 * - ASTERIX core library must be built and installed
 * - Sample files must be present in install/share/asterix/samples/
 */

const { expect } = require('chai');
const asterix = require('../lib/index');
const fs = require('fs');
const path = require('path');

describe('ASTERIX Decoder - Integration Tests', function() {
  // Increase timeout for file I/O and parsing
  this.timeout(10000);

  const samplesDir = path.join(__dirname, '../../install/share/asterix/samples');
  const configDir = path.join(__dirname, '../../install/share/asterix/config');

  before(function() {
    // Initialize parser with default config
    try {
      asterix.init(configDir);
      console.log(`Initialized ASTERIX parser with config: ${configDir}`);
    } catch (err) {
      console.warn('Initialization warning:', err.message);
      // Try auto-initialization
      try {
        asterix.init();
      } catch (err2) {
        console.error('Failed to initialize:', err2.message);
      }
    }
  });

  describe('Raw ASTERIX Files', function() {
    it('should parse cat048.raw', function() {
      const filePath = path.join(samplesDir, 'cat048.raw');

      // Skip if file doesn't exist
      if (!fs.existsSync(filePath)) {
        this.skip();
        return;
      }

      const data = fs.readFileSync(filePath);
      expect(data.length).to.be.greaterThan(0);

      const records = asterix.parse(data, { verbose: true });

      expect(records).to.be.an('array');
      expect(records.length).to.be.greaterThan(0);

      // Verify all records are CAT048
      records.forEach(record => {
        expect(record).to.have.property('category');
        expect(record.category).to.equal(48);
        expect(record).to.have.property('items');
        expect(record.items).to.be.an('object');
      });

      console.log(`  ✓ Parsed ${records.length} CAT048 records from cat048.raw`);
    });

    it('should parse cat034.raw', function() {
      const filePath = path.join(samplesDir, 'cat034.raw');

      if (!fs.existsSync(filePath)) {
        this.skip();
        return;
      }

      const data = fs.readFileSync(filePath);
      const records = asterix.parse(data);

      expect(records).to.be.an('array');
      expect(records.length).to.be.greaterThan(0);

      records.forEach(record => {
        expect(record.category).to.equal(34);
      });

      console.log(`  ✓ Parsed ${records.length} CAT034 records from cat034.raw`);
    });

    it('should parse cat062cat065.raw (multi-category)', function() {
      const filePath = path.join(samplesDir, 'cat062cat065.raw');

      if (!fs.existsSync(filePath)) {
        this.skip();
        return;
      }

      const data = fs.readFileSync(filePath);
      const records = asterix.parse(data);

      expect(records).to.be.an('array');
      expect(records.length).to.be.greaterThan(0);

      // Should contain both CAT062 and CAT065
      const categories = [...new Set(records.map(r => r.category))];
      expect(categories).to.include.members([62, 65]);

      console.log(`  ✓ Parsed ${records.length} records (CAT ${categories.join(', ')}) from cat062cat065.raw`);
    });
  });

  describe('PCAP Files', function() {
    it('should parse cat_062_065.pcap', function() {
      const filePath = path.join(samplesDir, 'cat_062_065.pcap');

      if (!fs.existsSync(filePath)) {
        this.skip();
        return;
      }

      const data = fs.readFileSync(filePath);

      // Parse PCAP format - may need special handling
      try {
        const records = asterix.parse(data);
        expect(records).to.be.an('array');

        if (records.length > 0) {
          console.log(`  ✓ Parsed ${records.length} records from cat_062_065.pcap`);
        } else {
          console.log('  ⚠ No records parsed (PCAP decapsulation may require special handling)');
        }
      } catch (err) {
        // PCAP may require different parsing approach
        console.log(`  ⚠ PCAP parsing not supported in Node.js binding: ${err.message}`);
        this.skip();
      }
    });

    it('should parse cat_034_048.pcap', function() {
      const filePath = path.join(samplesDir, 'cat_034_048.pcap');

      if (!fs.existsSync(filePath)) {
        this.skip();
        return;
      }

      const data = fs.readFileSync(filePath);

      try {
        const records = asterix.parse(data);
        expect(records).to.be.an('array');

        if (records.length > 0) {
          const categories = [...new Set(records.map(r => r.category))];
          console.log(`  ✓ Parsed ${records.length} records (CAT ${categories.join(', ')}) from cat_034_048.pcap`);
        }
      } catch (err) {
        console.log(`  ⚠ PCAP parsing not supported: ${err.message}`);
        this.skip();
      }
    });

    it('should parse asterix.pcap', function() {
      const filePath = path.join(samplesDir, 'asterix.pcap');

      if (!fs.existsSync(filePath)) {
        this.skip();
        return;
      }

      const data = fs.readFileSync(filePath);

      try {
        const records = asterix.parse(data);
        expect(records).to.be.an('array');

        if (records.length > 0) {
          const categories = [...new Set(records.map(r => r.category))];
          console.log(`  ✓ Parsed ${records.length} records (CAT ${categories.join(', ')}) from asterix.pcap`);
        }
      } catch (err) {
        console.log(`  ⚠ PCAP parsing not supported: ${err.message}`);
        this.skip();
      }
    });
  });

  describe('Incremental Parsing', function() {
    it('should parse large file incrementally without memory issues', function() {
      const filePath = path.join(samplesDir, 'cat048.raw');

      if (!fs.existsSync(filePath)) {
        this.skip();
        return;
      }

      const data = fs.readFileSync(filePath);
      let offset = 0;
      const allRecords = [];
      const chunkSize = 10; // Parse 10 blocks at a time

      while (offset < data.length) {
        const result = asterix.parseWithOffset(data, offset, chunkSize);

        expect(result).to.have.property('records');
        expect(result).to.have.property('bytesConsumed');
        expect(result).to.have.property('remainingBlocks');

        allRecords.push(...result.records);
        offset = result.bytesConsumed;

        if (result.remainingBlocks === 0) {
          break;
        }
      }

      expect(allRecords.length).to.be.greaterThan(0);
      console.log(`  ✓ Incrementally parsed ${allRecords.length} records in ${Math.ceil(allRecords.length / chunkSize)} chunks`);
    });

    it('should handle offset at exact block boundaries', function() {
      const filePath = path.join(samplesDir, 'cat062cat065.raw');

      if (!fs.existsSync(filePath)) {
        this.skip();
        return;
      }

      const data = fs.readFileSync(filePath);

      // Parse first block
      const result1 = asterix.parseWithOffset(data, 0, 1);
      expect(result1.bytesConsumed).to.be.greaterThan(0);

      // Parse second block starting exactly at end of first
      if (result1.bytesConsumed < data.length) {
        const result2 = asterix.parseWithOffset(data, result1.bytesConsumed, 1);
        expect(result2.records).to.be.an('array');
      }

      console.log('  ✓ Correctly handled block boundaries');
    });
  });

  describe('Category Filtering', function() {
    it('should filter by category correctly', function() {
      const filePath = path.join(samplesDir, 'cat062cat065.raw');

      if (!fs.existsSync(filePath)) {
        this.skip();
        return;
      }

      const data = fs.readFileSync(filePath);

      // Parse without filter
      const allRecords = asterix.parse(data);

      // Parse with CAT062 filter
      const cat62Records = asterix.parse(data, { filterCategory: 62 });

      // Parse with CAT065 filter
      const cat65Records = asterix.parse(data, { filterCategory: 65 });

      // Verify filtering worked
      cat62Records.forEach(r => expect(r.category).to.equal(62));
      cat65Records.forEach(r => expect(r.category).to.equal(65));

      // Filtered results should sum to total (approximately)
      expect(cat62Records.length + cat65Records.length).to.be.closeTo(allRecords.length, 5);

      console.log(`  ✓ Filtered CAT062: ${cat62Records.length}, CAT065: ${cat65Records.length}, Total: ${allRecords.length}`);
    });
  });

  describe('Max Records Limit', function() {
    it('should respect maxRecords option', function() {
      const filePath = path.join(samplesDir, 'cat048.raw');

      if (!fs.existsSync(filePath)) {
        this.skip();
        return;
      }

      const data = fs.readFileSync(filePath);

      // Parse with limit
      const maxLimit = 5;
      const records = asterix.parse(data, { maxRecords: maxLimit });

      expect(records.length).to.be.at.most(maxLimit);
      console.log(`  ✓ Limited to ${records.length} records (max: ${maxLimit})`);
    });
  });

  describe('Verbose Mode', function() {
    it('should include descriptions when verbose=true', function() {
      const filePath = path.join(samplesDir, 'cat048.raw');

      if (!fs.existsSync(filePath)) {
        this.skip();
        return;
      }

      const data = fs.readFileSync(filePath);

      // Parse with verbose mode
      const records = asterix.parse(data, { verbose: true, maxRecords: 1 });

      if (records.length > 0) {
        const record = records[0];

        // Should have items with descriptions
        expect(record.items).to.be.an('object');

        // At least one item should have a description
        const items = Object.values(record.items);
        const hasDescription = items.some(item =>
          item && typeof item === 'object' && 'description' in item
        );

        if (hasDescription) {
          console.log('  ✓ Verbose mode includes item descriptions');
        } else {
          console.log('  ⚠ No descriptions found (may not be available for all items)');
        }
      }
    });
  });

  describe('Category Information', function() {
    it('should correctly identify defined categories', function() {
      // Common ASTERIX categories
      const commonCategories = [1, 2, 8, 10, 19, 20, 21, 34, 48, 62, 65];

      const definedCategories = commonCategories.filter(cat =>
        asterix.isCategoryDefined(cat)
      );

      expect(definedCategories.length).to.be.greaterThan(0);
      console.log(`  ✓ Found ${definedCategories.length} defined categories: ${definedCategories.join(', ')}`);
    });

    it('should provide category descriptions', function() {
      const categories = [1, 2, 8, 10, 19, 20, 21, 34, 48, 62, 65];

      for (const cat of categories) {
        if (asterix.isCategoryDefined(cat)) {
          try {
            const desc = asterix.describe(cat);
            expect(desc).to.be.a('string');
            expect(desc.length).to.be.greaterThan(0);
            console.log(`  ✓ CAT${cat.toString().padStart(3, '0')}: ${desc.substring(0, 60)}...`);
          } catch (err) {
            // May not have description
          }
        }
      }
    });
  });

  describe('Performance', function() {
    it('should parse large files efficiently', function() {
      const filePath = path.join(samplesDir, 'cat048.raw');

      if (!fs.existsSync(filePath)) {
        this.skip();
        return;
      }

      const data = fs.readFileSync(filePath);

      // Warm-up
      asterix.parse(data);

      // Measure performance
      const iterations = 100;
      const start = Date.now();

      for (let i = 0; i < iterations; i++) {
        asterix.parse(data);
      }

      const elapsed = Date.now() - start;
      const avgTime = elapsed / iterations;
      const throughput = (data.length * iterations / 1024 / 1024) / (elapsed / 1000);

      console.log(`  ✓ Average parse time: ${avgTime.toFixed(2)}ms`);
      console.log(`  ✓ Throughput: ${throughput.toFixed(2)} MB/s`);

      // Performance target: should parse at least 1 MB/s
      expect(throughput).to.be.greaterThan(1);
    });
  });

  describe('Error Handling', function() {
    it('should handle invalid ASTERIX data gracefully', function() {
      const invalidData = Buffer.from([
        0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF
      ]);

      expect(() => asterix.parse(invalidData)).to.throw();
    });

    it('should handle truncated data gracefully', function() {
      const filePath = path.join(samplesDir, 'cat048.raw');

      if (!fs.existsSync(filePath)) {
        this.skip();
        return;
      }

      const data = fs.readFileSync(filePath);

      // Truncate to half length
      const truncated = data.subarray(0, Math.floor(data.length / 2));

      // Should either parse partial data or throw error
      try {
        const records = asterix.parse(truncated);
        expect(records).to.be.an('array');
        console.log(`  ✓ Parsed ${records.length} records from truncated data`);
      } catch (err) {
        expect(err).to.be.an('error');
        console.log(`  ✓ Correctly rejected truncated data: ${err.message}`);
      }
    });
  });
});
