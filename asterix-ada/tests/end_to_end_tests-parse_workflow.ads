--  end_to_end_tests-parse_workflow.ads
--  End-to-end tests for complete parsing workflows
--
--  Copyright (c) 2024 ASTERIX Contributors
--  SPDX-License-Identifier: GPL-3.0-or-later

with AUnit.Test_Fixtures;

package End_To_End_Tests.Parse_Workflow is

   type Test_Case is new AUnit.Test_Fixtures.Test_Fixture with null record;

   procedure Test_Full_Parse (T : in out Test_Case);
   --  Test complete init -> parse -> access results workflow

   procedure Test_Incremental_Parse (T : in out Test_Case);
   --  Test incremental parsing with offset

   procedure Test_Multi_Category (T : in out Test_Case);
   --  Test parsing data with multiple ASTERIX categories

end End_To_End_Tests.Parse_Workflow;
