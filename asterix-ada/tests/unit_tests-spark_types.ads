--  unit_tests-spark_types.ads
--  Unit tests for ASTERIX.SPARK_Types inline functions
--
--  Copyright (c) 2024 ASTERIX Contributors
--  SPDX-License-Identifier: GPL-3.0-or-later

with AUnit.Test_Fixtures;

package Unit_Tests.SPARK_Types is

   type Test_Case is new AUnit.Test_Fixtures.Test_Fixture with null record;

   procedure Test_Is_Common_Category (T : in out Test_Case);
   --  Test Is_Common_Category with various category values

   procedure Test_Is_Error (T : in out Test_Case);
   --  Test Is_Error with OK and error codes

   procedure Test_Is_Initialization_Error (T : in out Test_Case);
   --  Test Is_Initialization_Error classification

   procedure Test_Is_Data_Error (T : in out Test_Case);
   --  Test Is_Data_Error classification

   procedure Test_Valid_Parse_Options (T : in out Test_Case);
   --  Test Valid_Parse_Options validation

   procedure Test_Default_Parse_Options (T : in out Test_Case);
   --  Test Default_Parse_Options returns valid options

   procedure Test_To_Natural (T : in out Test_Case);
   --  Test To_Natural conversion from Byte

   procedure Test_Safe_Length (T : in out Test_Case);
   --  Test Safe_Length buffer length calculation

end Unit_Tests.SPARK_Types;
