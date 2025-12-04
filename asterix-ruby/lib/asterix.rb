# frozen_string_literal: true

require 'asterix_ext'

# ASTERIX (All Purpose STructured EUROCONTROL SuRveillance Information EXchange) decoder
#
# This module provides Ruby bindings for decoding ASTERIX surveillance data used in
# Air Traffic Management (ATM) and Air Traffic Control (ATC) systems.
#
# @example Basic usage
#   require 'asterix'
#
#   # Parse ASTERIX data
#   data = File.binread('sample.pcap')
#   records = Asterix.parse(data)
#
#   records.each do |record|
#     puts "Category #{record[:category]}: #{record[:items].length} items"
#   end
#
# @example Custom categories
#   # Load custom category definition
#   Asterix.init('/path/to/asterix_cat999_1_0.xml')
#
# @example Incremental parsing
#   data = File.binread('large_file.asterix')
#   offset = 0
#   all_records = []
#
#   while offset < data.bytesize
#     records, offset = Asterix.parse_with_offset(data, offset: offset, blocks_count: 100)
#     break if records.empty?
#     all_records.concat(records)
#   end
#
# @note Thread Safety Warning
#   This module is NOT thread-safe. The ASTERIX parser uses global state to manage
#   category definitions. Concurrent calls from multiple threads will cause race
#   conditions and undefined behavior.
#
#   For multi-threaded applications:
#   - Use a Mutex around all Asterix calls
#   - Use separate processes instead of threads
#   - Ensure all ASTERIX operations happen in a single thread
#
module Asterix
  # Version of the ASTERIX decoder
  VERSION = '2.8.10'

  # Default configuration directory (relative to gem root)
  DEFAULT_CONFIG_DIR = File.expand_path('../config', __dir__)

  class << self
    # Initialize ASTERIX parser with XML category definition
    #
    # This method loads a custom ASTERIX category definition from an XML file.
    # Use this when you need to add support for custom ASTERIX categories or
    # modify existing category definitions. All default categories are automatically
    # loaded when the module is required.
    #
    # @param filename [String] Path to XML category definition file
    # @return [Integer] 0 if successful
    # @raise [ArgumentError] if filename is invalid or empty
    # @raise [IOError] if file cannot be read
    # @raise [SyntaxError] if XML is malformed
    #
    # @example Load custom category
    #   Asterix.init('/path/to/asterix_cat999_1_0.xml')
    #
    # @example Load from gem config directory
    #   config_file = File.join(Asterix::DEFAULT_CONFIG_DIR, 'asterix_cat048_1_30.xml')
    #   Asterix.init(config_file)
    #
    def init(filename)
      raise ArgumentError, 'Filename must be a String' unless filename.is_a?(String)
      raise ArgumentError, 'Filename cannot be empty' if filename.empty?

      AsterixNative.init(filename)
    end

    # Parse ASTERIX data from binary string
    #
    # Parses ASTERIX binary data and returns an array of decoded records. Each record
    # contains the category, timestamp, data items, and their decoded values.
    #
    # @param data [String] Binary ASTERIX data (must be ASCII-8BIT encoding)
    # @param verbose [Boolean] Include descriptions and metadata in output (default: true)
    # @return [Array<Hash>] Array of parsed ASTERIX records
    # @raise [ArgumentError] if data is empty or too large (>64KB)
    # @raise [RuntimeError] if parser not initialized
    #
    # @example Parse with descriptions
    #   data = File.binread('sample.pcap')
    #   records = Asterix.parse(data)
    #   puts records[0][:category]  # => 48
    #
    # @example Parse without descriptions (faster, less memory)
    #   records = Asterix.parse(data, verbose: false)
    #
    def parse(data, verbose: true)
      raise ArgumentError, 'Data must be a String' unless data.is_a?(String)
      raise ArgumentError, 'Data cannot be empty' if data.empty?

      # Ensure data is in binary encoding
      data = data.b unless data.encoding == Encoding::ASCII_8BIT

      AsterixNative.parse(data, verbose)
    end

    # Parse ASTERIX data with incremental offset tracking
    #
    # This method enables incremental/streaming parsing of large ASTERIX data streams.
    # It parses a specified number of data blocks starting from a given offset and
    # returns the new offset for the next parsing iteration.
    #
    # @param data [String] Binary ASTERIX data buffer
    # @param offset [Integer] Byte offset to start parsing from (default: 0)
    # @param blocks_count [Integer] Maximum blocks to parse in this call (default: 1000)
    # @param verbose [Boolean] Include descriptions in output (default: true)
    # @return [Array(Array<Hash>, Integer)] Tuple of [records, new_offset]
    # @raise [ArgumentError] if parameters are invalid
    # @raise [RuntimeError] if parser not initialized
    #
    # @example Incremental parsing
    #   data = File.binread('large_file.asterix')
    #   offset = 0
    #   all_records = []
    #
    #   while offset < data.bytesize
    #     records, offset = Asterix.parse_with_offset(data, offset: offset, blocks_count: 100)
    #     break if records.empty?
    #     all_records.concat(records)
    #     puts "Parsed #{records.length} records, offset now at #{offset}"
    #   end
    #
    def parse_with_offset(data, offset: 0, blocks_count: 1000, verbose: true)
      raise ArgumentError, 'Data must be a String' unless data.is_a?(String)
      raise ArgumentError, 'Offset must be an Integer' unless offset.is_a?(Integer)
      raise ArgumentError, 'blocks_count must be an Integer' unless blocks_count.is_a?(Integer)

      # Ensure data is in binary encoding
      data = data.b unless data.encoding == Encoding::ASCII_8BIT

      AsterixNative.parse_with_offset(data, offset, blocks_count, verbose)
    end

    # Get human-readable description for ASTERIX category/item/field/value
    #
    # @param category [Integer] ASTERIX category number (1-255)
    # @param item [String, nil] Item name (e.g., "010"), optional
    # @param field [String, nil] Field name, optional
    # @param value [String, nil] Field value, optional
    # @return [String] Description text
    # @raise [ArgumentError] if category is invalid (not in 1-255 range)
    #
    # @example Get category description
    #   Asterix.describe(48)  # => "Monoradar Target Reports"
    #
    # @example Get item description
    #   Asterix.describe(48, '010')  # => "Data Source Identifier"
    #
    # @example Get field description
    #   Asterix.describe(48, '010', 'SAC')  # => "System Area Code"
    #
    # @example Get field value description
    #   Asterix.describe(48, '010', 'SAC', '7')  # => "System Area Code: 7"
    #
    def describe(category, item = nil, field = nil, value = nil)
      raise ArgumentError, 'Category must be an Integer' unless category.is_a?(Integer)
      raise ArgumentError, 'Item must be a String or nil' unless item.nil? || item.is_a?(String)
      raise ArgumentError, 'Field must be a String or nil' unless field.nil? || field.is_a?(String)
      raise ArgumentError, 'Value must be a String or nil' unless value.nil? || value.is_a?(String)

      AsterixNative.describe(category, item, field, value)
    end

    # List all configuration files in the gem's config directory
    #
    # @return [Array<String>] Array of absolute paths to XML configuration files
    #
    # @example List all configs
    #   Asterix.list_configuration_files.each do |file|
    #     puts File.basename(file)
    #   end
    #
    def list_configuration_files
      return [] unless File.directory?(DEFAULT_CONFIG_DIR)

      Dir.glob(File.join(DEFAULT_CONFIG_DIR, '*.xml')).sort
    end

    # Get configuration file matching search string
    #
    # @param match [String] Search string to match against filenames
    # @return [String, nil] Path to first matching file, or nil if not found
    #
    # @example Get CAT048 config
    #   config = Asterix.get_configuration_file('cat048')
    #
    def get_configuration_file(match)
      list_configuration_files.find { |file| File.basename(file).include?(match) }
    end
  end
end

# Auto-initialize with default configuration files on module load
if File.directory?(Asterix::DEFAULT_CONFIG_DIR)
  Dir.glob(File.join(Asterix::DEFAULT_CONFIG_DIR, 'asterix_cat*.xml')).sort.each do |config_file|
    begin
      Asterix.init(config_file)
    rescue StandardError => e
      warn "Warning: Failed to load #{File.basename(config_file)}: #{e.message}"
    end
  end
end
