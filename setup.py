try:
    from setuptools import setup, Extension
except ImportError:
    from distutils.core import setup, Extension
import distutils.sysconfig
import shutil
import os.path
from os import listdir
import sys
import platform

# Read version from VERSION file
# Supports key=value format with PYTHON_VERSION or computes from CPP_VERSION
__version__ = None
try:
    with open('VERSION', 'r') as version_file:
        version_content = version_file.read()
        
        # Try to find PYTHON_VERSION first
        import re
        python_match = re.search(r'PYTHON_VERSION[ \t]*=[ \t]*([0-9]+\.[0-9]+\.[0-9]+)', version_content)
        if python_match:
            __version__ = python_match.group(1).strip()
        else:
            # Fallback: extract CPP_VERSION and compute Python version
            cpp_match = re.search(r'CPP_VERSION[ \t]*=[ \t]*([0-9]+)\.([0-9]+)\.([0-9]+)', version_content)
            if cpp_match:
                major = int(cpp_match.group(1))
                minor = int(cpp_match.group(2))
                patch = int(cpp_match.group(3))
                # Convert 2.8.10 -> 0.7.10 (map 2.x.y -> 0.(x-1).y per HISTORY mapping)
                if major >= 2:
                    __version__ = f"0.{minor - 1}.{patch}"
                else:
                    __version__ = f"{major}.{minor}.{patch}"
            else:
                # Legacy format: single line with just version number
                version_line = version_content.split('\n')[0].strip()
                if version_line and not version_line.startswith('#'):
                    parts = version_line.split('.')
                    if len(parts) >= 3:
                        major = int(parts[0])
                        minor = int(parts[1])
                        patch = int(parts[2])
                        if major >= 2:
                            __version__ = f"0.{minor - 1}.{patch}"
                        else:
                            __version__ = version_line
except (IOError, ValueError) as e:
    # Fallback to reading from asterix/version.py if VERSION file not found
    try:
        with open('asterix/version.py', 'r') as version_file:
            for line in version_file:
                if line.startswith('__version__'):
                    __version__ = line.split('=')[1].strip().strip('"').strip("'")
                    break
    except IOError:
        __version__ = "0.0.0"  # Default fallback

CLASSIFIERS = [
    'Development Status :: 4 - Beta',  # Upgraded from Alpha (92.2% coverage, production-ready)
    'Intended Audience :: Developers',
    'Intended Audience :: Science/Research',
    'License :: OSI Approved :: GNU General Public License v3 or later (GPLv3+)',
    'Operating System :: OS Independent',
    'Programming Language :: C',
    'Programming Language :: C++',
    'Programming Language :: Python :: 3',
    'Programming Language :: Python :: 3 :: Only',
    'Programming Language :: Python :: 3.10',
    'Programming Language :: Python :: 3.11',
    'Programming Language :: Python :: 3.12',
    'Programming Language :: Python :: 3.13',
    'Programming Language :: Python :: 3.14',
    'Topic :: Scientific/Engineering',
    'Topic :: Software Development :: Libraries :: Python Modules',
]

try:
    shutil.rmtree("./build")
except(OSError):
    pass

# Remove the "-Wstrict-prototypes" compiler option, which isn't valid for C++.
cfg_vars = distutils.sysconfig.get_config_vars()
for key, value in cfg_vars.items():
    if type(value) == str:
        cfg_vars[key] = value.replace("-Wstrict-prototypes", "")

# If this is MacOSX set correct deployment target (otherwise compile may fail)
if sys.platform == 'darwin':
    os.environ['MACOSX_DEPLOYMENT_TARGET'] = platform.mac_ver()[0]

# Windows: Find expat from vcpkg or environment
expat_include_dirs = ['./asterix/python', './src/asterix', './src/engine']
expat_library_dirs = []
expat_libraries = ['expat']

