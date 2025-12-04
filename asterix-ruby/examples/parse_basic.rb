#!/usr/bin/env ruby
# frozen_string_literal: true

# Basic ASTERIX parsing example
#
# This example demonstrates the simplest way to parse ASTERIX data from a file.

require 'asterix'

# Check command line arguments
if ARGV.empty?
  puts 'Usage: ruby parse_basic.rb <asterix_file>'
  puts ''
  puts 'Example:'
  puts '  ruby parse_basic.rb ../sample_data/cat048.pcap'
  exit 1
end

filename = ARGV[0]

unless File.exist?(filename)
  puts "Error: File not found: #{filename}"
  exit 1
end

# Read ASTERIX data from file
data = File.binread(filename)

puts "Parsing ASTERIX data from: #{filename}"
puts "File size: #{data.bytesize} bytes"
puts ''

# Parse the data
begin
  records = Asterix.parse(data)

  puts "Parsed #{records.length} ASTERIX records"
  puts ''

  # Display first record details
  if records.any?
    record = records.first
    puts 'First record:'
    puts "  Category: #{record[:category]}"
    puts "  Length:   #{record[:len]} bytes"
    puts "  Items:    #{record[:items]&.length || 0}"

    if record[:items]
      puts '  Data items:'
      record[:items].each_key do |item_name|
        puts "    - #{item_name}"
      end
    end
  end

rescue ArgumentError => e
  puts "Error: #{e.message}"
  exit 1
rescue RuntimeError => e
  puts "Parser error: #{e.message}"
  exit 1
end
