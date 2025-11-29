// Package asterix provides Go bindings for the ASTERIX decoder.
//
// ASTERIX (All Purpose STructured EUROCONTROL SuRveillance Information EXchange)
// is a binary data format used in Air Traffic Management for surveillance data.
//
// This package wraps the C++ ASTERIX parser via cgo, providing a safe and
// idiomatic Go API for parsing ASTERIX data.
//
// Basic usage:
//
//	import "github.com/montge/asterix/asterix-go"
//
//	func main() {
//	    // Initialize with default configuration
//	    if err := asterix.Init(""); err != nil {
//	        log.Fatal(err)
//	    }
//
//	    // Parse ASTERIX data
//	    data, _ := os.ReadFile("sample.asterix")
//	    records, err := asterix.Parse(data)
//	    if err != nil {
//	        log.Fatal(err)
//	    }
//
//	    for _, rec := range records {
//	        fmt.Printf("Category %d: %s\n", rec.Category, rec.JSON)
//	    }
//	}
package asterix

/*
#cgo CFLAGS: -I${SRCDIR} -I${SRCDIR}/../src/asterix -I${SRCDIR}/../src/engine -I${SRCDIR}/../install/include
#cgo LDFLAGS: -L${SRCDIR}/../install/lib -lasterix -lexpat -lstdc++

#include "asterix.h"
#include <stdlib.h>
*/
import "C"

import (
	"encoding/json"
	"errors"
	"fmt"
	"sync"
	"time"
	"unsafe"
)

// Error codes
var (
	ErrNotInitialized = errors.New("asterix: parser not initialized")
	ErrInvalidData    = errors.New("asterix: invalid data")
	ErrParseFailed    = errors.New("asterix: parse failed")
	ErrMemory         = errors.New("asterix: memory allocation failed")
)

// Record represents a parsed ASTERIX record.
type Record struct {
	// Category is the ASTERIX category (1-255)
	Category uint8 `json:"category"`

	// Length is the total length of the record in bytes
	Length uint16 `json:"len"`

	// Timestamp is when the record was parsed
	Timestamp time.Time `json:"ts"`

	// CRC is the checksum of the record
	CRC uint32 `json:"crc"`

	// JSON is the full parsed record as JSON string
	JSON string `json:"data"`

	// Items contains the parsed data items (lazy-loaded from JSON)
	items map[string]interface{}
}

// Items returns the parsed data items as a map.
// The result is cached after first call.
func (r *Record) Items() (map[string]interface{}, error) {
	if r.items != nil {
		return r.items, nil
	}

	var data map[string]interface{}
	if err := json.Unmarshal([]byte(r.JSON), &data); err != nil {
		return nil, fmt.Errorf("asterix: failed to parse record JSON: %w", err)
	}

	r.items = data
	return r.items, nil
}

// ParseResult contains the result of a parse operation.
type ParseResult struct {
	// Records contains the parsed ASTERIX records
	Records []Record

	// BytesConsumed is the number of bytes consumed from input
	BytesConsumed int

	// RemainingBlocks is the number of blocks not yet parsed
	RemainingBlocks int
}

var (
	initOnce    sync.Once
	initialized bool
	initMu      sync.Mutex
)

// Init initializes the ASTERIX parser with the given configuration path.
// If configPath is empty, uses the default configuration.
// This function is thread-safe and can be called multiple times.
func Init(configPath string) error {
	initMu.Lock()
	defer initMu.Unlock()

	if initialized {
		return nil
	}

	var cPath *C.char
	if configPath != "" {
		cPath = C.CString(configPath)
		defer C.free(unsafe.Pointer(cPath))
	}

	ret := C.asterix_init(cPath)
	if ret != C.ASTERIX_OK {
		errMsg := C.asterix_get_last_error()
		if errMsg != nil {
			return fmt.Errorf("asterix: init failed: %s", C.GoString(errMsg))
		}
		return fmt.Errorf("asterix: init failed with code %d", ret)
	}

	initialized = true
	return nil
}

// IsInitialized returns true if the parser has been initialized.
func IsInitialized() bool {
	initMu.Lock()
	defer initMu.Unlock()
	return initialized
}

// Parse parses ASTERIX data and returns the parsed records.
// The data should contain raw ASTERIX binary data.
func Parse(data []byte) ([]Record, error) {
	return ParseWithOptions(data, true)
}

