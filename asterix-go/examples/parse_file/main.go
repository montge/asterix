// Example: Parse ASTERIX data from file
package main

import (
	"fmt"
	"log"
	"os"

	asterix "github.com/montge/asterix/asterix-go"
)

func main() {
	if len(os.Args) < 2 {
		fmt.Println("Usage: parse_file <filename>")
		fmt.Println("  Parses raw ASTERIX binary file")
		os.Exit(1)
	}

	filename := os.Args[1]

	// Initialize the parser
	if err := asterix.Init(""); err != nil {
		log.Fatalf("Failed to initialize: %v", err)
	}

	// Read file
	data, err := os.ReadFile(filename)
	if err != nil {
		log.Fatalf("Failed to read file: %v", err)
	}

	fmt.Printf("Read %d bytes from %s\n", len(data), filename)

	// Parse with incremental offset to handle multiple blocks
	offset := 0
	totalRecords := 0

	for offset < len(data) {
		result, err := asterix.ParseWithOffset(data, offset, 100)
		if err != nil {
			log.Printf("Parse error at offset %d: %v", offset, err)
			break
		}

		if len(result.Records) == 0 {
			break
		}

		for _, rec := range result.Records {
			totalRecords++
			fmt.Printf("Record %d: Category=%d Length=%d CRC=0x%08x\n",
				totalRecords, rec.Category, rec.Length, rec.CRC)

			if rec.JSON != "" {
				fmt.Printf("  JSON: %s\n", rec.JSON)
			}
		}

		if result.BytesConsumed == 0 {
			break
		}
		offset += result.BytesConsumed
	}

	fmt.Printf("\nTotal: %d records parsed\n", totalRecords)
}
