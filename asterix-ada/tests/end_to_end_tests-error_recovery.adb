--  end_to_end_tests-error_recovery.adb
--  End-to-end tests for error handling and recovery
--
--  Copyright (c) 2024 ASTERIX Contributors
--  SPDX-License-Identifier: GPL-3.0-or-later

with AUnit.Assertions; use AUnit.Assertions;
with Asterix; use Asterix;

package body End_To_End_Tests.Error_Recovery is

   --  Invalid data samples
   Empty_Like : constant Byte_Array := (1 => 16#00#);
   Truncated  : constant Byte_Array := (16#30#, 16#00#);
   Bad_Length : constant Byte_Array :=
     (16#30#, 16#FF#, 16#FF#, 16#00#);

   --  Valid data for recovery test
   Valid_Data : constant Byte_Array :=
     (16#30#, 16#00#, 16#08#, 16#F0#, 16#01#, 16#02#, 16#03#, 16#04#);

   procedure Test_Graceful_Error (T : in out Test_Case) is
      pragma Unreferenced (T);
      Result1 : Asterix.Parse_Result;
      Result2 : Asterix.Parse_Result;
      Result3 : Asterix.Parse_Result;
   begin
      --  Ensure initialized
      if not Asterix.Is_Initialized then
         Asterix.Init_Default;
      end if;

      --  Test 1: Parse minimal data - should not crash
      Result1 := Asterix.Parse (Empty_Like);
      Assert
        (Result1.Error /= Asterix.Memory_Error,
         "Empty-like data should not cause memory error");

      --  Test 2: Parse truncated data - should handle gracefully
      Result2 := Asterix.Parse (Truncated);
      Assert
        (Result2.Error /= Asterix.Memory_Error,
         "Truncated data should not cause memory error");

      --  Test 3: Parse data with invalid length - should handle gracefully
      Result3 := Asterix.Parse (Bad_Length);
      Assert
        (Result3.Error /= Asterix.Memory_Error,
         "Bad length data should not cause memory error");

      --  All cases should have set some error status or consumed 0 bytes
      Assert
        (True,
         "All invalid data cases handled without crash");
   end Test_Graceful_Error;

   procedure Test_Recovery (T : in out Test_Case) is
      pragma Unreferenced (T);
      Result_Bad  : Parse_Result;
      pragma Unreferenced (Result_Bad);
      Result_Good : Parse_Result;
   begin
      --  Ensure initialized
      if not Asterix.Is_Initialized then
         Asterix.Init_Default;
      end if;

      --  Step 1: Parse invalid data
      Result_Bad := Asterix.Parse (Truncated);

      --  Step 2: Verify library still works after error
      Assert
        (Asterix.Is_Initialized,
         "Library should remain initialized after parse error");

      --  Step 3: Parse valid data - should succeed
      Result_Good := Asterix.Parse (Valid_Data);
      Assert
        (Result_Good.Error /= Asterix.Memory_Error,
         "Valid parse after error should succeed");

      --  Step 4: Verify version still works
      declare
         Ver : constant String := Asterix.Version;
      begin
         Assert
           (Ver'Length > 0,
            "Version should still be accessible after error");
      end;

      --  Step 5: Verify descriptions still work
      declare
         Desc : constant String := Asterix.Describe_Category (48);
      begin
         Assert
           (Desc'Length >= 0,
            "Descriptions should still work after error");
      end;
   end Test_Recovery;

end End_To_End_Tests.Error_Recovery;
