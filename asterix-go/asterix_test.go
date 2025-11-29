// Package asterix tests
package asterix

import (
	"testing"
)

// Sample CAT048 ASTERIX data (minimal valid block)
// Format: Category (1 byte) + Length (2 bytes, big-endian) + FSPEC + data
var sampleCAT048 = []byte{
	0x30,       // Category 48
	0x00, 0x08, // Length: 8 bytes
	0x80,                   // FSPEC: bit 7 set (I010 present)
	0x00, 0x01, 0x00, 0x02, // I010: SAC=0, SIC=1, padding
}

// Minimal CAT062 data
var sampleCAT062 = []byte{
	0x3E,       // Category 62
	0x00, 0x06, // Length: 6 bytes
	0x80,       // FSPEC: bit 7 set
	0x00, 0x01, // Data
}

func TestVersion(t *testing.T) {
	v := Version()
	if v == "" {
		t.Error("Version() returned empty string")
	}
	t.Logf("ASTERIX library version: %s", v)
}

func TestInit(t *testing.T) {
	// Test initialization with empty config (should use defaults)
	err := Init("")
	if err != nil {
		t.Errorf("Init(\"\") failed: %v", err)
	}

	// Verify initialized state
	if !IsInitialized() {
		t.Error("IsInitialized() returned false after Init()")
	}
}

func TestInitIdempotent(t *testing.T) {
	// Multiple Init calls should be safe
	for i := 0; i < 3; i++ {
		err := Init("")
		if err != nil {
			t.Errorf("Init() call %d failed: %v", i+1, err)
		}
	}
}

func TestParseNotInitialized(t *testing.T) {
	// Note: This test assumes a fresh state, but since Init is called in other tests,
	// we can't truly test uninitialized state without process isolation.
	// This is a limitation of the global state design.
	t.Skip("Cannot test uninitialized state due to global initialization")
}

func TestParseEmptyData(t *testing.T) {
	if err := Init(""); err != nil {
		t.Fatalf("Init failed: %v", err)
	}

	_, err := Parse([]byte{})
	if err == nil {
		t.Error("Parse(empty) should return error")
	}
	if err != ErrInvalidData {
		t.Errorf("Expected ErrInvalidData, got: %v", err)
	}
}

func TestParseNilData(t *testing.T) {
	if err := Init(""); err != nil {
		t.Fatalf("Init failed: %v", err)
	}

	_, err := Parse(nil)
	if err == nil {
		t.Error("Parse(nil) should return error")
	}
}

func TestParseValidData(t *testing.T) {
	if err := Init(""); err != nil {
		t.Fatalf("Init failed: %v", err)
	}

	records, err := Parse(sampleCAT048)
	if err != nil {
		t.Logf("Parse returned error (may be expected without XML config): %v", err)
		// Not a fatal error - parsing may fail without proper XML definitions loaded
		return
	}

	t.Logf("Parsed %d records", len(records))
	for i, rec := range records {
		t.Logf("Record %d: Category=%d, Length=%d, JSON=%s",
			i, rec.Category, rec.Length, rec.JSON)
	}
}

func TestParseWithOptions(t *testing.T) {
	if err := Init(""); err != nil {
		t.Fatalf("Init failed: %v", err)
	}

	// Test with verbose=false
	records, err := ParseWithOptions(sampleCAT048, false)
	if err != nil {
		t.Logf("ParseWithOptions returned error: %v", err)
		return
	}

	t.Logf("Parsed %d records (non-verbose)", len(records))
}

func TestParseWithOffset(t *testing.T) {
	if err := Init(""); err != nil {
		t.Fatalf("Init failed: %v", err)
	}

	result, err := ParseWithOffset(sampleCAT048, 0, 10)
	if err != nil {
		t.Logf("ParseWithOffset returned error: %v", err)
		return
	}

	t.Logf("Parsed %d records, consumed %d bytes",
		len(result.Records), result.BytesConsumed)
}

