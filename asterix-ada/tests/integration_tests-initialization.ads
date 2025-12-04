--  integration_tests-initialization.ads
--  Integration tests for library initialization
--
--  Copyright (c) 2024 ASTERIX Contributors
--  SPDX-License-Identifier: GPL-3.0-or-later

with AUnit.Test_Fixtures;

package Integration_Tests.Initialization is

   type Test_Case is new AUnit.Test_Fixtures.Test_Fixture with null record;

   procedure Test_Init_Default (T : in out Test_Case);
   --  Test Init_Default initializes library

   procedure Test_Is_Initialized (T : in out Test_Case);
   --  Test Is_Initialized returns correct state

   procedure Test_Version (T : in out Test_Case);
   --  Test Version returns valid version string

end Integration_Tests.Initialization;
