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

# Safely load version without using exec()
__version__ = None
with open('asterix/version.py', 'r') as version_file:
    for line in version_file:
        if line.startswith('__version__'):
            # Parse the version string safely
            __version__ = line.split('=')[1].strip().strip('"').strip("'")

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

                           include_dirs=['./asterix/python', './src/asterix', './src/engine'],
                           # SECURITY: Hardening flags for buffer overflow and stack protection
                           # Platform-specific C++ standard: C++17 for macOS/Windows (better compiler support),
                           # C++23 for Linux (matches CMakeLists.txt for feature parity)
                           extra_compile_args=['-DPYTHON_WRAPPER',
                                             '-std=c++17' if sys.platform in ('darwin', 'win32') else '-std=c++23',
                                             '-fstack-protector-strong', '-D_FORTIFY_SOURCE=2'],
                           # SECURITY: Read-only relocations for hardening (Linux only - macOS doesn't support -z flags)
                           extra_link_args=['-lexpat'] if sys.platform == 'darwin'
                                          else (['-lexpat'] if sys.platform == 'win32'
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