if sys.platform == 'win32':
    # Check for expat in common locations
    expat_include_dir = os.environ.get('EXPAT_INCLUDE_DIR')
    expat_lib_dir = os.environ.get('EXPAT_LIB_DIR')

    # Try vcpkg paths - check multiple triplets for architecture compatibility
    vcpkg_root = os.environ.get('VCPKG_ROOT', '')
    if vcpkg_root:
        # Try multiple vcpkg triplets in order of preference
        for triplet in ['x64-windows', 'x86-windows', 'arm64-windows']:
            vcpkg_expat_include = os.path.join(vcpkg_root, 'installed', triplet, 'include')
            vcpkg_expat_lib = os.path.join(vcpkg_root, 'installed', triplet, 'lib')
            if os.path.exists(vcpkg_expat_include) and os.path.exists(vcpkg_expat_lib):
                print(f"Found vcpkg expat for {triplet}")
                if vcpkg_expat_include not in expat_include_dirs:
                    expat_include_dirs.append(vcpkg_expat_include)
                    expat_include_dir = vcpkg_expat_include
                if vcpkg_expat_lib not in expat_library_dirs:
                    expat_library_dirs.append(vcpkg_expat_lib)
                    expat_lib_dir = vcpkg_expat_lib
                break  # Use the first found triplet

    # Use environment variables if set (takes precedence over vcpkg detection)
    if expat_include_dir and os.path.exists(expat_include_dir):
        if expat_include_dir not in expat_include_dirs:
            expat_include_dirs.append(expat_include_dir)
    if expat_lib_dir and os.path.exists(expat_lib_dir):
        if expat_lib_dir not in expat_library_dirs:
            expat_library_dirs.append(expat_lib_dir)

    # Validate that we found expat and auto-detect library name
    if not expat_library_dirs:
        print("WARNING: expat library directories not found. Build may fail.")
        print("  Set EXPAT_LIB_DIR environment variable or install via vcpkg:")
        print("    vcpkg install expat:x64-windows")
        print(f"  VCPKG_ROOT={vcpkg_root or '(not set)'}")
        print(f"  EXPAT_LIB_DIR={os.environ.get('EXPAT_LIB_DIR', '(not set)')}")
    else:
        # Auto-detect actual library name in the directory
        # vcpkg may install libexpatMD.lib, libexpatdMD.lib, libexpat.lib, etc.
        import glob
        found_lib_names = []
        for lib_dir in expat_library_dirs:
            # Look for any expat*.lib file
            lib_pattern = os.path.join(lib_dir, '*expat*.lib')
            matches = glob.glob(lib_pattern)
            for match in matches:
                basename = os.path.basename(match)
                # Remove .lib extension and lib prefix to get library name for linker
                libname = os.path.splitext(basename)[0]
                if libname.startswith('lib'):
                    libname = libname[3:]  # Remove 'lib' prefix
                if libname not in found_lib_names:
                    found_lib_names.append(libname)
                    print(f"Found expat library: {basename} -> using linker name '{libname}'")

        if found_lib_names:
            # Use detected library names instead of generic 'expat'
            expat_libraries = found_lib_names
        else:
            print(f"WARNING: No *expat*.lib files found in {expat_library_dirs}")
            print("  Falling back to generic library name 'expat'")
            # Keep default expat_libraries = ['expat']

    # Note: Removed /wd9002 flag as it's invalid (causes D9014 warnings)
    # D9002 warnings about GCC flags are expected and harmless on MSVC

