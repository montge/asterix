--  integration_tests-parsing.ads
--  Integration tests for ASTERIX parsing via FFI
--
--  Copyright (c) 2024 ASTERIX Contributors
--  SPDX-License-Identifier: GPL-3.0-or-later

with AUnit.Test_Fixtures;

package Integration_Tests.Parsing is

   type Test_Case is new AUnit.Test_Fixtures.Test_Fixture with null record;

   procedure Test_Parse_Cat048 (T : in out Test_Case);
   --  Test parsing valid Category 48 data

   procedure Test_Parse_Invalid_Data (T : in out Test_Case);
   --  Test parsing invalid data returns error

   procedure Test_Parse_With_Options (T : in out Test_Case);
   --  Test parsing with custom options

end Integration_Tests.Parsing;
