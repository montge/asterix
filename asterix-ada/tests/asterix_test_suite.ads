--  asterix_test_suite.ads
--  Main test suite combining all test categories
--
--  Copyright (c) 2024 ASTERIX Contributors
--  SPDX-License-Identifier: GPL-3.0-or-later

with AUnit.Test_Suites;

package Asterix_Test_Suite is

   function Suite return AUnit.Test_Suites.Access_Test_Suite;

end Asterix_Test_Suite;
