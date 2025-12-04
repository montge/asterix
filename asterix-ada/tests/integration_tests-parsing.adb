--  integration_tests-parsing.adb
--  Integration tests for ASTERIX parsing via FFI
--
--  Copyright (c) 2024 ASTERIX Contributors
--  SPDX-License-Identifier: GPL-3.0-or-later

with AUnit.Assertions; use AUnit.Assertions;
with Asterix; use Asterix;

package body Integration_Tests.Parsing is

   --  Sample Category 48 data block (minimal valid structure)
   --  Byte 0: Category (48 = 0x30)
   --  Bytes 1-2: Length (big-endian)
   --  Bytes 3+: FSPEC and data items
   Cat048_Sample : constant Byte_Array :=
     (16#30#,        --  Category 48
      16#00#, 16#08#, --  Length: 8 bytes
      16#F0#,        --  FSPEC: I048/010, /140, /020, /040 present
      16#01#, 16#02#, --  I048/010: SAC/SIC
      16#03#, 16#04#); --  Minimal data

   procedure Test_Parse_Cat048 (T : in out Test_Case) is
      pragma Unreferenced (T);
      Result : Parse_Result;
   begin
      --  Ensure initialized
      if not Is_Initialized then
         Init_Default;
      end if;

      --  Parse sample data
      Result := Parse (Cat048_Sample);

      --  Check that parsing did not return memory error
      Assert
        (Result.Error /= Memory_Error,
         "Parse should not return memory error");

      --  Check bytes consumed is reasonable
      Assert
        (Result.Bytes_Consumed <= Cat048_Sample'Length,
         "Bytes consumed should not exceed input length");
   end Test_Parse_Cat048;

   procedure Test_Parse_Invalid_Data (T : in out Test_Case) is
      pragma Unreferenced (T);
      --  Invalid data: too short to be valid ASTERIX
      Invalid_Data : constant Byte_Array := (16#00#, 16#00#);
      Result       : Parse_Result;
   begin
      --  Ensure initialized
      if not Is_Initialized then
         Init_Default;
      end if;

      --  Parse invalid data
      Result := Parse (Invalid_Data);

      --  Should either return error or consume 0 bytes
      Assert
        (Result.Error /= OK or Result.Bytes_Consumed = 0,
         "Invalid data should fail or consume 0 bytes");
   end Test_Parse_Invalid_Data;

   procedure Test_Parse_With_Options (T : in out Test_Case) is
      pragma Unreferenced (T);
      Options : Parse_Options;
      Result  : Parse_Result;
   begin
      --  Ensure initialized
      if not Is_Initialized then
         Init_Default;
      end if;

      --  Set custom options
      Options.Verbose := False;
      Options.Max_Blocks := 1;
      Options.Offset := 0;

      --  Parse with options
      Result := Parse (Cat048_Sample, Options);

      --  Check result is reasonable
      Assert
        (Result.Bytes_Consumed <= Cat048_Sample'Length,
         "Bytes consumed should not exceed input length with options");
   end Test_Parse_With_Options;

end Integration_Tests.Parsing;