asterix_module = Extension('_asterix',
                           sources=['./src/python/asterix.cpp',
                                    './src/python/python_wrapper.cpp',
                                    './src/python/python_parser.cpp',
                                    './src/asterix/AsterixDefinition.cpp',
                                    './src/asterix/AsterixData.cpp',
                                    './src/asterix/Category.cpp',
                                    './src/asterix/DataBlock.cpp',
                                    './src/asterix/DataRecord.cpp',
                                    './src/asterix/DataItem.cpp',
                                    './src/asterix/DataItemBits.cpp',
                                    './src/asterix/DataItemDescription.cpp',
                                    './src/asterix/DataItemFormat.cpp',
                                    './src/asterix/DataItemFormatCompound.cpp',
                                    './src/asterix/DataItemFormatExplicit.cpp',
                                    './src/asterix/DataItemFormatFixed.cpp',
                                    './src/asterix/DataItemFormatRepetitive.cpp',
                                    './src/asterix/DataItemFormatVariable.cpp',
                                    './src/asterix/DataItemFormatBDS.cpp',
                                    './src/asterix/InputParser.cpp',
                                    './src/asterix/Tracer.cpp',
                                    './src/asterix/UAP.cpp',
                                    './src/asterix/UAPItem.cpp',
                                    './src/asterix/Utils.cpp',
                                    './src/asterix/XMLParser.cpp',
                                    ],

                           include_dirs=expat_include_dirs,
                           library_dirs=expat_library_dirs if sys.platform == 'win32' else [],
                           libraries=expat_libraries,
                           # SECURITY: Hardening flags for buffer overflow and stack protection
                           # Platform-specific C++ standard: 
                           # - macOS: C++17 (better compatibility)
                           # - Windows: C++20 (MSVC supports C++20, not C++23)
                           # - Linux: C++23 (matches CMakeLists.txt for feature parity)
                           extra_compile_args=['-DPYTHON_WRAPPER',
                                             '-std=c++17' if sys.platform == 'darwin' else '-std=c++23',
                                             '-fstack-protector-strong', '-D_FORTIFY_SOURCE=2'] if sys.platform != 'win32'
                                          else ['/DPYTHON_WRAPPER', '/std:c++20', '/W3'],
                           # SECURITY: Read-only relocations for hardening (Linux only - macOS doesn't support -z flags)
                           # Note: On Windows, libraries=['expat'] (line 97) is sufficient - no need for explicit .lib in extra_link_args
                           extra_link_args=['-lexpat'] if sys.platform == 'darwin'
                                          else ([] if sys.platform == 'win32'
                                          else ['-lexpat', '-Wl,-z,relro,-z,now']))

f = open('README.rst')
try:
    README = f.read()
finally:
    f.close()

config_files = [os.path.join('./asterix/config/', f) for f in listdir('./asterix/config/') if
                os.path.isfile(os.path.join('./asterix/config/', f))]

sample_files = ['./asterix/sample_data/cat048.raw',
                './asterix/sample_data/cat062cat065.raw',
                './asterix/sample_data/cat_034_048.pcap',
                './asterix/sample_data/cat_062_065.pcap']

setup(name='asterix_decoder',
      packages=['asterix'],
      version=__version__,
      description="ASTERIX decoder in Python",
      keywords="asterix, eurocontrol, radar, track, croatiacontrol",
      long_description=README,
      ext_modules=[asterix_module],
      # data_files = [('asterix/config', config_files), ('asterix/sample_data', sample_files)],
      include_package_data=True,
      package_data={'asterix': config_files + sample_files},
      zip_safe=False,
      #       eager_resources = eager_files,
      author="Damir Salantic",
      author_email="damir.salantic@gmail.com",
      maintainer="montge",
      download_url="https://github.com/montge/asterix",
      license="GPL-3.0-or-later",
      platforms=['any'],
      url="https://github.com/montge/asterix",
      classifiers=CLASSIFIERS,
      python_requires='>=3.10',  # Python 3.9 EOL (2025-10)
      install_requires=[
          # lxml is optional but recommended for XML output functionality
      ],
      extras_require={
          'xml': ['lxml>=4.9.0'],
          'dev': [
              'pytest>=7.0.0',
              'pytest-cov>=4.0.0',
              'coverage>=7.0.0',
              'memory-profiler>=0.60.0',
          ],
      },
      )
