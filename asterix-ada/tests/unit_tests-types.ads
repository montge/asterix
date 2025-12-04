--  unit_tests-types.ads
--  Unit tests for ASTERIX Ada type definitions
--
--  Copyright (c) 2024 ASTERIX Contributors
--  SPDX-License-Identifier: GPL-3.0-or-later

with AUnit.Test_Fixtures;

package Unit_Tests.Types is

   type Test_Case is new AUnit.Test_Fixtures.Test_Fixture with null record;

   procedure Test_Byte_Array_Valid (T : in out Test_Case);
   --  Test that Byte_Array holds valid byte values

   procedure Test_Byte_Array_Max_Size (T : in out Test_Case);
   --  Test Valid_Byte_Array subtype constraint

   procedure Test_Category_Number_Range (T : in out Test_Case);
   --  Test Category_Number range (1..255)

   procedure Test_Parse_Options_Defaults (T : in out Test_Case);
   --  Test Parse_Options default values

end Unit_Tests.Types;
