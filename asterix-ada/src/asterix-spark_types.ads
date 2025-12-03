--  asterix-spark_types.ads
--  SPARK-verified type definitions for safety-critical use
--
--  This package contains types and operations that are fully verified
--  using SPARK formal methods. All types in this package have:
--    - Proven absence of runtime errors
--    - Verified preconditions and postconditions
--    - Guaranteed bounded ranges
--
--  For safety-critical aviation applications (DO-178C/DO-278A), use types
--  from this package where formal verification is required.
--
--  Copyright (c) 2024 ASTERIX Contributors
--  SPDX-License-Identifier: GPL-3.0-or-later

with Asterix;

package Asterix.SPARK_Types
  with SPARK_Mode => On
is

   ---------------------------------------------------------------------------
   --  Bounded message buffer type (proven safe)
   ---------------------------------------------------------------------------

   Max_Buffer_Size : constant := Max_Message_Size;
   --  Maximum buffer size (inherits from parent)

   subtype Buffer_Index is Positive range 1 .. Max_Buffer_Size;
   --  Valid index range for message buffers

   subtype Buffer_Length is Natural range 0 .. Max_Buffer_Size;
   --  Valid length for message buffers

   type Safe_Buffer is array (Buffer_Index range <>) of Byte
     with Predicate => Safe_Buffer'Length <= Max_Buffer_Size;
   --  Buffer type with proven bounds

   ---------------------------------------------------------------------------
   --  Category range types (proven valid)
   ---------------------------------------------------------------------------

   subtype Valid_Category is Category_Number;
   --  Category number (range 1..255 already proven in parent)

   function Is_Common_Category (Cat : Category_Number) return Boolean is
     (Cat in 1 | 2 | 4 | 8 | 10 | 19 | 20 | 21 | 23 | 34 | 48 | 62 | 63 |
           65 | 240 | 247)
     with Inline;
   --  Check if category is a commonly-used ASTERIX category

   ---------------------------------------------------------------------------
   --  Error code utilities (proven correct)
   ---------------------------------------------------------------------------

   function Is_Error (Code : Error_Code) return Boolean is
     (Code /= OK)
     with Inline,
          Post => Is_Error'Result = (Code /= OK);
   --  Check if error code indicates an error

   function Is_Initialization_Error (Code : Error_Code) return Boolean is
     (Code in Init_Error | Not_Initialized)
     with Inline;
   --  Check if error relates to initialization

   function Is_Data_Error (Code : Error_Code) return Boolean is
     (Code in Parse_Error | Invalid_Data)
     with Inline;
   --  Check if error relates to data processing

   ---------------------------------------------------------------------------
   --  Parse option validation (proven correct)
   ---------------------------------------------------------------------------

   function Valid_Parse_Options (Opts : Parse_Options) return Boolean is
     (Opts.Offset < Max_Message_Size and
      Opts.Max_Blocks <= Max_Blocks)
     with Inline;
   --  Check if parse options are valid

   function Default_Parse_Options return Parse_Options is
     (Verbose => True, Max_Blocks => 0, Offset => 0)
     with Post => Valid_Parse_Options (Default_Parse_Options'Result);
   --  Return default parse options (proven valid)

   ---------------------------------------------------------------------------
   --  Safe conversion utilities (proven no overflow)
   ---------------------------------------------------------------------------

   function To_Natural (B : Byte) return Natural is
     (Natural (B))
     with Inline,
          Post => To_Natural'Result in 0 .. 255;
   --  Convert byte to natural (proven in range)

   function To_Byte (N : Natural) return Byte
     with Pre => N <= 255,
          Post => Natural (To_Byte'Result) = N;
   --  Convert natural to byte (precondition ensures no overflow)

   ---------------------------------------------------------------------------
   --  Buffer length calculation (proven safe)
   ---------------------------------------------------------------------------

   function Safe_Length
     (First : Buffer_Index;
      Last  : Buffer_Index) return Buffer_Length
     with Pre => First <= Last,
          Post => Safe_Length'Result = Last - First + 1;
   --  Calculate buffer length safely

end Asterix.SPARK_Types;
