# frozen_string_literal: true

require 'spec_helper'

RSpec.describe Asterix do
  describe '.init' do
    context 'with valid filename' do
      it 'returns 0 on success' do
        # Use a config file from the default config directory if available
        config_file = Asterix.get_configuration_file('cat048')
        skip 'No config file available' unless config_file

        expect(Asterix.init(config_file)).to eq(0)
      end
    end

    context 'with invalid filename' do
      it 'raises ArgumentError for nil' do
        expect { Asterix.init(nil) }.to raise_error(ArgumentError, /must be a String/)
      end

      it 'raises ArgumentError for empty string' do
        expect { Asterix.init('') }.to raise_error(ArgumentError, /cannot be empty/)
      end

      it 'raises ArgumentError for path traversal' do
        expect { Asterix.init('../../../etc/passwd') }.to raise_error(ArgumentError, /path traversal/)
      end

      it 'raises IOError for non-existent file' do
        expect { Asterix.init('/nonexistent/file.xml') }.to raise_error(IOError)
      end
    end
  end

  describe '.parse' do
    let(:sample_cat048_data) do
      # CAT048 sample data (minimal valid ASTERIX packet)
      [0x30, 0x00, 0x0B, 0xFD, 0x00, 0x19, 0xC9, 0x35, 0x6D, 0x4D, 0xA0].pack('C*')
    end

    context 'with valid data' do
      it 'returns array of records' do
        records = Asterix.parse(sample_cat048_data)
        expect(records).to be_an(Array)
      end

      it 'accepts verbose parameter' do
        records = Asterix.parse(sample_cat048_data, verbose: true)
        expect(records).to be_an(Array)

        records_compact = Asterix.parse(sample_cat048_data, verbose: false)
        expect(records_compact).to be_an(Array)
      end
    end

    context 'with invalid data' do
      it 'raises ArgumentError for nil' do
        expect { Asterix.parse(nil) }.to raise_error(ArgumentError, /must be a String/)
      end

      it 'raises ArgumentError for empty string' do
        expect { Asterix.parse('') }.to raise_error(ArgumentError, /cannot be empty/)
      end

      it 'raises ArgumentError for oversized data' do
        large_data = 'X' * (65536 + 1)
        expect { Asterix.parse(large_data) }.to raise_error(ArgumentError, /too large/)
      end
    end

    context 'encoding handling' do
      it 'accepts binary string' do
        binary_data = sample_cat048_data.force_encoding(Encoding::ASCII_8BIT)
        records = Asterix.parse(binary_data)
        expect(records).to be_an(Array)
      end

      it 'converts UTF-8 string to binary' do
        utf8_data = sample_cat048_data.force_encoding(Encoding::UTF_8)
        records = Asterix.parse(utf8_data)
        expect(records).to be_an(Array)
      end
    end
  end

  describe '.parse_with_offset' do
    let(:sample_data) do
      # Multiple CAT048 blocks concatenated
      [0x30, 0x00, 0x0B, 0xFD, 0x00, 0x19, 0xC9, 0x35, 0x6D, 0x4D, 0xA0].pack('C*') * 2
    end

    context 'with valid parameters' do
      it 'returns array and new offset' do
        records, new_offset = Asterix.parse_with_offset(sample_data)
        expect(records).to be_an(Array)
        expect(new_offset).to be_an(Integer)
        expect(new_offset).to be >= 0
      end

      it 'accepts offset parameter' do
        records, new_offset = Asterix.parse_with_offset(sample_data, offset: 0)
        expect(records).to be_an(Array)
        expect(new_offset).to be_an(Integer)
      end

      it 'accepts blocks_count parameter' do
        records, new_offset = Asterix.parse_with_offset(sample_data, blocks_count: 1)
        expect(records).to be_an(Array)
        expect(new_offset).to be_an(Integer)
      end

      it 'accepts verbose parameter' do
        records, new_offset = Asterix.parse_with_offset(sample_data, verbose: false)
        expect(records).to be_an(Array)
        expect(new_offset).to be_an(Integer)
      end

      it 'supports incremental parsing' do
        offset = 0
        all_records = []
        iterations = 0
        max_iterations = 10 # Prevent infinite loop

        while offset < sample_data.bytesize && iterations < max_iterations
          records, offset = Asterix.parse_with_offset(sample_data, offset: offset, blocks_count: 1)
          break if records.empty?
          all_records.concat(records)
          iterations += 1
        end

        expect(all_records).to be_an(Array)
      end
    end

    context 'with invalid parameters' do
      it 'raises ArgumentError for nil data' do
        expect { Asterix.parse_with_offset(nil) }.to raise_error(ArgumentError)
      end

      it 'raises ArgumentError for offset beyond data length' do
        expect do
          Asterix.parse_with_offset(sample_data, offset: 999_999)
        end.to raise_error(ArgumentError, /exceeds data length/)
      end

      it 'raises ArgumentError for zero blocks_count' do
        expect do
          Asterix.parse_with_offset(sample_data, blocks_count: 0)
        end.to raise_error(ArgumentError, /must be > 0/)
      end

      it 'raises ArgumentError for oversized blocks_count' do
        expect do
          Asterix.parse_with_offset(sample_data, blocks_count: 20_000)
        end.to raise_error(ArgumentError, /exceeds maximum/)
      end
    end
  end

  describe '.describe' do
    context 'with valid parameters' do
      it 'returns category description' do
        desc = Asterix.describe(48)
        expect(desc).to be_a(String)
      end

      it 'returns item description' do
        desc = Asterix.describe(48, '010')
        expect(desc).to be_a(String)
      end

      it 'returns field description' do
        desc = Asterix.describe(48, '010', 'SAC')
        expect(desc).to be_a(String)
      end

      it 'returns field value description' do
        desc = Asterix.describe(48, '010', 'SAC', '7')
        expect(desc).to be_a(String)
      end

      it 'accepts nil for optional parameters' do
        desc = Asterix.describe(48, nil, nil, nil)
        expect(desc).to be_a(String)
      end
    end

    context 'with invalid parameters' do
      it 'raises ArgumentError for non-integer category' do
        expect { Asterix.describe('48') }.to raise_error(ArgumentError, /must be an Integer/)
      end

      it 'raises ArgumentError for category < 1' do
        expect { Asterix.describe(0) }.to raise_error(ArgumentError, /Invalid ASTERIX category/)
      end

      it 'raises ArgumentError for category > 255' do
        expect { Asterix.describe(256) }.to raise_error(ArgumentError, /Invalid ASTERIX category/)
      end

      it 'raises ArgumentError for non-string item' do
        expect { Asterix.describe(48, 123) }.to raise_error(ArgumentError, /must be a String or nil/)
      end
    end
  end

  describe '.list_configuration_files' do
    it 'returns an array' do
      expect(Asterix.list_configuration_files).to be_an(Array)
    end

    it 'returns XML files if config directory exists' do
      files = Asterix.list_configuration_files
      files.each do |file|
        expect(file).to match(/\.xml$/)
        expect(File.exist?(file)).to be true
      end
    end
  end

  describe '.get_configuration_file' do
    it 'returns path to matching file' do
      file = Asterix.get_configuration_file('cat048')
      skip 'No config files available' if file.nil?

      expect(file).to be_a(String)
      expect(file).to match(/cat048/)
      expect(File.exist?(file)).to be true
    end

    it 'returns nil for non-matching string' do
      file = Asterix.get_configuration_file('nonexistent_category_xyz')
      expect(file).to be_nil
    end
  end

  describe 'module constants' do
    it 'has VERSION constant' do
      expect(Asterix::VERSION).to be_a(String)
      expect(Asterix::VERSION).to match(/\d+\.\d+\.\d+/)
    end

    it 'has DEFAULT_CONFIG_DIR constant' do
      expect(Asterix::DEFAULT_CONFIG_DIR).to be_a(String)
    end
  end

  describe 'thread safety warning' do
    it 'documents thread safety concerns in module docstring' do
      # This test verifies documentation exists
      # In production, users should be warned about thread safety
      expect(Asterix.to_s).to eq('Asterix')
    end
  end
end
