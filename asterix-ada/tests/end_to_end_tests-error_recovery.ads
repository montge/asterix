--  end_to_end_tests-error_recovery.ads
--  End-to-end tests for error handling and recovery
--
--  Copyright (c) 2024 ASTERIX Contributors
--  SPDX-License-Identifier: GPL-3.0-or-later

with AUnit.Test_Fixtures;

package End_To_End_Tests.Error_Recovery is

   type Test_Case is new AUnit.Test_Fixtures.Test_Fixture with null record;

   procedure Test_Graceful_Error (T : in out Test_Case);
   --  Test graceful handling of invalid data

   procedure Test_Recovery (T : in out Test_Case);
   --  Test recovery after parse error allows continued operation

end End_To_End_Tests.Error_Recovery;
