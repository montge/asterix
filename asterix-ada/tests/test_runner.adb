--  test_runner.adb
--  Main test runner for ASTERIX Ada test suite
--
--  This runs all unit, integration, and end-to-end tests using AUnit.
--
--  Usage:
--    ./bin/test_runner           -- Run all tests
--    ./bin/test_runner --help    -- Show options
--
--  Copyright (c) 2024 ASTERIX Contributors
--  SPDX-License-Identifier: GPL-3.0-or-later

with AUnit.Run;
with AUnit.Reporter.Text;

with Asterix_Test_Suite;

procedure Test_Runner is
   procedure Run is new AUnit.Run.Test_Runner (Asterix_Test_Suite.Suite);
   Reporter : AUnit.Reporter.Text.Text_Reporter;
begin
   Run (Reporter);
end Test_Runner;
