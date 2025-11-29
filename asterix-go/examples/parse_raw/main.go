// Example: Parse raw ASTERIX bytes
package main

import (
	"fmt"
	"log"
	"os"

	asterix "github.com/montge/asterix/asterix-go"
)

func main() {
	// Initialize the parser
	if err := asterix.Init(""); err != nil {
		log.Fatalf("Failed to initialize: %v", err)
	}

	fmt.Printf("ASTERIX library version: %s\n", asterix.Version())

	// Sample ASTERIX CAT048 data
	data := []byte{
		0x30,       // Category 48
		0x00, 0x08, // Length: 8 bytes
		0x80,                   // FSPEC
		0x00, 0x01, 0x00, 0x02, // Data
	}

	// Parse the data
	records, err := asterix.Parse(data)
	if err != nil {
		log.Printf("Parse error (expected without XML config): %v", err)
		os.Exit(0)
	}

	// Print results
	fmt.Printf("Parsed %d records:\n", len(records))
	for i, rec := range records {
		fmt.Printf("  [%d] Category: %d, Length: %d\n", i, rec.Category, rec.Length)
		fmt.Printf("       JSON: %s\n", rec.JSON)

		// Access parsed items
		items, err := rec.Items()
		if err == nil {
			fmt.Printf("       Items: %v\n", items)
		}
	}
}
