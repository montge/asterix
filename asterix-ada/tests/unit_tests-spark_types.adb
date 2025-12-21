--  unit_tests-spark_types.adb
--  Unit tests for ASTERIX.SPARK_Types inline functions
--
--  Copyright (c) 2024 ASTERIX Contributors
--  SPDX-License-Identifier: GPL-3.0-or-later

with AUnit.Assertions; use AUnit.Assertions;
with Asterix; use Asterix;
with Asterix.SPARK_Types; use Asterix.SPARK_Types;

package body Unit_Tests.SPARK_Types is

   procedure Test_Is_Common_Category (T : in out Test_Case) is
      pragma Unreferenced (T);
   begin
      --  Test common categories (should return True)
      Assert (Is_Common_Category (1), "Category 1 should be common");
      Assert (Is_Common_Category (2), "Category 2 should be common");
      Assert (Is_Common_Category (4), "Category 4 should be common");
      Assert (Is_Common_Category (8), "Category 8 should be common");
      Assert (Is_Common_Category (10), "Category 10 should be common");
      Assert (Is_Common_Category (19), "Category 19 should be common");
      Assert (Is_Common_Category (20), "Category 20 should be common");
      Assert (Is_Common_Category (21), "Category 21 should be common");
      Assert (Is_Common_Category (23), "Category 23 should be common");
      Assert (Is_Common_Category (34), "Category 34 should be common");
      Assert (Is_Common_Category (48), "Category 48 should be common");
      Assert (Is_Common_Category (62), "Category 62 should be common");
      Assert (Is_Common_Category (63), "Category 63 should be common");
      Assert (Is_Common_Category (65), "Category 65 should be common");
      Assert (Is_Common_Category (240), "Category 240 should be common");
      Assert (Is_Common_Category (247), "Category 247 should be common");

      --  Test non-common categories (should return False)
      Assert (not Is_Common_Category (3), "Category 3 should not be common");
      Assert (not Is_Common_Category (5), "Category 5 should not be common");
      Assert (not Is_Common_Category (100), "Category 100 should not be common");
      Assert (not Is_Common_Category (255), "Category 255 should not be common");
   end Test_Is_Common_Category;

   procedure Test_Is_Error (T : in out Test_Case) is
      pragma Unreferenced (T);
   begin
      --  OK should not be an error
      Assert (not Is_Error (OK), "OK should not be an error");

      --  All other error codes should be errors
      Assert (Is_Error (Init_Error), "Init_Error should be an error");
      Assert (Is_Error (Parse_Error), "Parse_Error should be an error");
      Assert (Is_Error (Invalid_Data), "Invalid_Data should be an error");
      Assert (Is_Error (Memory_Error), "Memory_Error should be an error");
      Assert (Is_Error (Not_Initialized), "Not_Initialized should be an error");
   end Test_Is_Error;

   procedure Test_Is_Initialization_Error (T : in out Test_Case) is
      pragma Unreferenced (T);
   begin
      --  Initialization errors
      Assert
        (Is_Initialization_Error (Init_Error),
         "Init_Error should be initialization error");
      Assert
        (Is_Initialization_Error (Not_Initialized),
         "Not_Initialized should be initialization error");

      --  Non-initialization errors
      Assert
        (not Is_Initialization_Error (OK),
         "OK should not be initialization error");
      Assert
        (not Is_Initialization_Error (Parse_Error),
         "Parse_Error should not be initialization error");
      Assert
        (not Is_Initialization_Error (Invalid_Data),
         "Invalid_Data should not be initialization error");
      Assert
        (not Is_Initialization_Error (Memory_Error),
         "Memory_Error should not be initialization error");
   end Test_Is_Initialization_Error;

   procedure Test_Is_Data_Error (T : in out Test_Case) is
      pragma Unreferenced (T);
   begin
      --  Data errors
      Assert
        (Is_Data_Error (Parse_Error),
         "Parse_Error should be data error");
      Assert
        (Is_Data_Error (Invalid_Data),
         "Invalid_Data should be data error");

      --  Non-data errors
      Assert
        (not Is_Data_Error (OK),
         "OK should not be data error");
      Assert
        (not Is_Data_Error (Init_Error),
         "Init_Error should not be data error");
      Assert
        (not Is_Data_Error (Not_Initialized),
         "Not_Initialized should not be data error");
      Assert
        (not Is_Data_Error (Memory_Error),
         "Memory_Error should not be data error");
   end Test_Is_Data_Error;

   procedure Test_Valid_Parse_Options (T : in out Test_Case) is
      pragma Unreferenced (T);
      Valid_Opts   : constant Parse_Options := (Verbose => True, Max_Blocks => 100, Offset => 0);
      Invalid_Opts_Offset : constant Parse_Options :=
        (Verbose => True, Max_Blocks => 0, Offset => Max_Message_Size);
      Invalid_Opts_Blocks : constant Parse_Options :=
        (Verbose => True, Max_Blocks => Max_Blocks + 1, Offset => 0);
   begin
      --  Test valid options
      Assert
        (Valid_Parse_Options (Valid_Opts),
         "Valid options should pass validation");

      Assert
        (Valid_Parse_Options ((Verbose => False, Max_Blocks => 0, Offset => 0)),
         "Default-like options should be valid");

      Assert
        (Valid_Parse_Options ((Verbose => True, Max_Blocks => Max_Blocks, Offset => 0)),
         "Max_Blocks at limit should be valid");

      Assert
        (Valid_Parse_Options ((Verbose => True, Max_Blocks => 0, Offset => Max_Message_Size - 1)),
         "Offset just below limit should be valid");

      --  Test invalid options
      Assert
        (not Valid_Parse_Options (Invalid_Opts_Offset),
         "Offset at Max_Message_Size should be invalid");

      Assert
        (not Valid_Parse_Options (Invalid_Opts_Blocks),
         "Max_Blocks above limit should be invalid");
   end Test_Valid_Parse_Options;

   procedure Test_Default_Parse_Options (T : in out Test_Case) is
      pragma Unreferenced (T);
      Opts : constant Parse_Options := Default_Parse_Options;
   begin
      --  Test default values
      Assert (Opts.Verbose = True, "Default Verbose should be True");
      Assert (Opts.Max_Blocks = 0, "Default Max_Blocks should be 0");
      Assert (Opts.Offset = 0, "Default Offset should be 0");

      --  Test that default options are valid (postcondition)
      Assert
        (Valid_Parse_Options (Opts),
         "Default options should be valid");
   end Test_Default_Parse_Options;

   procedure Test_To_Natural (T : in out Test_Case) is
      pragma Unreferenced (T);
      B_Min : constant Byte := 0;
      B_Max : constant Byte := 255;
      B_Mid : constant Byte := 128;
      B_Cat48 : constant Byte := 48;
   begin
      --  Test conversion at boundaries
      Assert
        (To_Natural (B_Min) = 0,
         "Byte 0 should convert to Natural 0");
      Assert
        (To_Natural (B_Max) = 255,
         "Byte 255 should convert to Natural 255");

      --  Test mid-range values
      Assert
        (To_Natural (B_Mid) = 128,
         "Byte 128 should convert to Natural 128");
      Assert
        (To_Natural (B_Cat48) = 48,
         "Byte 48 should convert to Natural 48");

      --  Test that conversion works (postcondition ensures range)
      declare
         Result : constant Natural := To_Natural (B_Max);
         pragma Unreferenced (Result);
      begin
         --  If we get here without constraint error, the postcondition passed
         Assert (True, "To_Natural postcondition verified");
      end;
   end Test_To_Natural;

   procedure Test_Safe_Length (T : in out Test_Case) is
      pragma Unreferenced (T);
   begin
      --  Test single element
      Assert
        (Safe_Length (1, 1) = 1,
         "Length of [1..1] should be 1");

      --  Test small range
      Assert
        (Safe_Length (1, 10) = 10,
         "Length of [1..10] should be 10");

      --  Test different starting indices
      Assert
        (Safe_Length (5, 15) = 11,
         "Length of [5..15] should be 11");

      Assert
        (Safe_Length (100, 200) = 101,
         "Length of [100..200] should be 101");

      --  Test at buffer limits
      Assert
        (Safe_Length (1, Max_Buffer_Size) = Max_Buffer_Size,
         "Length of [1..Max_Buffer_Size] should be Max_Buffer_Size");

      Assert
        (Safe_Length (Max_Buffer_Size, Max_Buffer_Size) = 1,
         "Length of [Max_Buffer_Size..Max_Buffer_Size] should be 1");

      --  Test that result is Buffer_Length type (type ensures valid range)
      declare
         Len : constant Buffer_Length := Safe_Length (1, 1000);
         pragma Unreferenced (Len);
      begin
         --  If we get here without constraint error, the type constraint passed
         Assert (True, "Safe_Length returns valid Buffer_Length");
      end;
   end Test_Safe_Length;

end Unit_Tests.SPARK_Types;
