#!/usr/bin/env ruby
# frozen_string_literal: true

# Custom ASTERIX category loading example
#
# This example demonstrates how to load custom ASTERIX category definitions
# from XML files.

require 'asterix'

# Check command line arguments
if ARGV.length < 2
  puts 'Usage: ruby custom_category.rb <category_xml> <data_file>'
  puts ''
  puts 'Example:'
  puts '  ruby custom_category.rb /path/to/asterix_cat999_1_0.xml sample.asterix'
  puts ''
  puts 'This script loads a custom ASTERIX category definition and parses data using it.'
  exit 1
end

category_xml = ARGV[0]
data_file = ARGV[1]

# Validate files exist
unless File.exist?(category_xml)
  puts "Error: Category definition not found: #{category_xml}"
  exit 1
end

unless File.exist?(data_file)
  puts "Error: Data file not found: #{data_file}"
  exit 1
end

puts 'Loading custom ASTERIX category definition'
puts "Category XML: #{category_xml}"
puts ''

# Load custom category definition
begin
  result = Asterix.init(category_xml)
  if result == 0
    puts "Successfully loaded category definition from: #{File.basename(category_xml)}"
  else
    puts "Warning: init returned #{result}"
  end
rescue IOError => e
  puts "Error loading category: #{e.message}"
  exit 1
rescue SyntaxError => e
  puts "Error parsing XML: #{e.message}"
  exit 1
end

puts ''
puts 'Reading data file...'

# Read and parse data
data = File.binread(data_file)
puts "Data file size: #{data.bytesize} bytes"
puts ''

begin
  records = Asterix.parse(data)

  puts "Parsed #{records.length} ASTERIX records"

  if records.any?
    puts ''
    puts 'Records by category:'
    categories = records.group_by { |r| r[:category] }
    categories.each do |cat, cat_records|
      cat_desc = Asterix.describe(cat)
      puts "  CAT#{format('%03d', cat)} (#{cat_desc}): #{cat_records.length} records"
    end

    puts ''
    puts 'First record details:'
    record = records.first
    puts "  Category:  #{record[:category]}"
    puts "  Length:    #{record[:len]} bytes"
    puts "  Timestamp: #{record[:ts]} ms"
    puts "  CRC:       #{record[:crc]}"

    if record[:items]
      puts "  Items:     #{record[:items].length}"
      puts ''
      puts '  Data items:'
      record[:items].each do |item_name, item_data|
        item_desc = Asterix.describe(record[:category], item_name)
        puts "    #{item_name}: #{item_desc}"
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
