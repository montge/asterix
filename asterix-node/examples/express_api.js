/**
 * Express.js REST API Example
 *
 * This example demonstrates building a REST API for ASTERIX parsing using Express.js.
 * The API provides endpoints for parsing files, getting category descriptions,
 * and querying metadata.
 */

const express = require('express');
const asterix = require('asterix-decoder');
const multer = require('multer');
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

// Setup Express app
const app = express();
const upload = multer({
    limits: {
        fileSize: 64 * 1024 * 1024 // 64 MB max file size
    }
});

// Middleware
app.use(express.json());

// CORS headers for API access
app.use((req, res, next) => {
    res.header('Access-Control-Allow-Origin', '*');
    res.header('Access-Control-Allow-Methods', 'GET, POST, OPTIONS');
    res.header('Access-Control-Allow-Headers', 'Content-Type');
    next();
});

// Request logging
app.use((req, res, next) => {
    console.log(`${new Date().toISOString()} ${req.method} ${req.path}`);
    next();
});

/**
 * GET /
 * API documentation
 */
app.get('/', (req, res) => {
    res.json({
        name: 'ASTERIX Decoder API',
        version: asterix.version,
        endpoints: {
            'GET /': 'This documentation',
            'GET /health': 'Health check',
            'GET /categories': 'List available ASTERIX categories',
            'GET /categories/:category': 'Get category description',
            'GET /categories/:category/describe': 'Get field descriptions',
            'POST /parse': 'Parse ASTERIX data from uploaded file',
            'POST /parse-raw': 'Parse ASTERIX data from JSON body',
            'GET /stats': 'API statistics'
        },
        examples: {
            parse: 'POST /parse with multipart/form-data file upload',
            describe: 'GET /categories/48/describe?item=010&field=SAC&value=7',
            filter: 'POST /parse?category=62&limit=100'
        }
    });
});

/**
 * GET /health
 * Health check endpoint
 */
app.get('/health', (req, res) => {
    res.json({
        status: 'ok',
        timestamp: new Date().toISOString(),
        version: asterix.version,
        uptime: process.uptime()
    });
});

/**
 * GET /categories
 * List available ASTERIX categories
 */
app.get('/categories', (req, res) => {
    const categories = [];
    const allCategories = [1, 2, 4, 8, 10, 11, 15, 19, 20, 21, 23, 25, 30, 31, 32, 34, 48, 62, 63, 65, 205, 240, 247, 252];

    for (const cat of allCategories) {
        if (asterix.isCategoryDefined(cat)) {
            categories.push({
                category: cat,
                description: asterix.describe(cat)
            });
        }
    }

    res.json({
        total: categories.length,
        categories: categories
    });
});

/**
 * GET /categories/:category
 * Get description for a specific category
 */
app.get('/categories/:category', (req, res) => {
    try {
        const category = parseInt(req.params.category);

        if (isNaN(category) || category < 1 || category > 255) {
            return res.status(400).json({
                error: 'Invalid category number (must be 1-255)'
            });
        }

        if (!asterix.isCategoryDefined(category)) {
            return res.status(404).json({
                error: `Category ${category} not defined`
            });
        }

        res.json({
            category: category,
            description: asterix.describe(category),
            defined: true
        });
    } catch (err) {
        res.status(500).json({
            error: err.message
        });
    }
});

/**
 * GET /categories/:category/describe
 * Get human-readable description for ASTERIX field values
 */
app.get('/categories/:category/describe', (req, res) => {
    try {
        const category = parseInt(req.params.category);
        const { item, field, value } = req.query;

        if (isNaN(category) || category < 1 || category > 255) {
            return res.status(400).json({
                error: 'Invalid category number'
            });
        }

        const description = asterix.describe(category, item, field, value);

        res.json({
            category: category,
            item: item || null,
            field: field || null,
            value: value || null,
            description: description
        });
    } catch (err) {
        res.status(400).json({
            error: err.message
        });
    }
});

/**
 * POST /parse
 * Parse ASTERIX data from uploaded file
 */
