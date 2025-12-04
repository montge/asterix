require 'mkmf'

# Extension name
extension_name = 'asterix_ext'

# Get the project root directory (two levels up from ext/asterix/)
project_root = File.expand_path('../../../..', __FILE__)

# Add include directories for ASTERIX C++ headers
$INCFLAGS << " -I#{project_root}/src/asterix"
$INCFLAGS << " -I#{project_root}/src/engine"
$INCFLAGS << " -I#{project_root}/src/main"

# Check for required libraries
have_library('expat') or abort "*** ERROR: libexpat not found. Install libexpat-dev (Debian/Ubuntu) or expat-devel (RHEL/Fedora)"
have_library('stdc++') or abort "*** ERROR: C++ standard library not found"

# Set C++ standard (C++17 minimum, C++23 preferred on Linux)
if RUBY_PLATFORM =~ /linux/
  # Linux: Try C++23, fallback to C++20, then C++17
  $CXXFLAGS << " -std=c++23"
elsif RUBY_PLATFORM =~ /darwin/
  # macOS: Use C++17 for better compatibility
  $CXXFLAGS << " -std=c++17"
elsif RUBY_PLATFORM =~ /mingw|mswin/
  # Windows: Use C++20 (MSVC doesn't support C++23 yet)
  $CXXFLAGS << " -std=c++20"
else
  # Default: C++17
  $CXXFLAGS << " -std=c++17"
end

# Add optimization flags for release builds
$CXXFLAGS << " -O2 -DNDEBUG" unless ENV['DEBUG']

# Add debug flags if DEBUG environment variable is set
if ENV['DEBUG']
  $CXXFLAGS << " -g -O0"
  $CFLAGS << " -g -O0"
end

# Enable all warnings
$CXXFLAGS << " -Wall -Wextra"
$CFLAGS << " -Wall -Wextra"

# Define source files
# We need to compile the ASTERIX C++ core files along with our wrapper
asterix_sources = %w[
  AsterixData.cpp
  AsterixDefinition.cpp
  Category.cpp
  DataBlock.cpp
  DataItem.cpp
  DataItemBits.cpp
  DataItemDescription.cpp
  DataItemFormat.cpp
  DataItemFormatBDS.cpp
  DataItemFormatCompound.cpp
  DataItemFormatExplicit.cpp
  DataItemFormatFixed.cpp
  DataItemFormatRepetitive.cpp
  DataItemFormatVariable.cpp
  DataRecord.cpp
  Tracer.cpp
  UAP.cpp
  UAPItem.cpp
  Utils.cpp
  WatchDog.cpp
  XMLParser.cpp
  InputParser.cpp
  asterixformat.cpp
  asterixformatdescriptor.cpp
  asterixgpssubformat.cpp
  asterixhdlcsubformat.cpp
  asterixpcapsubformat.cpp
  asterixrawsubformat.cpp
  asterixfinalsubformat.cpp
]

engine_sources = %w[
  basedevice.cpp
  baseformat.cpp
  baseformatdescriptor.cpp
  channelmanager.cpp
  descriptor.cpp
  serialdevice.cpp
  sockdevice.cpp
  stdindevice.cpp
  udpdevice.cpp
  tcpdevice.cpp
]

# Build full paths to source files
asterix_obj_files = asterix_sources.map { |f| "#{project_root}/src/asterix/#{f}" }
engine_obj_files = engine_sources.map { |f| "#{project_root}/src/engine/#{f}" }

# Add our Ruby wrapper source files
wrapper_sources = [
  'ruby_parser.cpp',
  'asterix_ext.c'
]

# Combine all source files
all_sources = asterix_obj_files + engine_obj_files + wrapper_sources

# Set source files for the extension
$srcs = all_sources
$objs = all_sources.map { |src| src.sub(/\.(cpp|c)$/, '.o') }

# Create the Makefile
create_makefile(extension_name)

puts "=" * 70
puts "ASTERIX Ruby Extension Configuration"
puts "=" * 70
puts "Extension name: #{extension_name}"
puts "Project root:   #{project_root}"
puts "C++ Standard:   #{$CXXFLAGS[/std=c\+\+\d+/]}"
puts "Source files:   #{all_sources.length} files"
puts "  - ASTERIX:    #{asterix_sources.length} files"
puts "  - Engine:     #{engine_sources.length} files"
puts "  - Wrapper:    #{wrapper_sources.length} files"
puts "=" * 70