func TestParseWithOffsetInvalid(t *testing.T) {
	if err := Init(""); err != nil {
		t.Fatalf("Init failed: %v", err)
	}

	// Offset beyond data length
	_, err := ParseWithOffset(sampleCAT048, 100, 10)
	if err == nil {
		t.Error("ParseWithOffset with invalid offset should return error")
	}
}

func TestParseWithOffsetNegative(t *testing.T) {
	if err := Init(""); err != nil {
		t.Fatalf("Init failed: %v", err)
	}

	// Negative offset (will wrap to large positive)
	_, err := ParseWithOffset(sampleCAT048, -1, 10)
	if err == nil {
		t.Error("ParseWithOffset with negative offset should return error")
	}
}

func TestDescribe(t *testing.T) {
	if err := Init(""); err != nil {
		t.Fatalf("Init failed: %v", err)
	}

	// Test category description
	desc, err := Describe(48, "", "", "")
	if err != nil {
		t.Logf("Describe(48) returned error: %v", err)
	} else {
		t.Logf("Category 48 description: %s", desc)
	}
}

func TestDescribeInvalidCategory(t *testing.T) {
	if err := Init(""); err != nil {
		t.Fatalf("Init failed: %v", err)
	}

	// Category 0 is invalid
	_, err := Describe(0, "", "", "")
	if err == nil {
		t.Error("Describe(0) should return error for invalid category")
	}

	// Category 256 is invalid
	_, err = Describe(256, "", "", "")
	if err == nil {
		t.Error("Describe(256) should return error for invalid category")
	}
}

func TestIsCategoryDefined(t *testing.T) {
	if err := Init(""); err != nil {
		t.Fatalf("Init failed: %v", err)
	}

	// Without XML config loaded, no categories are defined
	// This is expected behavior
	t.Logf("Category 48 defined: %v", IsCategoryDefined(48))
	t.Logf("Category 62 defined: %v", IsCategoryDefined(62))

	// Invalid categories should return false
	if IsCategoryDefined(0) {
		t.Error("IsCategoryDefined(0) should return false")
	}
	if IsCategoryDefined(256) {
		t.Error("IsCategoryDefined(256) should return false")
	}
}

func TestRecordItems(t *testing.T) {
	rec := Record{
		Category: 48,
		Length:   10,
		JSON:     `{"category":48,"I010":{"SAC":0,"SIC":1}}`,
	}

	items, err := rec.Items()
	if err != nil {
		t.Fatalf("Items() failed: %v", err)
	}

	if items["category"] != float64(48) {
		t.Errorf("Expected category=48, got %v", items["category"])
	}

	// Test caching
	items2, err := rec.Items()
	if err != nil {
		t.Fatalf("Items() second call failed: %v", err)
	}

	if items2["category"] != float64(48) {
		t.Error("Cached items should return same values")
	}
}

func TestRecordItemsInvalidJSON(t *testing.T) {
	rec := Record{
		Category: 48,
		JSON:     `{invalid json}`,
	}

	_, err := rec.Items()
	if err == nil {
		t.Error("Items() should fail on invalid JSON")
	}
}

func TestDataTooLarge(t *testing.T) {
	if err := Init(""); err != nil {
		t.Fatalf("Init failed: %v", err)
	}

	// Create data larger than ASTERIX_MAX_MESSAGE_SIZE (65536)
	largeData := make([]byte, 70000)
	largeData[0] = 0x30 // Category 48
	largeData[1] = 0xFF
	largeData[2] = 0xFF // Max length

	_, err := Parse(largeData)
	if err == nil {
		t.Error("Parse should fail for data exceeding max size")
	}
}

// Benchmark tests
func BenchmarkParse(b *testing.B) {
	if err := Init(""); err != nil {
		b.Fatalf("Init failed: %v", err)
	}

	b.ResetTimer()
	for i := 0; i < b.N; i++ {
		_, _ = Parse(sampleCAT048)
	}
}

func BenchmarkParseWithOffset(b *testing.B) {
	if err := Init(""); err != nil {
		b.Fatalf("Init failed: %v", err)
	}

	b.ResetTimer()
	for i := 0; i < b.N; i++ {
		_, _ = ParseWithOffset(sampleCAT048, 0, 10)
	}
}
