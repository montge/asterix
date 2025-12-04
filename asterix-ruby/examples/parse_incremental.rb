#!/usr/bin/env ruby
# frozen_string_literal: true

# Incremental ASTERIX parsing example
#
# This example demonstrates how to parse large ASTERIX files incrementally
# to avoid loading all data into memory at once.

require 'asterix'

# Check command line arguments
if ARGV.empty?
  puts 'Usage: ruby parse_incremental.rb <asterix_file> [blocks_per_iteration]'
  puts ''
  puts 'Example:'
  puts '  ruby parse_incremental.rb ../sample_data/large_file.asterix 100'
  exit 1
end

filename = ARGV[0]
blocks_per_iteration = (ARGV[1] || 100).to_i

unless File.exist?(filename)
  puts "Error: File not found: #{filename}"
  exit 1
end

# Read ASTERIX data from file
data = File.binread(filename)

puts "Incremental parsing of: #{filename}"
puts "File size: #{data.bytesize} bytes"
puts "Blocks per iteration: #{blocks_per_iteration}"
puts ''

# Incremental parsing loop
offset = 0
total_records = 0
iteration = 0

begin
  while offset < data.bytesize
    iteration += 1

    # Parse next chunk
    records, offset = Asterix.parse_with_offset(
      data,
      offset: offset,
      blocks_count: blocks_per_iteration,
      verbose: false # Disable verbose mode for performance
    )

    # Break if no more records
    break if records.empty?

    total_records += records.length

    puts "Iteration #{iteration}: Parsed #{records.length} records, offset now at #{offset}"

    # Process records (example: count categories)
    categories = records.group_by { |r| r[:category] }
    categories.each do |cat, cat_records|
      puts "  - Category #{cat}: #{cat_records.length} records"
    end
  end

  puts ''
  puts "Parsing complete!"
  puts "Total records: #{total_records}"
  puts "Total iterations: #{iteration}"
  puts "Final offset: #{offset} / #{data.bytesize} bytes"

rescue ArgumentError => e
  puts "Error: #{e.message}"
  exit 1
rescue RuntimeError => e
  puts "Parser error: #{e.message}"
  exit 1
end
