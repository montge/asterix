--  unit_tests.adb
--  Unit tests for ASTERIX Ada bindings
--
--  Copyright (c) 2024 ASTERIX Contributors
--  SPDX-License-Identifier: GPL-3.0-or-later

with AUnit.Test_Caller;
with Unit_Tests.Types;
with Unit_Tests.Error_Handling;

package body Unit_Tests is

   package Types_Caller is new AUnit.Test_Caller
     (Unit_Tests.Types.Test_Case);

   package Error_Caller is new AUnit.Test_Caller
     (Unit_Tests.Error_Handling.Test_Case);

   Result : aliased AUnit.Test_Suites.Test_Suite;

   function Suite return AUnit.Test_Suites.Access_Test_Suite is
   begin
      --  Type validation tests
      AUnit.Test_Suites.Add_Test
        (Result'Access,
         Types_Caller.Create
           ("Test_Byte_Array_Valid",
            Unit_Tests.Types.Test_Byte_Array_Valid'Access));

      AUnit.Test_Suites.Add_Test
        (Result'Access,
         Types_Caller.Create
           ("Test_Byte_Array_Max_Size",
            Unit_Tests.Types.Test_Byte_Array_Max_Size'Access));

      AUnit.Test_Suites.Add_Test
        (Result'Access,
         Types_Caller.Create
           ("Test_Category_Number_Range",
            Unit_Tests.Types.Test_Category_Number_Range'Access));

      AUnit.Test_Suites.Add_Test
        (Result'Access,
         Types_Caller.Create
           ("Test_Parse_Options_Defaults",
            Unit_Tests.Types.Test_Parse_Options_Defaults'Access));

      --  Error handling tests
      AUnit.Test_Suites.Add_Test
        (Result'Access,
         Error_Caller.Create
           ("Test_Error_Code_Values",
            Unit_Tests.Error_Handling.Test_Error_Code_Values'Access));

      AUnit.Test_Suites.Add_Test
        (Result'Access,
         Error_Caller.Create
           ("Test_Error_Code_Image",
            Unit_Tests.Error_Handling.Test_Error_Code_Image'Access));

      return Result'Access;
   end Suite;

end Unit_Tests;
