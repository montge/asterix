--  integration_tests.ads
--  Integration tests for ASTERIX Ada bindings
--
--  Tests interaction between Ada API and C++ library via FFI.
--
--  Copyright (c) 2024 ASTERIX Contributors
--  SPDX-License-Identifier: GPL-3.0-or-later

with AUnit.Test_Suites;

package Integration_Tests is

   function Suite return AUnit.Test_Suites.Access_Test_Suite;

end Integration_Tests;
