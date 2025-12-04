--  asterix_test_suite.adb
--  Main test suite combining all test categories
--
--  Copyright (c) 2024 ASTERIX Contributors
--  SPDX-License-Identifier: GPL-3.0-or-later

with Unit_Tests;
with Integration_Tests;
with End_To_End_Tests;

package body Asterix_Test_Suite is

   Result : aliased AUnit.Test_Suites.Test_Suite;

   function Suite return AUnit.Test_Suites.Access_Test_Suite is
   begin
      --  Add unit tests
      AUnit.Test_Suites.Add_Test
        (Result'Access, Unit_Tests.Suite);

      --  Add integration tests
      AUnit.Test_Suites.Add_Test
        (Result'Access, Integration_Tests.Suite);

      --  Add end-to-end tests
      AUnit.Test_Suites.Add_Test
        (Result'Access, End_To_End_Tests.Suite);

      return Result'Access;
   end Suite;

end Asterix_Test_Suite;
