# Changelog

All notable changes to the ASTERIX Ruby bindings will be documented in this file.

The format is based on [Keep a Changelog](https://keepachangelog.com/en/1.0.0/),
and this project adheres to [Semantic Versioning](https://semver.org/spec/v2.0.0.html).

## [Unreleased]

### Added
- Initial Ruby bindings implementation for ASTERIX decoder
- Native C extension using Ruby C API
- Support for Ruby 3.0, 3.1, 3.2, 3.3+
- Core API: `parse`, `parse_with_offset`, `describe`, `init`
- Comprehensive FFI boundary validation (follows BINDING_GUIDELINES.md)
- RSpec test suite with >80% coverage target
- Example programs: basic parsing, incremental parsing, descriptions, custom categories
- YARD documentation for all public APIs
- Thread safety warnings and documentation
- Incremental parsing support for large files
- Category auto-initialization on module load
- Helper methods: `list_configuration_files`, `get_configuration_file`

### Security
- Input validation at FFI boundary (buffer lengths, offsets, category ranges)
- Path traversal protection in `init` method
- Maximum message size enforcement (64KB limit)
- Maximum blocks per call limit (10,000 blocks)
- Comprehensive bounds checking in `parse_with_offset`

### Documentation
- Comprehensive README.md with usage examples
- API reference documentation with YARD
- Thread safety warnings
- Installation instructions for all platforms
- Troubleshooting guide
- Example programs with detailed comments

### Testing
- RSpec test suite covering all public APIs
- Input validation tests (edge cases, error conditions)
- Integration tests with sample data
- SimpleCov for code coverage reporting
- RuboCop for code quality
- bundler-audit for security auditing

[Unreleased]: https://github.com/montge/asterix/tree/master/asterix-ruby
