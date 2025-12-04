--  end_to_end_tests.ads
--  End-to-end tests for ASTERIX Ada bindings
--
--  Tests complete workflows similar to parse_example.adb.
--
--  Copyright (c) 2024 ASTERIX Contributors
--  SPDX-License-Identifier: GPL-3.0-or-later

with AUnit.Test_Suites;

package End_To_End_Tests is

   function Suite return AUnit.Test_Suites.Access_Test_Suite;

end End_To_End_Tests;
