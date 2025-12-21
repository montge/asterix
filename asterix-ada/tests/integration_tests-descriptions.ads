--  integration_tests-descriptions.ads
--  Integration tests for ASTERIX description functions
--
--  Copyright (c) 2024 ASTERIX Contributors
--  SPDX-License-Identifier: GPL-3.0-or-later

with AUnit.Test_Fixtures;

package Integration_Tests.Descriptions is

   type Test_Case is new AUnit.Test_Fixtures.Test_Fixture with null record;

   procedure Test_Describe_Category (T : in out Test_Case);
   --  Test Describe_Category returns valid descriptions

   procedure Test_Is_Category_Defined (T : in out Test_Case);
   --  Test Is_Category_Defined for known categories

   procedure Test_Describe_Item (T : in out Test_Case);
   --  Test Describe_Item returns valid descriptions for known items

   procedure Test_Describe_Item_Invalid (T : in out Test_Case);
   --  Test Describe_Item with invalid item names

   procedure Test_Describe_Value (T : in out Test_Case);
   --  Test Describe_Value returns descriptions for field values

   procedure Test_Describe_Value_Invalid (T : in out Test_Case);
   --  Test Describe_Value with invalid parameters

end Integration_Tests.Descriptions;
