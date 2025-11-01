# Install script for directory: /home/e/Development/asterix

# Set the install prefix
if(NOT DEFINED CMAKE_INSTALL_PREFIX)
  set(CMAKE_INSTALL_PREFIX "/usr/local")
endif()
string(REGEX REPLACE "/$" "" CMAKE_INSTALL_PREFIX "${CMAKE_INSTALL_PREFIX}")

# Set the install configuration name.
if(NOT DEFINED CMAKE_INSTALL_CONFIG_NAME)
  if(BUILD_TYPE)
    string(REGEX REPLACE "^[^A-Za-z0-9_]+" ""
           CMAKE_INSTALL_CONFIG_NAME "${BUILD_TYPE}")
  else()
    set(CMAKE_INSTALL_CONFIG_NAME "")
  endif()
  message(STATUS "Install configuration: \"${CMAKE_INSTALL_CONFIG_NAME}\"")
endif()

# Set the component getting installed.
if(NOT CMAKE_INSTALL_COMPONENT)
  if(COMPONENT)
    message(STATUS "Install component: \"${COMPONENT}\"")
    set(CMAKE_INSTALL_COMPONENT "${COMPONENT}")
  else()
    set(CMAKE_INSTALL_COMPONENT)
  endif()
endif()

# Install shared libraries without execute permission?
if(NOT DEFINED CMAKE_INSTALL_SO_NO_EXE)
  set(CMAKE_INSTALL_SO_NO_EXE "1")
endif()

# Is this installation the result of a crosscompile?
if(NOT DEFINED CMAKE_CROSSCOMPILING)
  set(CMAKE_CROSSCOMPILING "FALSE")
endif()

# Set default install directory permissions.
if(NOT DEFINED CMAKE_OBJDUMP)
  set(CMAKE_OBJDUMP "/usr/bin/objdump")
endif()

if(CMAKE_INSTALL_COMPONENT STREQUAL "Unspecified" OR NOT CMAKE_INSTALL_COMPONENT)
  foreach(file
      "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/lib/libasterix.so.2.8.9"
      "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/lib/libasterix.so.2"
      )
    if(EXISTS "${file}" AND
       NOT IS_SYMLINK "${file}")
      file(RPATH_CHECK
           FILE "${file}"
           RPATH "")
    endif()
  endforeach()
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/lib" TYPE SHARED_LIBRARY FILES
    "/home/e/Development/asterix/lib/libasterix.so.2.8.9"
    "/home/e/Development/asterix/lib/libasterix.so.2"
    )
  foreach(file
      "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/lib/libasterix.so.2.8.9"
      "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/lib/libasterix.so.2"
      )
    if(EXISTS "${file}" AND
       NOT IS_SYMLINK "${file}")
      if(CMAKE_INSTALL_DO_STRIP)
        execute_process(COMMAND "/usr/bin/strip" "${file}")
      endif()
    endif()
  endforeach()
endif()

if(CMAKE_INSTALL_COMPONENT STREQUAL "Unspecified" OR NOT CMAKE_INSTALL_COMPONENT)
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/lib" TYPE SHARED_LIBRARY FILES "/home/e/Development/asterix/lib/libasterix.so")
endif()

if(CMAKE_INSTALL_COMPONENT STREQUAL "Unspecified" OR NOT CMAKE_INSTALL_COMPONENT)
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/include/asterix" TYPE FILE FILES
    "/home/e/Development/asterix/src/asterix/AsterixData.h"
    "/home/e/Development/asterix/src/asterix/AsterixDefinition.h"
    "/home/e/Development/asterix/src/asterix/Category.h"
    "/home/e/Development/asterix/src/asterix/DataBlock.h"
    "/home/e/Development/asterix/src/asterix/DataItem.h"
    "/home/e/Development/asterix/src/asterix/DataItemBits.h"
    "/home/e/Development/asterix/src/asterix/DataItemDescription.h"
    "/home/e/Development/asterix/src/asterix/DataItemFormat.h"
    "/home/e/Development/asterix/src/asterix/DataItemFormatBDS.h"
    "/home/e/Development/asterix/src/asterix/DataItemFormatCompound.h"
    "/home/e/Development/asterix/src/asterix/DataItemFormatExplicit.h"
    "/home/e/Development/asterix/src/asterix/DataItemFormatFixed.h"
    "/home/e/Development/asterix/src/asterix/DataItemFormatRepetitive.h"
    "/home/e/Development/asterix/src/asterix/DataItemFormatVariable.h"
    "/home/e/Development/asterix/src/asterix/DataRecord.h"
    "/home/e/Development/asterix/src/asterix/InputParser.h"
    "/home/e/Development/asterix/src/asterix/Tracer.h"
    "/home/e/Development/asterix/src/asterix/UAP.h"
    "/home/e/Development/asterix/src/asterix/UAPItem.h"
    "/home/e/Development/asterix/src/asterix/Utils.h"
    "/home/e/Development/asterix/src/asterix/XMLParser.h"
    )
endif()

