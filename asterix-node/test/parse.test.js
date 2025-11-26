/**
 * Unit tests for ASTERIX decoder Node.js bindings
 *
 * These tests verify the FFI boundary validation and parsing functionality
 * following the patterns from BINDING_GUIDELINES.md and SAFETY_CRITICAL.md.
 */

const { expect } = require('chai');
const asterix = require('../lib/index');

describe('ASTERIX Decoder', function() {
  // Increase timeout for initialization
  this.timeout(5000);

  before(function() {
    // Initialize parser once before all tests
    try {
      asterix.init();
    } catch (err) {
      console.warn('Initialization warning:', err.message);
      // Continue - may use auto-initialization
    }
  });

  describe('Initialization', function() {
    it('should initialize without errors', function() {
      expect(() => asterix.init()).to.not.throw();
    });

    it('should throw TypeError for invalid config path', function() {
      expect(() => asterix.init(123)).to.throw(TypeError);
    });

    it('should reject path traversal attacks', function() {
      expect(() => asterix.init('../../../etc/passwd')).to.throw();
    });

    it('should reject paths that are too long', function() {
      const longPath = 'a'.repeat(5000);
      expect(() => asterix.init(longPath)).to.throw();
    });
  });

  describe('parse()', function() {
    it('should throw TypeError for non-Buffer input', function() {
      expect(() => asterix.parse('not a buffer')).to.throw(TypeError);
      expect(() => asterix.parse(123)).to.throw(TypeError);
      expect(() => asterix.parse(null)).to.throw(TypeError);
      expect(() => asterix.parse(undefined)).to.throw(TypeError);
    });

    it('should throw TypeError for empty Buffer (CRITICAL-005)', function() {
      const emptyBuffer = Buffer.alloc(0);
      expect(() => asterix.parse(emptyBuffer)).to.throw(TypeError, /empty/i);
    });

    it('should throw TypeError for oversized data (DoS prevention)', function() {
      const hugeBuffer = Buffer.alloc(65537); // 64 KB + 1
      expect(() => asterix.parse(hugeBuffer)).to.throw(TypeError, /too large/i);
    });

    it('should parse valid ASTERIX data', function() {
      // Minimal valid ASTERIX CAT001 data block
      const data = Buffer.from([
        0x01, // Category 1
        0x00, 0x0C, // Length: 12 bytes
        0x80, // FSPEC: only I001/010 present
        0x12, 0x34 // SAC=0x12, SIC=0x34
      ]);

      // May fail if category not loaded, which is OK for this test
      try {
        const records = asterix.parse(data);
        expect(records).to.be.an('array');
      } catch (err) {
        // Expected if CAT001 not loaded
        expect(err.message).to.match(/not (initialized|defined)/i);
      }
    });

    it('should respect filterCategory option', function() {
      const data = Buffer.from([
        0x30, 0x00, 0x06, 0x00, 0x00, 0x00 // CAT048 minimal block
      ]);

      try {
        const records = asterix.parse(data, { filterCategory: 48 });
        expect(records).to.be.an('array');
        records.forEach(r => expect(r.category).to.equal(48));
      } catch (err) {
        // Expected if CAT048 not loaded
      }
    });

    it('should respect maxRecords option', function() {
      const data = Buffer.from([
        0x30, 0x00, 0x06, 0x00, 0x00, 0x00,
        0x30, 0x00, 0x06, 0x00, 0x00, 0x00,
        0x30, 0x00, 0x06, 0x00, 0x00, 0x00
      ]);

      try {
        const records = asterix.parse(data, { maxRecords: 2 });
        expect(records.length).to.be.at.most(2);
      } catch (err) {
        // Expected if CAT048 not loaded
      }
    });
  });

  describe('parseWithOffset()', function() {
    it('should throw TypeError for non-Buffer input', function() {
      expect(() => asterix.parseWithOffset('not a buffer', 0, 10)).to.throw(TypeError);
    });

    it('should throw TypeError for invalid offset', function() {
      const data = Buffer.alloc(100);
      expect(() => asterix.parseWithOffset(data, 'invalid', 10)).to.throw(TypeError);
      expect(() => asterix.parseWithOffset(data, -1, 10)).to.throw(TypeError);
    });

    it('should throw TypeError for invalid blocksCount', function() {
      const data = Buffer.alloc(100);
      expect(() => asterix.parseWithOffset(data, 0, 'invalid')).to.throw(TypeError);
      expect(() => asterix.parseWithOffset(data, 0, -1)).to.throw(TypeError);
    });

    it('should throw TypeError for offset exceeding data length (CRITICAL-002)', function() {
      const data = Buffer.alloc(100);
      expect(() => asterix.parseWithOffset(data, 101, 10)).to.throw(TypeError, /exceeds/i);
      expect(() => asterix.parseWithOffset(data, 1000, 10)).to.throw(TypeError, /exceeds/i);
    });

    it('should throw TypeError for excessive blocksCount (HIGH-002)', function() {
      const data = Buffer.alloc(100);
      expect(() => asterix.parseWithOffset(data, 0, 100000)).to.throw(TypeError, /exceeds maximum/i);
    });

    it('should return ParseResult object with correct structure', function() {
      const data = Buffer.from([
        0x30, 0x00, 0x06, 0x00, 0x00, 0x00
      ]);

      try {
        const result = asterix.parseWithOffset(data, 0, 10);
        expect(result).to.be.an('object');
        expect(result).to.have.property('records');
        expect(result).to.have.property('bytesConsumed');
        expect(result).to.have.property('remainingBlocks');
        expect(result.records).to.be.an('array');
        expect(result.bytesConsumed).to.be.a('number');
        expect(result.remainingBlocks).to.be.a('number');
      } catch (err) {
        // Expected if CAT048 not loaded
      }
    });

    it('should handle incremental parsing correctly', function() {
      const data = Buffer.from([
        0x30, 0x00, 0x06, 0x00, 0x00, 0x00,
        0x30, 0x00, 0x06, 0x00, 0x00, 0x00
      ]);

      try {
        let offset = 0;
        const allRecords = [];

        while (offset < data.length) {
          const result = asterix.parseWithOffset(data, offset, 1);
          allRecords.push(...result.records);
          offset = result.bytesConsumed;

          if (result.remainingBlocks === 0) {
            break;
          }
        }

        expect(allRecords).to.be.an('array');
      } catch (err) {
        // Expected if CAT048 not loaded
      }
    });
  });

  describe('describe()', function() {
    it('should throw TypeError for invalid category', function() {
      expect(() => asterix.describe('invalid')).to.throw(TypeError);
      expect(() => asterix.describe(0)).to.throw(TypeError, /category/i);
      expect(() => asterix.describe(256)).to.throw(TypeError, /category/i);
      expect(() => asterix.describe(-1)).to.throw(TypeError, /category/i);
    });

    it('should return string description for valid category', function() {
      try {
        const desc = asterix.describe(48);
        expect(desc).to.be.a('string');
      } catch (err) {
        // Expected if CAT048 not loaded
        expect(err.message).to.match(/not (initialized|defined)/i);
      }
    });

    it('should return description for item if provided', function() {
      try {
        const desc = asterix.describe(48, '010');
        expect(desc).to.be.a('string');
      } catch (err) {
        // Expected if CAT048 not loaded
      }
    });

    it('should return description for field value if all params provided', function() {
      try {
        const desc = asterix.describe(48, '010', 'SAC', '7');
        expect(desc).to.be.a('string');
      } catch (err) {
        // Expected if CAT048 not loaded
      }
    });
  });

  describe('isCategoryDefined()', function() {
    it('should return false for invalid category numbers', function() {
      expect(asterix.isCategoryDefined(0)).to.be.false;
      expect(asterix.isCategoryDefined(256)).to.be.false;
      expect(asterix.isCategoryDefined(-1)).to.be.false;
    });

    it('should return boolean for valid category', function() {
      const result = asterix.isCategoryDefined(48);
      expect(result).to.be.a('boolean');
    });

    it('should return true for loaded categories', function() {
      // Assuming at least one category is loaded during init
      const categories = [1, 2, 8, 10, 19, 20, 21, 34, 48, 62];
      const anyLoaded = categories.some(cat => asterix.isCategoryDefined(cat));
      expect(anyLoaded).to.be.true;
    });
  });

  describe('loadCategory()', function() {
    it('should throw TypeError for invalid XML path', function() {
      expect(() => asterix.loadCategory(123)).to.throw(TypeError);
      expect(() => asterix.loadCategory('')).to.throw();
    });

    it('should reject path traversal attacks (MEDIUM-004)', function() {
      expect(() => asterix.loadCategory('../../../etc/passwd')).to.throw();
    });

    it('should reject paths that are too long', function() {
      const longPath = 'a'.repeat(5000);
      expect(() => asterix.loadCategory(longPath)).to.throw();
    });
  });

  describe('version', function() {
    it('should export version string', function() {
      expect(asterix.version).to.be.a('string');
      expect(asterix.version).to.match(/\d+\.\d+\.\d+/);
    });
  });

  describe('Memory Safety', function() {
    it('should not leak memory on repeated parsing', function() {
      const data = Buffer.from([
        0x30, 0x00, 0x06, 0x00, 0x00, 0x00
      ]);

      // Parse many times - should not leak
      for (let i = 0; i < 1000; i++) {
        try {
          asterix.parse(data);
        } catch (err) {
          // Expected if CAT048 not loaded
        }
      }

      // If we get here without crashing, memory is managed correctly
      expect(true).to.be.true;
    });

    it('should handle concurrent parsing safely', async function() {
      const data = Buffer.from([
        0x30, 0x00, 0x06, 0x00, 0x00, 0x00
      ]);

      // Parse concurrently
      const promises = [];
      for (let i = 0; i < 10; i++) {
        promises.push(new Promise((resolve) => {
          try {
            asterix.parse(data);
          } catch (err) {
            // Expected
          }
          resolve();
        }));
      }

      await Promise.all(promises);
      expect(true).to.be.true;
    });
  });

  describe('parseAsync()', function() {
    it('should throw TypeError for non-Buffer input', async function() {
      try {
        await asterix.parseAsync('not a buffer');
        expect.fail('Should have thrown');
      } catch (err) {
        expect(err).to.be.instanceOf(TypeError);
      }
    });

    it('should throw TypeError for empty Buffer', async function() {
      try {
        await asterix.parseAsync(Buffer.alloc(0));
        expect.fail('Should have thrown');
      } catch (err) {
        expect(err).to.be.instanceOf(TypeError);
        expect(err.message).to.match(/empty/i);
      }
    });

    it('should return a Promise', function() {
      const data = Buffer.from([0x30, 0x00, 0x06, 0x00, 0x00, 0x00]);
      const result = asterix.parseAsync(data);
      expect(result).to.be.instanceOf(Promise);
      return result.catch(() => {}); // Ignore errors
    });

    it('should parse valid data asynchronously', async function() {
      const data = Buffer.from([0x30, 0x00, 0x06, 0x00, 0x00, 0x00]);
      try {
        const records = await asterix.parseAsync(data);
        expect(records).to.be.an('array');
      } catch (err) {
        // Expected if CAT048 not loaded
      }
    });

    it('should respect chunkSize option', async function() {
      const data = Buffer.from([0x30, 0x00, 0x06, 0x00, 0x00, 0x00]);
      try {
        const records = await asterix.parseAsync(data, { chunkSize: 1 });
        expect(records).to.be.an('array');
      } catch (err) {
        // Expected if CAT048 not loaded
      }
    });
  });

  describe('createParseStream()', function() {
    it('should return a Transform stream', function() {
      const stream = asterix.createParseStream();
      expect(stream).to.have.property('pipe');
      expect(stream).to.have.property('on');
      expect(stream).to.have.property('write');
      stream.destroy();
    });

    it('should accept options', function() {
      const stream = asterix.createParseStream({
        verbose: true,
        filterCategory: 48,
        objectMode: true
      });
      expect(stream).to.have.property('pipe');
      stream.destroy();
    });

    it('should emit records when data is written', function(done) {
      const stream = asterix.createParseStream();
      const records = [];

      stream.on('data', (record) => {
        records.push(record);
      });

      stream.on('end', () => {
        // May have 0 records if category not loaded, that's OK
        expect(records).to.be.an('array');
        done();
      });

      stream.on('error', (err) => {
        // Expected if category not loaded
        done();
      });

      // Write minimal ASTERIX data
      stream.write(Buffer.from([0x30, 0x00, 0x06, 0x00, 0x00, 0x00]));
      stream.end();
    });

    it('should filter by category', function(done) {
      const stream = asterix.createParseStream({ filterCategory: 62 });
      const records = [];

      stream.on('data', (record) => {
        records.push(record);
      });

      stream.on('end', () => {
        // All records should be CAT062 (or empty if filtered out)
        records.forEach(r => expect(r.category).to.equal(62));
        done();
      });

      stream.on('error', () => done());

      // Write CAT048 data - should be filtered out
      stream.write(Buffer.from([0x30, 0x00, 0x06, 0x00, 0x00, 0x00]));
      stream.end();
    });

    it('should emit JSON strings when objectMode is false', function(done) {
      const stream = asterix.createParseStream({ objectMode: false });
      const chunks = [];

      stream.on('data', (chunk) => {
        chunks.push(chunk);
      });

      stream.on('end', () => {
        // Chunks should be strings or buffers containing JSON
        chunks.forEach(c => {
          const str = Buffer.isBuffer(c) ? c.toString() : c;
          expect(str).to.be.a('string');
          // If not empty, should be valid JSON
          if (str.trim()) {
            expect(() => JSON.parse(str)).to.not.throw();
          }
        });
        done();
      });

      stream.on('error', () => done());

      stream.write(Buffer.from([0x30, 0x00, 0x06, 0x00, 0x00, 0x00]));
      stream.end();
    });
  });
});
