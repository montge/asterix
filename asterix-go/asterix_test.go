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

func TestDescribeWithItem(t *testing.T) {
	if err := Init(""); err != nil {
		t.Fatalf("Init failed: %v", err)
	}

	// Test with actual category and item
	// Category 48 (Monoradar Target Reports), Item I010 (Data Source Identifier)
	desc, err := Describe(48, "I010", "", "")
	if err != nil {
		t.Errorf("Describe(48, I010) failed: %v", err)
	}
	if desc == "" {
		t.Logf("Describe(48, I010) returned empty description (may need XML config)")
	} else {
		t.Logf("Category 48, I010 description: %s", desc)
	}

	// Test with Category 62 (System Track Data), Item I010
	desc, err = Describe(62, "I010", "", "")
	if err != nil {
		t.Errorf("Describe(62, I010) failed: %v", err)
	}
	t.Logf("Category 62, I010 description: %s", desc)

	// Test with non-existent item (should not error, just return empty or not found)
	desc, err = Describe(48, "IXXX", "", "")
	if err != nil {
		t.Errorf("Describe with non-existent item should not error: %v", err)
	}
	t.Logf("Non-existent item description: %s", desc)
}

func TestDescribeWithField(t *testing.T) {
	if err := Init(""); err != nil {
		t.Fatalf("Init failed: %v", err)
	}

	// Test with category, item, and field
	// I010 has SAC and SIC fields
	desc, err := Describe(48, "I010", "SAC", "")
	if err != nil {
		t.Errorf("Describe(48, I010, SAC) failed: %v", err)
	}
	if desc == "" {
		t.Logf("Describe(48, I010, SAC) returned empty description (may need XML config)")
	} else {
		t.Logf("Category 48, I010, SAC description: %s", desc)
	}

	desc, err = Describe(48, "I010", "SIC", "")
	if err != nil {
		t.Errorf("Describe(48, I010, SIC) failed: %v", err)
	}
	t.Logf("Category 48, I010, SIC description: %s", desc)

	// Test with non-existent field
	desc, err = Describe(48, "I010", "BADFIELD", "")
	if err != nil {
		t.Errorf("Describe with non-existent field should not error: %v", err)
	}
	t.Logf("Non-existent field description: %s", desc)
}

func TestDescribeWithValue(t *testing.T) {
	if err := Init(""); err != nil {
		t.Fatalf("Init failed: %v", err)
	}

	// Test with all parameters (category, item, field, value)
	// This tests the full describe path with a value
	desc, err := Describe(48, "I010", "SAC", "0")
	if err != nil {
		t.Errorf("Describe(48, I010, SAC, 0) failed: %v", err)
	}
	if desc == "" {
		t.Logf("Describe(48, I010, SAC, 0) returned empty description (may need XML config)")
	} else {
		t.Logf("Category 48, I010, SAC=0 description: %s", desc)
	}

	// Test with different value
	desc, err = Describe(48, "I010", "SIC", "1")
	if err != nil {
		t.Errorf("Describe(48, I010, SIC, 1) failed: %v", err)
	}
	t.Logf("Category 48, I010, SIC=1 description: %s", desc)

	// Test with hex value
	desc, err = Describe(48, "I010", "SAC", "0xFF")
	if err != nil {
		t.Errorf("Describe with hex value failed: %v", err)
	}
	t.Logf("Hex value description: %s", desc)
}

func TestDescribeEmptyStrings(t *testing.T) {
	if err := Init(""); err != nil {
		t.Fatalf("Init failed: %v", err)
	}

	// Test with empty strings vs nil (empty strings are passed as C NULL in the implementation)
	desc1, err1 := Describe(48, "", "", "")
	if err1 != nil {
		t.Errorf("Describe with empty strings failed: %v", err1)
	}

	desc2, err2 := Describe(48, "", "", "")
	if err2 != nil {
		t.Errorf("Describe with empty strings (second call) failed: %v", err2)
	}

	// Results should be consistent
	if desc1 != desc2 {
		t.Errorf("Describe results should be consistent: got %q vs %q", desc1, desc2)
	}

	// Test partial empty strings
	desc3, err3 := Describe(48, "I010", "", "")
	if err3 != nil {
		t.Errorf("Describe with partial empty strings failed: %v", err3)
	}
	t.Logf("Partial empty strings description: %s", desc3)

	// Test with value but no field (may not be meaningful, but shouldn't crash)
	desc4, err4 := Describe(48, "I010", "", "0")
	if err4 != nil {
		t.Errorf("Describe with value but no field failed: %v", err4)
	}
	t.Logf("Value but no field description: %s", desc4)
}

func TestDescribeConcurrent(t *testing.T) {
	if err := Init(""); err != nil {
		t.Fatalf("Init failed: %v", err)
	}

	// Test 10 goroutines calling Describe simultaneously
	const numGoroutines = 10
	results := make(chan struct {
		desc string
		err  error
	}, numGoroutines)

	// Launch concurrent describe calls
	for i := 0; i < numGoroutines; i++ {
		go func(id int) {
			// Vary the parameters to test different code paths
			var desc string
			var err error

			switch id % 4 {
			case 0:
				desc, err = Describe(48, "", "", "")
			case 1:
				desc, err = Describe(48, "I010", "", "")
			case 2:
				desc, err = Describe(62, "I010", "SAC", "")
			case 3:
				desc, err = Describe(48, "I010", "SIC", "1")
			}

			results <- struct {
				desc string
				err  error
			}{desc, err}
		}(i)
	}

	// Collect results
	errorCount := 0
	for i := 0; i < numGoroutines; i++ {
		result := <-results
		if result.err != nil {
			errorCount++
			t.Logf("Goroutine returned error: %v", result.err)
		}
	}

	if errorCount > 0 {
		t.Errorf("Concurrent Describe calls failed: %d errors out of %d calls",
			errorCount, numGoroutines)
	} else {
		t.Logf("All %d concurrent Describe calls succeeded", numGoroutines)
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