// ParseWithOptions parses ASTERIX data with the given options.
// If verbose is true, descriptions are included in the output.
func ParseWithOptions(data []byte, verbose bool) ([]Record, error) {
	if !IsInitialized() {
		return nil, ErrNotInitialized
	}

	if len(data) == 0 {
		return nil, ErrInvalidData
	}

	if len(data) > C.ASTERIX_MAX_MESSAGE_SIZE {
		return nil, fmt.Errorf("asterix: data too large (%d bytes, max %d)",
			len(data), C.ASTERIX_MAX_MESSAGE_SIZE)
	}

	verboseInt := 0
	if verbose {
		verboseInt = 1
	}

	result := C.asterix_parse(
		(*C.uint8_t)(unsafe.Pointer(&data[0])),
		C.size_t(len(data)),
		C.int(verboseInt),
	)
	if result == nil {
		return nil, ErrMemory
	}
	defer C.asterix_free_result(result)

	if result.error_code != C.ASTERIX_OK {
		if result.error_message != nil {
			return nil, fmt.Errorf("asterix: %s", C.GoString(result.error_message))
		}
		return nil, ErrParseFailed
	}

	return convertRecords(result), nil
}

// ParseWithOffset parses ASTERIX data starting at the given offset.
// This is useful for incremental parsing of large data streams.
func ParseWithOffset(data []byte, offset, maxBlocks int) (*ParseResult, error) {
	if !IsInitialized() {
		return nil, ErrNotInitialized
	}

	if len(data) == 0 {
		return nil, ErrInvalidData
	}

	if offset < 0 || offset >= len(data) {
		return nil, fmt.Errorf("asterix: offset %d out of range [0, %d)", offset, len(data))
	}

	if maxBlocks < 0 {
		maxBlocks = 0
	}

	result := C.asterix_parse_with_offset(
		(*C.uint8_t)(unsafe.Pointer(&data[0])),
		C.size_t(len(data)),
		C.size_t(offset),
		C.size_t(maxBlocks),
		C.int(1), // verbose
	)
	if result == nil {
		return nil, ErrMemory
	}
	defer C.asterix_free_result(result)

	if result.error_code != C.ASTERIX_OK {
		if result.error_message != nil {
			return nil, fmt.Errorf("asterix: %s", C.GoString(result.error_message))
		}
		return nil, ErrParseFailed
	}

	return &ParseResult{
		Records:       convertRecords(result),
		BytesConsumed: int(result.bytes_consumed),
	}, nil
}

// Describe returns a description for the given ASTERIX category, item, field, or value.
func Describe(category int, item, field, value string) (string, error) {
	if !IsInitialized() {
		return "", ErrNotInitialized
	}

	if category < 1 || category > 255 {
		return "", fmt.Errorf("asterix: invalid category %d (must be 1-255)", category)
	}

	var cItem, cField, cValue *C.char
	if item != "" {
		cItem = C.CString(item)
		defer C.free(unsafe.Pointer(cItem))
	}
	if field != "" {
		cField = C.CString(field)
		defer C.free(unsafe.Pointer(cField))
	}
	if value != "" {
		cValue = C.CString(value)
		defer C.free(unsafe.Pointer(cValue))
	}

	desc := C.asterix_describe(C.int(category), cItem, cField, cValue)
	if desc == nil {
		return "", nil
	}
	defer C.asterix_free_string(desc)

	return C.GoString(desc), nil
}

// IsCategoryDefined returns true if the given category is defined.
func IsCategoryDefined(category int) bool {
	if !IsInitialized() {
		return false
	}

	if category < 1 || category > 255 {
		return false
	}

	return C.asterix_is_category_defined(C.int(category)) != 0
}

// Version returns the library version.
func Version() string {
	return C.GoString(C.asterix_version())
}

// convertRecords converts C records to Go records
func convertRecords(result *C.AsterixParseResult) []Record {
	if result.count == 0 {
		return nil
	}

	records := make([]Record, result.count)
	cRecords := (*[1 << 20]C.AsterixRecord)(unsafe.Pointer(result.records))[:result.count:result.count]

	for i, cr := range cRecords {
		records[i] = Record{
			Category:  uint8(cr.category),
			Length:    uint16(cr.length),
			Timestamp: time.UnixMicro(int64(cr.timestamp_us)),
			CRC:       uint32(cr.crc),
		}
		if cr.json_data != nil {
			records[i].JSON = C.GoString(cr.json_data)
		}
	}

	return records
}