if(CMAKE_INSTALL_COMPONENT STREQUAL "Unspecified" OR NOT CMAKE_INSTALL_COMPONENT)
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/lib" TYPE STATIC_LIBRARY FILES "/home/e/Development/asterix/lib/libasterix.a")
endif()

if(CMAKE_INSTALL_COMPONENT STREQUAL "Unspecified" OR NOT CMAKE_INSTALL_COMPONENT)
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/include/asterix" TYPE FILE FILES
    "/home/e/Development/asterix/src/asterix/AsterixData.h"
    "/home/e/Development/asterix/src/asterix/AsterixDefinition.h"
    "/home/e/Development/asterix/src/asterix/Category.h"
    "/home/e/Development/asterix/src/asterix/DataBlock.h"
    "/home/e/Development/asterix/src/asterix/DataItem.h"
    "/home/e/Development/asterix/src/asterix/DataItemBits.h"
    "/home/e/Development/asterix/src/asterix/DataItemDescription.h"
    "/home/e/Development/asterix/src/asterix/DataItemFormat.h"
    "/home/e/Development/asterix/src/asterix/DataItemFormatBDS.h"
    "/home/e/Development/asterix/src/asterix/DataItemFormatCompound.h"
    "/home/e/Development/asterix/src/asterix/DataItemFormatExplicit.h"
    "/home/e/Development/asterix/src/asterix/DataItemFormatFixed.h"
    "/home/e/Development/asterix/src/asterix/DataItemFormatRepetitive.h"
    "/home/e/Development/asterix/src/asterix/DataItemFormatVariable.h"
    "/home/e/Development/asterix/src/asterix/DataRecord.h"
    "/home/e/Development/asterix/src/asterix/InputParser.h"
    "/home/e/Development/asterix/src/asterix/Tracer.h"
    "/home/e/Development/asterix/src/asterix/UAP.h"
    "/home/e/Development/asterix/src/asterix/UAPItem.h"
    "/home/e/Development/asterix/src/asterix/Utils.h"
    "/home/e/Development/asterix/src/asterix/XMLParser.h"
    )
endif()

if(CMAKE_INSTALL_COMPONENT STREQUAL "Unspecified" OR NOT CMAKE_INSTALL_COMPONENT)
  if(EXISTS "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/bin/asterix" AND
     NOT IS_SYMLINK "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/bin/asterix")
    file(RPATH_CHECK
         FILE "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/bin/asterix"
         RPATH "")
  endif()
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/bin" TYPE EXECUTABLE FILES "/home/e/Development/asterix/bin/asterix")
  if(EXISTS "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/bin/asterix" AND
     NOT IS_SYMLINK "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/bin/asterix")
    file(RPATH_CHANGE
         FILE "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/bin/asterix"
         OLD_RPATH "/home/e/Development/asterix/lib:"
         NEW_RPATH "")
    if(CMAKE_INSTALL_DO_STRIP)
      execute_process(COMMAND "/usr/bin/strip" "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/bin/asterix")
    endif()
  endif()
endif()

if(CMAKE_INSTALL_COMPONENT STREQUAL "Unspecified" OR NOT CMAKE_INSTALL_COMPONENT)
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/share/asterix/config" TYPE DIRECTORY FILES "/home/e/Development/asterix/asterix/config/" FILES_MATCHING REGEX "/[^/]*\\.xml$" REGEX "/[^/]*\\.dtd$" REGEX "/[^/]*\\.ini$")
endif()

if(CMAKE_INSTALL_COMPONENT STREQUAL "Unspecified" OR NOT CMAKE_INSTALL_COMPONENT)
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/share/asterix/samples" TYPE DIRECTORY FILES "/home/e/Development/asterix/asterix/sample_data/" FILES_MATCHING REGEX "/[^/]*$")
endif()

if(CMAKE_INSTALL_COMPONENT STREQUAL "Unspecified" OR NOT CMAKE_INSTALL_COMPONENT)
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/share/doc/asterix" TYPE FILE FILES
    "/home/e/Development/asterix/README.md"
    "/home/e/Development/asterix/CLAUDE.md"
    "/home/e/Development/asterix/DO-278_IMPLEMENTATION_GUIDE.md"
    )
endif()

if(NOT CMAKE_INSTALL_LOCAL_ONLY)
  # Include the install script for the subdirectory.
  include("/home/e/Development/asterix/tests/cpp/cmake_install.cmake")
endif()

if(CMAKE_INSTALL_COMPONENT)
  set(CMAKE_INSTALL_MANIFEST "install_manifest_${CMAKE_INSTALL_COMPONENT}.txt")
else()
  set(CMAKE_INSTALL_MANIFEST "install_manifest.txt")
endif()

string(REPLACE ";" "\n" CMAKE_INSTALL_MANIFEST_CONTENT
       "${CMAKE_INSTALL_MANIFEST_FILES}")
file(WRITE "/home/e/Development/asterix/${CMAKE_INSTALL_MANIFEST}"
     "${CMAKE_INSTALL_MANIFEST_CONTENT}")
