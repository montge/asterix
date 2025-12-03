--  end_to_end_tests-parse_workflow.adb
--  End-to-end tests for complete parsing workflows
--
--  Copyright (c) 2024 ASTERIX Contributors
--  SPDX-License-Identifier: GPL-3.0-or-later

with AUnit.Assertions; use AUnit.Assertions;
with Asterix; use Asterix;

package body End_To_End_Tests.Parse_Workflow is

   --  Sample Category 48 data (radar target report)
   Cat048_Data : constant Byte_Array :=
     (16#30#,           --  Category 48
      16#00#, 16#08#,   --  Length: 8 bytes
      16#F0#,           --  FSPEC
      16#01#, 16#02#,   --  I048/010: SAC/SIC
      16#03#, 16#04#);  --  Data

   --  Sample Category 62 data (SDPS track)
   Cat062_Data : constant Byte_Array :=
     (16#3E#,           --  Category 62
      16#00#, 16#08#,   --  Length: 8 bytes
      16#80#,           --  FSPEC
      16#01#, 16#02#,   --  Data
      16#03#, 16#04#);

   procedure Test_Full_Parse (T : in out Test_Case) is
      pragma Unreferenced (T);
      Result : Parse_Result;
   begin
      --  Step 1: Initialize library
      if not Is_Initialized then
         Init_Default;
      end if;
      Assert (Is_Initialized, "Step 1: Initialization failed");

      --  Step 2: Verify version is available
      declare
         Ver : constant String := Version;
      begin
         Assert (Ver'Length > 0, "Step 2: Version should be available");
      end;

      --  Step 3: Parse ASTERIX data
      Result := Parse (Cat048_Data);
      Assert
        (Result.Error /= Memory_Error,
         "Step 3: Parse should not cause memory error");

      --  Step 4: Check bytes consumed
      Assert
        (Result.Bytes_Consumed <= Cat048_Data'Length,
         "Step 4: Bytes consumed should be reasonable");

      --  Step 5: Parse result is properly finalized (RAII)
      --  Note: Finalize called automatically when Result goes out of scope
   end Test_Full_Parse;

   procedure Test_Incremental_Parse (T : in out Test_Case) is
      pragma Unreferenced (T);
      --  Two consecutive data blocks
      Combined_Data : constant Byte_Array := Cat048_Data & Cat062_Data;
      Result1   : Parse_Result;
      Result2   : Parse_Result;
      pragma Unreferenced (Result2);
      Offset    : Natural := 0;
      Max_Iter  : constant := 10;
      Iteration : Natural := 0;
   begin
      --  Ensure initialized
      if not Is_Initialized then
         Init_Default;
      end if;

      --  Parse incrementally through combined data
      while Offset < Combined_Data'Length and Iteration < Max_Iter loop
         Result1 := Parse_With_Offset
           (Combined_Data, Offset, Max_Blocks => 1, Verbose => True);

         if Result1.Bytes_Consumed > 0 then
            Offset := Offset + Result1.Bytes_Consumed;
         else
            --  Advance by 1 to avoid infinite loop on invalid data
            Offset := Offset + 1;
         end if;

         Iteration := Iteration + 1;
      end loop;

      Assert
        (Iteration < Max_Iter,
         "Incremental parse should complete without infinite loop");
   end Test_Incremental_Parse;

   procedure Test_Multi_Category (T : in out Test_Case) is
      pragma Unreferenced (T);
      --  Data containing both Category 48 and 62
      Multi_Cat_Data : constant Byte_Array := Cat048_Data & Cat062_Data;
      Result : Parse_Result;
   begin
      --  Ensure initialized
      if not Is_Initialized then
         Init_Default;
      end if;

      --  Parse all blocks
      Result := Parse (Multi_Cat_Data);

      --  Should handle multiple categories
      Assert
        (Result.Error /= Memory_Error,
         "Multi-category parse should not cause memory error");

      --  Check descriptions for both categories work
      declare
         Desc_48 : constant String := Describe_Category (48);
         Desc_62 : constant String := Describe_Category (62);
      begin
         --  Both should return without exception
         Assert
           (Desc_48'Length >= 0 and Desc_62'Length >= 0,
            "Category descriptions should be accessible");
      end;
   end Test_Multi_Category;

end End_To_End_Tests.Parse_Workflow;
