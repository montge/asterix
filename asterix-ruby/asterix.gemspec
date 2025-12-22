# frozen_string_literal: true

require_relative 'lib/asterix/version'

Gem::Specification.new do |spec|
  spec.name = 'asterix_decoder'
  spec.version = Asterix::VERSION
  spec.authors = ['ASTERIX Contributors', 'Croatia Control Ltd.']
  spec.email = ['asterix@crocontrol.hr']

  spec.summary = 'ASTERIX (All Purpose STructured EUROCONTROL SuRveillance Information EXchange) decoder'
  spec.description = <<~DESC
    Ruby bindings for ASTERIX decoder - a parser for EUROCONTROL ASTERIX protocol used in
    Air Traffic Management (ATM) and Air Traffic Control (ATC) systems. Supports CAT001-CAT252,
    PCAP encapsulation, and incremental parsing of large data streams.
  DESC

  spec.homepage = 'https://github.com/montge/asterix'
  spec.license = 'GPL-3.0-or-later'
  spec.required_ruby_version = '>= 3.0.0'

  spec.metadata['homepage_uri'] = spec.homepage
  spec.metadata['source_code_uri'] = 'https://github.com/montge/asterix/tree/master/asterix-ruby'
  spec.metadata['bug_tracker_uri'] = 'https://github.com/montge/asterix/issues'
  spec.metadata['changelog_uri'] = 'https://github.com/montge/asterix/blob/master/ChangeLog'
  spec.metadata['documentation_uri'] = 'https://rubydoc.info/gems/asterix_decoder'

  # Specify which files should be added to the gem when it is released.
  spec.files = Dir[
    'lib/**/*.rb',
    'ext/**/*.{c,cpp,h,rb}',
    'config/**/*.xml',
    'config/asterix.dtd',
    'README.md',
    'LICENSE',
    'CHANGELOG.md'
  ]

  spec.require_paths = ['lib']
  spec.extensions = ['ext/asterix/extconf.rb']

  # Runtime dependencies
  # (none - this is a native extension)

  # Development dependencies
  spec.add_development_dependency 'rake', '~> 13.0'
  spec.add_development_dependency 'rake-compiler', '~> 1.2'
  spec.add_development_dependency 'rspec', '~> 3.12'
  spec.add_development_dependency 'simplecov', '~> 0.22'
  spec.add_development_dependency 'yard', '~> 0.9'
  spec.add_development_dependency 'rubocop', '~> 1.50'
  spec.add_development_dependency 'bundler-audit', '~> 0.9'

  # System requirements
  spec.requirements << 'libexpat (expat-devel or libexpat-dev package)'
  spec.requirements << 'C++17 compatible compiler (GCC 7+, Clang 5+, MSVC 2019+)'
  spec.requirements << 'CMake 3.20+ (for building ASTERIX C++ core)'

  # Platform-specific notes
  spec.post_install_message = <<~MSG

    ========================================================================
    ASTERIX Ruby Decoder successfully installed!

    This gem provides Ruby bindings for the ASTERIX surveillance data decoder.

    Quick Start:
      require 'asterix'
      data = File.binread('sample.pcap')
      records = Asterix.parse(data)

    Documentation: https://rubydoc.info/gems/asterix_decoder

    Thread Safety Warning:
      This module uses global state and is NOT thread-safe.
      Use a Mutex or separate processes for concurrent parsing.

    System Requirements:
      - libexpat (already checked during installation)
      - C++17+ compiler

    Report issues: https://github.com/montge/asterix/issues
    ========================================================================

  MSG
end
