--  unit_tests.ads
--  Unit tests for ASTERIX Ada bindings
--
--  Tests individual functions in isolation with mock/stub dependencies.
--
--  Copyright (c) 2024 ASTERIX Contributors
--  SPDX-License-Identifier: GPL-3.0-or-later

with AUnit.Test_Suites;

package Unit_Tests is

   function Suite return AUnit.Test_Suites.Access_Test_Suite;

end Unit_Tests;
