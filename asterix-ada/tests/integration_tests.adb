--  integration_tests.adb
--  Integration tests for ASTERIX Ada bindings
--
--  Copyright (c) 2024 ASTERIX Contributors
--  SPDX-License-Identifier: GPL-3.0-or-later

with AUnit.Test_Caller;
with Integration_Tests.Initialization;
with Integration_Tests.Parsing;
with Integration_Tests.Descriptions;

package body Integration_Tests is

   package Init_Caller is new AUnit.Test_Caller
     (Integration_Tests.Initialization.Test_Case);

   package Parse_Caller is new AUnit.Test_Caller
     (Integration_Tests.Parsing.Test_Case);

   package Desc_Caller is new AUnit.Test_Caller
     (Integration_Tests.Descriptions.Test_Case);

   Result : aliased AUnit.Test_Suites.Test_Suite;

   function Suite return AUnit.Test_Suites.Access_Test_Suite is
   begin
      --  Initialization tests
      AUnit.Test_Suites.Add_Test
        (Result'Access,
         Init_Caller.Create
           ("Test_Init_Default",
            Integration_Tests.Initialization.Test_Init_Default'Access));

      AUnit.Test_Suites.Add_Test
        (Result'Access,
         Init_Caller.Create
           ("Test_Is_Initialized",
            Integration_Tests.Initialization.Test_Is_Initialized'Access));

      AUnit.Test_Suites.Add_Test
        (Result'Access,
         Init_Caller.Create
           ("Test_Version",
            Integration_Tests.Initialization.Test_Version'Access));

      --  Parsing tests
      AUnit.Test_Suites.Add_Test
        (Result'Access,
         Parse_Caller.Create
           ("Test_Parse_Cat048",
            Integration_Tests.Parsing.Test_Parse_Cat048'Access));

      AUnit.Test_Suites.Add_Test
        (Result'Access,
         Parse_Caller.Create
           ("Test_Parse_Invalid_Data",
            Integration_Tests.Parsing.Test_Parse_Invalid_Data'Access));

      AUnit.Test_Suites.Add_Test
        (Result'Access,
         Parse_Caller.Create
           ("Test_Parse_With_Options",
            Integration_Tests.Parsing.Test_Parse_With_Options'Access));

      --  Description tests
      AUnit.Test_Suites.Add_Test
        (Result'Access,
         Desc_Caller.Create
           ("Test_Describe_Category",
            Integration_Tests.Descriptions.Test_Describe_Category'Access));

      AUnit.Test_Suites.Add_Test
        (Result'Access,
         Desc_Caller.Create
           ("Test_Is_Category_Defined",
            Integration_Tests.Descriptions.Test_Is_Category_Defined'Access));

      AUnit.Test_Suites.Add_Test
        (Result'Access,
         Desc_Caller.Create
           ("Test_Describe_Item",
            Integration_Tests.Descriptions.Test_Describe_Item'Access));

      AUnit.Test_Suites.Add_Test
        (Result'Access,
         Desc_Caller.Create
           ("Test_Describe_Item_Invalid",
            Integration_Tests.Descriptions.Test_Describe_Item_Invalid'Access));

      AUnit.Test_Suites.Add_Test
        (Result'Access,
         Desc_Caller.Create
           ("Test_Describe_Value",
            Integration_Tests.Descriptions.Test_Describe_Value'Access));

      AUnit.Test_Suites.Add_Test
        (Result'Access,
         Desc_Caller.Create
           ("Test_Describe_Value_Invalid",
            Integration_Tests.Descriptions
              .Test_Describe_Value_Invalid'Access));

      return Result'Access;
   end Suite;

end Integration_Tests;
