--  asterix.ads
--  High-level Ada API for ASTERIX ATM surveillance protocol decoder
--
--  This package provides a safe, idiomatic Ada interface to the ASTERIX
--  decoder library. It wraps the low-level FFI bindings with proper
--  resource management and Ada types.
--
--  SPARK Status: Partial verification possible
--    - Public API contracts are SPARK-compatible
--    - FFI calls in body are excluded from SPARK analysis
--    - See SPARK_GUIDE.md for verification instructions
--
--  Example usage:
--
--     with Asterix;
--     with Ada.Text_IO;
--
--     procedure Example is
--        Data   : Asterix.Byte_Array := (16#30#, 16#00#, 16#05#, ...);
--        Result : Asterix.Parse_Result;
--     begin
--        Asterix.Init_Default;
--        Result := Asterix.Parse (Data);
--        for Rec of Result.Records loop
--           Ada.Text_IO.Put_Line ("Category:" & Rec.Category'Image);
--        end loop;
--     end Example;
--
--  Copyright (c) 2024 ASTERIX Contributors
--  SPDX-License-Identifier: GPL-3.0-or-later

with Ada.Finalization;
with Ada.Strings.Unbounded; use Ada.Strings.Unbounded;
with Interfaces;

package Asterix is

   pragma Preelaborate;

   ---------------------------------------------------------------------------
   --  Constants for safety bounds
   ---------------------------------------------------------------------------

   Max_Message_Size : constant := 65536;
   --  Maximum ASTERIX message size in bytes

   Max_Path_Length : constant := 4096;
   --  Maximum configuration path length

   Max_Blocks : constant := 10000;
   --  Maximum number of data blocks to parse

   ---------------------------------------------------------------------------
   --  Basic types
   ---------------------------------------------------------------------------

   type Byte is new Interfaces.Unsigned_8;
   type Byte_Array is array (Positive range <>) of Byte;

   subtype Valid_Byte_Array is Byte_Array
     with Dynamic_Predicate => Valid_Byte_Array'Length <= Max_Message_Size;
   --  Constrained byte array for safe parsing

   type Category_Number is range 1 .. 255;
   --  ASTERIX category number (1-255)

   type Timestamp_Microseconds is new Interfaces.Unsigned_64;
   --  Timestamp in microseconds since epoch

   ---------------------------------------------------------------------------
   --  Error handling
   ---------------------------------------------------------------------------

   type Error_Code is
     (OK,
      Init_Error,
      Parse_Error,
      Invalid_Data,
      Memory_Error,
      Not_Initialized);
   --  Result codes from parsing operations

   type Asterix_Exception is new Ada.Finalization.Controlled with record
      Code    : Error_Code;
      Message : Unbounded_String;
   end record;

   ---------------------------------------------------------------------------
   --  Parsed ASTERIX record
   ---------------------------------------------------------------------------

   type Asterix_Record is record
      Category  : Category_Number;
      Length    : Natural;
      Timestamp : Timestamp_Microseconds;
      Json_Data : Unbounded_String;
      CRC       : Interfaces.Unsigned_32;
   end record;

   type Record_Array is array (Positive range <>) of Asterix_Record;
   type Record_Array_Access is access Record_Array;

   ---------------------------------------------------------------------------
   --  Parse result with automatic cleanup
   ---------------------------------------------------------------------------

   type Parse_Result is new Ada.Finalization.Controlled with record
      Records        : Record_Array_Access;
      Bytes_Consumed : Natural;
      Error          : Error_Code;
      Error_Message  : Unbounded_String;
   end record;

   overriding procedure Finalize (Self : in out Parse_Result);

   ---------------------------------------------------------------------------
   --  Parse options
   ---------------------------------------------------------------------------

   type Parse_Options is record
      Verbose    : Boolean := True;    --  Include descriptions
      Max_Blocks : Natural := 0;       --  0 = parse all blocks
      Offset     : Natural := 0;       --  Start offset in bytes
   end record;

   Default_Options : constant Parse_Options := (others => <>);

   ---------------------------------------------------------------------------
   --  Initialization functions
   ---------------------------------------------------------------------------

   procedure Init_Default
     with Post => Is_Initialized;
   --  Initialize with default configuration (searches standard paths)
   --  Raises Program_Error if initialization fails

   procedure Init (Config_Path : String)
     with
       Pre  => Config_Path'Length > 0 and
               Config_Path'Length <= Max_Path_Length,
       Post => Is_Initialized;
   --  Initialize with custom configuration file
   --  Raises Program_Error if initialization fails

   function Is_Initialized return Boolean;
   --  Check if parser is initialized

   ---------------------------------------------------------------------------
   --  Parsing functions
   ---------------------------------------------------------------------------

   function Parse (Data : Byte_Array) return Parse_Result
     with Pre => Data'Length > 0 and Data'Length <= Max_Message_Size;
   --  Parse ASTERIX data with default options

   function Parse
     (Data    : Byte_Array;
      Options : Parse_Options) return Parse_Result
     with Pre => Data'Length > 0 and Data'Length <= Max_Message_Size;
   --  Parse ASTERIX data with custom options

   function Parse_With_Offset
     (Data       : Byte_Array;
      Offset     : Natural;
      Max_Blocks : Natural := 0;
      Verbose    : Boolean := True) return Parse_Result
     with Pre => Data'Length > 0 and
                 Data'Length <= Max_Message_Size and
                 Offset < Data'Length;
   --  Parse with offset (for incremental parsing)

   ---------------------------------------------------------------------------
   --  Description functions
   ---------------------------------------------------------------------------

   function Describe_Category (Category : Category_Number) return String;
   --  Get description for a category

   function Describe_Item
     (Category : Category_Number;
      Item     : String) return String
     with Pre => Item'Length > 0;
   --  Get description for an item in a category

   function Describe_Value
     (Category : Category_Number;
      Item     : String;
      Field    : String;
      Value    : String) return String
     with Pre => Item'Length > 0 and Field'Length > 0;
   --  Get description for a field value

   ---------------------------------------------------------------------------
   --  Utility functions
   ---------------------------------------------------------------------------

   function Is_Category_Defined (Category : Category_Number) return Boolean;
   --  Check if a category is defined

   function Version return String;
   --  Get library version

   function Get_Last_Error return String;
   --  Get last error message (if any)

private

   Initialized : Boolean := False;

end Asterix;
