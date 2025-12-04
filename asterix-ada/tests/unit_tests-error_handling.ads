--  unit_tests-error_handling.ads
--  Unit tests for ASTERIX Ada error handling
--
--  Copyright (c) 2024 ASTERIX Contributors
--  SPDX-License-Identifier: GPL-3.0-or-later

with AUnit.Test_Fixtures;

package Unit_Tests.Error_Handling is

   type Test_Case is new AUnit.Test_Fixtures.Test_Fixture with null record;

   procedure Test_Error_Code_Values (T : in out Test_Case);
   --  Test all Error_Code enumeration values

   procedure Test_Error_Code_Image (T : in out Test_Case);
   --  Test Error_Code_Image function returns correct strings

end Unit_Tests.Error_Handling;
