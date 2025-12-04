--  asterix-ffi.ads
--  Low-level FFI bindings to the C ASTERIX library
--
--  This package provides direct bindings to the C API defined in
--  src/go/asterix.h. Users should prefer the high-level Asterix package.
--
--  Copyright (c) 2024 ASTERIX Contributors
--  SPDX-License-Identifier: GPL-3.0-or-later

with Interfaces.C;         use Interfaces.C;
with Interfaces.C.Strings; use Interfaces.C.Strings;
with System;

package Asterix.FFI is

   pragma Preelaborate;

   --  Error codes from C API
   ASTERIX_OK         : constant int := 0;
   ASTERIX_ERR_INIT   : constant int := -1;
   ASTERIX_ERR_PARSE  : constant int := -2;
   ASTERIX_ERR_INVALID : constant int := -3;
   ASTERIX_ERR_MEMORY : constant int := -4;

   --  Maximum sizes for safety
   ASTERIX_MAX_MESSAGE_SIZE : constant := 65536;
   ASTERIX_MAX_PATH_LENGTH  : constant := 4096;
   ASTERIX_MAX_BLOCKS       : constant := 10000;

   --  Opaque parser handle
   type AsterixParser is limited private;
   type AsterixParser_Ptr is access all AsterixParser;
   pragma Convention (C, AsterixParser_Ptr);

   --  Record structure returned by parsing
   type AsterixRecord is record
      Category     : unsigned_char;
      Length       : unsigned_short;
      Timestamp_Us : unsigned_long;
      Json_Data    : chars_ptr;
      CRC          : unsigned;
   end record;
   pragma Convention (C, AsterixRecord);

   type AsterixRecord_Array is
     array (size_t range <>) of aliased AsterixRecord;
   pragma Convention (C, AsterixRecord_Array);

   type AsterixRecord_Ptr is access all AsterixRecord;
   pragma Convention (C, AsterixRecord_Ptr);

   --  Parse result structure
   type AsterixParseResult is record
      Records       : AsterixRecord_Ptr;
      Count         : size_t;
      Bytes_Consumed : size_t;
      Error_Code    : int;
      Error_Message : chars_ptr;
   end record;
   pragma Convention (C, AsterixParseResult);

   type AsterixParseResult_Ptr is access all AsterixParseResult;
   pragma Convention (C, AsterixParseResult_Ptr);

   --  Initialize the ASTERIX parser with configuration
   --  @param Config_Path Path to asterix.ini or XML config file
   --  @return ASTERIX_OK on success, error code on failure
   function Asterix_Init (Config_Path : chars_ptr) return int;
   pragma Import (C, Asterix_Init, "asterix_init");

   --  Check if parser is initialized
   --  @return 1 if initialized, 0 otherwise
   function Asterix_Is_Initialized return int;
   pragma Import (C, Asterix_Is_Initialized, "asterix_is_initialized");

   --  Parse ASTERIX data
   --  @param Data Raw ASTERIX binary data
   --  @param Length Length of data in bytes
   --  @param Verbose Include descriptions in output (1=yes, 0=no)
   --  @return Parse result (caller must free with asterix_free_result)
   function Asterix_Parse
     (Data    : System.Address;
      Length  : size_t;
      Verbose : int) return AsterixParseResult_Ptr;
   pragma Import (C, Asterix_Parse, "asterix_parse");

   --  Parse ASTERIX data with offset and block limit
   function Asterix_Parse_With_Offset
     (Data       : System.Address;
      Length     : size_t;
      Offset     : size_t;
      Max_Blocks : size_t;
      Verbose    : int) return AsterixParseResult_Ptr;
   pragma Import (C, Asterix_Parse_With_Offset, "asterix_parse_with_offset");

   --  Get description for ASTERIX category/item/field
   function Asterix_Describe
     (Category : int;
      Item     : chars_ptr;
      Field    : chars_ptr;
      Value    : chars_ptr) return chars_ptr;
   pragma Import (C, Asterix_Describe, "asterix_describe");

   --  Check if a category is defined
   function Asterix_Is_Category_Defined (Category : int) return int;
   pragma Import (C, Asterix_Is_Category_Defined,
                  "asterix_is_category_defined");

   --  Free parse result
   procedure Asterix_Free_Result (Result : AsterixParseResult_Ptr);
   pragma Import (C, Asterix_Free_Result, "asterix_free_result");

   --  Free string returned by asterix_describe
   procedure Asterix_Free_String (Str : chars_ptr);
   pragma Import (C, Asterix_Free_String, "asterix_free_string");

   --  Get library version
   function Asterix_Version return chars_ptr;
   pragma Import (C, Asterix_Version, "asterix_version");

   --  Get last error message
   function Asterix_Get_Last_Error return chars_ptr;
   pragma Import (C, Asterix_Get_Last_Error, "asterix_get_last_error");

private

   type AsterixParser is null record;

end Asterix.FFI;
