#!/usr/bin/env ruby
# frozen_string_literal: true

# ASTERIX category description example
#
# This example demonstrates how to get human-readable descriptions
# for ASTERIX categories, items, and fields.

require 'asterix'

# List of common ASTERIX categories to describe
CATEGORIES = [
  1,   # Monoradar Track Messages
  2,   # Monoradar Plot Messages
  19,  # Multilateration System Status Messages
  20,  # Multilateration Target Reports
  21,  # ADS-B Target Reports
  23,  # CNS/ATM Ground Station and Service Reports
  34,  # Monoradar Service Messages
  48,  # Monoradar Target Reports
  62,  # System Track Data
  63,  # Sensor Status Reports
  65,  # SDPS Service Status Reports
  252  # Reserved for Military Use
].freeze

puts 'ASTERIX Category Descriptions'
puts '=' * 70
puts ''

CATEGORIES.each do |category|
  description = Asterix.describe(category)
  puts "CAT#{format('%03d', category)}: #{description}"
end

puts ''
puts '=' * 70
puts ''

# Example: Describe CAT048 items
category = 48
puts "Detailed descriptions for CAT#{format('%03d', category)}:"
puts ''

# Some common CAT048 items
items = %w[010 020 040 070 140 220]

items.each do |item|
  item_desc = Asterix.describe(category, item)
  puts "  I#{item}: #{item_desc}"

  # Get field descriptions (example for I010)
  if item == '010'
    %w[SAC SIC].each do |field|
      field_desc = Asterix.describe(category, item, field)
      puts "    - #{field}: #{field_desc}"
    end
  end
end

puts ''
puts '=' * 70
puts ''

# Example: List all available configuration files
puts 'Available ASTERIX category configurations:'
puts ''

config_files = Asterix.list_configuration_files
if config_files.any?
  config_files.each do |file|
    basename = File.basename(file)
    puts "  - #{basename}"
  end
else
  puts '  (No configuration files found)'
end
