--  end_to_end_tests.adb
--  End-to-end tests for ASTERIX Ada bindings
--
--  Copyright (c) 2024 ASTERIX Contributors
--  SPDX-License-Identifier: GPL-3.0-or-later

with AUnit.Test_Caller;
with End_To_End_Tests.Parse_Workflow;
with End_To_End_Tests.Error_Recovery;

package body End_To_End_Tests is

   package Workflow_Caller is new AUnit.Test_Caller
     (End_To_End_Tests.Parse_Workflow.Test_Case);

   package Error_Caller is new AUnit.Test_Caller
     (End_To_End_Tests.Error_Recovery.Test_Case);

   Result : aliased AUnit.Test_Suites.Test_Suite;

   function Suite return AUnit.Test_Suites.Access_Test_Suite is
   begin
      --  Complete parse workflow tests
      AUnit.Test_Suites.Add_Test
        (Result'Access,
         Workflow_Caller.Create
           ("Test_Full_Parse_Workflow",
            End_To_End_Tests.Parse_Workflow.Test_Full_Parse'Access));

      AUnit.Test_Suites.Add_Test
        (Result'Access,
         Workflow_Caller.Create
           ("Test_Incremental_Parse",
            End_To_End_Tests.Parse_Workflow.Test_Incremental_Parse'Access));

      AUnit.Test_Suites.Add_Test
        (Result'Access,
         Workflow_Caller.Create
           ("Test_Multiple_Categories",
            End_To_End_Tests.Parse_Workflow.Test_Multi_Category'Access));

      --  Error recovery tests
      AUnit.Test_Suites.Add_Test
        (Result'Access,
         Error_Caller.Create
           ("Test_Graceful_Error_Handling",
            End_To_End_Tests.Error_Recovery.Test_Graceful_Error'Access));

      AUnit.Test_Suites.Add_Test
        (Result'Access,
         Error_Caller.Create
           ("Test_Recovery_After_Error",
            End_To_End_Tests.Error_Recovery.Test_Recovery'Access));

      return Result'Access;
   end Suite;

end End_To_End_Tests;