app.post('/parse', upload.single('file'), (req, res) => {
    try {
        if (!req.file) {
            return res.status(400).json({
                error: 'No file uploaded. Send file as multipart/form-data with field name "file"'
            });
        }

        // Parse options from query parameters
        const options = {
            verbose: req.query.verbose === 'true',
            filterCategory: req.query.category ? parseInt(req.query.category) : undefined,
            maxRecords: req.query.limit ? parseInt(req.query.limit) : undefined
        };

        // Validate options
        if (options.filterCategory && (isNaN(options.filterCategory) || options.filterCategory < 1 || options.filterCategory > 255)) {
            return res.status(400).json({
                error: 'Invalid category filter (must be 1-255)'
            });
        }

        if (options.maxRecords && (isNaN(options.maxRecords) || options.maxRecords < 1)) {
            return res.status(400).json({
                error: 'Invalid limit (must be > 0)'
            });
        }

        // Parse ASTERIX data
        const startTime = Date.now();
        const records = asterix.parse(req.file.buffer, options);
        const parseTime = Date.now() - startTime;

        // Count by category
        const categoryCount = {};
        for (const record of records) {
            categoryCount[record.category] = (categoryCount[record.category] || 0) + 1;
        }

        res.json({
            success: true,
            metadata: {
                filename: req.file.originalname,
                size: req.file.size,
                parse_time_ms: parseTime,
                total_records: records.length,
                categories: categoryCount
            },
            records: records
        });
    } catch (err) {
        res.status(400).json({
            success: false,
            error: err.message
        });
    }
});

/**
 * POST /parse-raw
 * Parse ASTERIX data from JSON body (for testing)
 */
app.post('/parse-raw', (req, res) => {
    try {
        if (!req.body.data) {
            return res.status(400).json({
                error: 'Missing "data" field in JSON body (must be array of bytes)'
            });
        }

        // Convert array of bytes to Buffer
        const buffer = Buffer.from(req.body.data);

        const options = {
            verbose: req.body.verbose || false,
            filterCategory: req.body.filterCategory,
            maxRecords: req.body.maxRecords
        };

        const startTime = Date.now();
        const records = asterix.parse(buffer, options);
        const parseTime = Date.now() - startTime;

        res.json({
            success: true,
            metadata: {
                size: buffer.length,
                parse_time_ms: parseTime,
                total_records: records.length
            },
            records: records
        });
    } catch (err) {
        res.status(400).json({
            success: false,
            error: err.message
        });
    }
});

/**
 * GET /stats
 * API usage statistics
 */
let requestCount = 0;
let parseCount = 0;
let totalRecordsParsed = 0;

app.use((req, res, next) => {
    requestCount++;
    next();
});

app.get('/stats', (req, res) => {
    res.json({
        requests: requestCount,
        parses: parseCount,
        total_records_parsed: totalRecordsParsed,
        uptime_seconds: process.uptime(),
        memory: process.memoryUsage()
    });
});

// Error handling middleware
app.use((err, req, res, next) => {
    console.error('Error:', err.message);
    res.status(500).json({
        error: 'Internal server error',
        message: err.message
    });
});

// Start server
const PORT = process.env.PORT || 3000;
const server = app.listen(PORT, () => {
    console.log(`\nASTERIX Decoder API running on port ${PORT}`);
    console.log(`\nAPI endpoints:`);
    console.log(`  GET  http://localhost:${PORT}/               - API documentation`);
    console.log(`  GET  http://localhost:${PORT}/health         - Health check`);
    console.log(`  GET  http://localhost:${PORT}/categories     - List categories`);
    console.log(`  POST http://localhost:${PORT}/parse          - Parse file upload`);
    console.log(`\nExample usage:`);
    console.log(`  curl http://localhost:${PORT}/categories`);
    console.log(`  curl -F "file=@sample.pcap" http://localhost:${PORT}/parse`);
    console.log(`  curl "http://localhost:${PORT}/categories/48/describe?item=010&field=SAC&value=7"`);
    console.log(`\nPress Ctrl+C to stop\n`);
});

// Graceful shutdown
process.on('SIGTERM', () => {
    console.log('\nSIGTERM received, shutting down gracefully...');
    server.close(() => {
        console.log('Server closed');
        process.exit(0);
    });
});

process.on('SIGINT', () => {
    console.log('\n\nSIGINT received, shutting down gracefully...');
    server.close(() => {
        console.log('Server closed');
        process.exit(0);
    });
});
