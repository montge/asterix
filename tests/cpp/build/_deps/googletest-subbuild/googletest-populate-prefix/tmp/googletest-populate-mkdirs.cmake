# Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
# file Copyright.txt or https://cmake.org/licensing for details.

cmake_minimum_required(VERSION 3.5)

file(MAKE_DIRECTORY
  "/path/to/asterix/tests/cpp/build/_deps/googletest-src"
  "/path/to/asterix/tests/cpp/build/_deps/googletest-build"
  "/path/to/asterix/tests/cpp/build/_deps/googletest-subbuild/googletest-populate-prefix"
  "/path/to/asterix/tests/cpp/build/_deps/googletest-subbuild/googletest-populate-prefix/tmp"
  "/path/to/asterix/tests/cpp/build/_deps/googletest-subbuild/googletest-populate-prefix/src/googletest-populate-stamp"
  "/path/to/asterix/tests/cpp/build/_deps/googletest-subbuild/googletest-populate-prefix/src"
  "/path/to/asterix/tests/cpp/build/_deps/googletest-subbuild/googletest-populate-prefix/src/googletest-populate-stamp"
)

set(configSubDirs )
foreach(subDir IN LISTS configSubDirs)
    file(MAKE_DIRECTORY "/path/to/asterix/tests/cpp/build/_deps/googletest-subbuild/googletest-populate-prefix/src/googletest-populate-stamp/${subDir}")
endforeach()
if(cfgdir)
  file(MAKE_DIRECTORY "/path/to/asterix/tests/cpp/build/_deps/googletest-subbuild/googletest-populate-prefix/src/googletest-populate-stamp${cfgdir}") # cfgdir has leading slash
endif()
