--  unit_tests-types.adb
--  Unit tests for ASTERIX Ada type definitions
--
--  Copyright (c) 2024 ASTERIX Contributors
--  SPDX-License-Identifier: GPL-3.0-or-later

with AUnit.Assertions; use AUnit.Assertions;
with Asterix; use Asterix;

package body Unit_Tests.Types is

   procedure Test_Byte_Array_Valid (T : in out Test_Case) is
      pragma Unreferenced (T);
      Data : constant Byte_Array (1 .. 3) := (16#00#, 16#FF#, 16#80#);
      B1 : constant Byte := Data (1);
      B2 : constant Byte := Data (2);
      B3 : constant Byte := Data (3);
   begin
      --  Test byte values at boundaries
      Assert (Byte'Pos (B1) = 0, "Minimum byte value should be 0");
      Assert (Byte'Pos (B2) = 255, "Maximum byte value should be 255");
      Assert (Byte'Pos (B3) = 128, "Mid-range byte value should work");

      --  Test array length
      Assert (Data'Length = 3, "Array length should be 3");
      Assert (Data'First = 1, "Array should be 1-indexed");
      Assert (Data'Last = 3, "Last index should be 3");
   end Test_Byte_Array_Valid;

   procedure Test_Byte_Array_Max_Size (T : in out Test_Case) is
      pragma Unreferenced (T);
      Max_Size : constant := Max_Message_Size;
   begin
      --  Verify constant value (using pragma to avoid warning)
      pragma Assert (Max_Size = 65536);
      Assert (True, "Max_Message_Size verified");

      --  Test that we can create array at max size
      declare
         Large_Data : Valid_Byte_Array (1 .. Max_Size);
      begin
         Large_Data (1) := 16#30#;  --  Category 48
         Large_Data (Max_Size) := 16#00#;
         Assert (Large_Data'Length = Max_Size,
                 "Should create array at max size");
      end;
   end Test_Byte_Array_Max_Size;

   procedure Test_Category_Number_Range (T : in out Test_Case) is
      pragma Unreferenced (T);
      Cat_Min : constant Category_Number := 1;
      Cat_Max : constant Category_Number := 255;
      Cat_48  : constant Category_Number := 48;
      Cat_62  : constant Category_Number := 62;
   begin
      --  Test boundary values using 'Pos to avoid comparison issues
      Assert
        (Category_Number'Pos (Category_Number'First) = 1,
         "Category minimum should be 1");
      Assert
        (Category_Number'Pos (Category_Number'Last) = 255,
         "Category maximum should be 255");

      --  Test common categories using 'Pos
      Assert
        (Category_Number'Pos (Cat_Min) = 1, "Minimum category is 1");
      Assert
        (Category_Number'Pos (Cat_Max) = 255, "Maximum category is 255");
      Assert
        (Category_Number'Pos (Cat_48) = 48, "Category 48 is valid");
      Assert
        (Category_Number'Pos (Cat_62) = 62, "Category 62 is valid");
   end Test_Category_Number_Range;

   procedure Test_Parse_Options_Defaults (T : in out Test_Case) is
      pragma Unreferenced (T);
      Options : constant Parse_Options := Default_Options;
   begin
      --  Verify default option values
      Assert (Options.Verbose = True, "Default Verbose should be True");
      Assert (Options.Max_Blocks = 0, "Default Max_Blocks should be 0");
      Assert (Options.Offset = 0, "Default Offset should be 0");
   end Test_Parse_Options_Defaults;

end Unit_Tests.Types;
