--  unit_tests-error_handling.adb
--  Unit tests for ASTERIX Ada error handling
--
--  Copyright (c) 2024 ASTERIX Contributors
--  SPDX-License-Identifier: GPL-3.0-or-later

with AUnit.Assertions; use AUnit.Assertions;
with Asterix;
with Asterix.Errors;

package body Unit_Tests.Error_Handling is

   procedure Test_Error_Code_Values (T : in out Test_Case) is
      pragma Unreferenced (T);
      use Asterix;
      E_OK   : constant Error_Code := OK;
      E_Init : constant Error_Code := Init_Error;
      E_Parse : constant Error_Code := Parse_Error;
      E_Data : constant Error_Code := Invalid_Data;
      E_Mem  : constant Error_Code := Memory_Error;
      E_Uninit : constant Error_Code := Not_Initialized;
   begin
      --  Test enumeration ordering
      Assert
        (Error_Code'Pos (E_OK) = 0,
         "OK should be at position 0");
      Assert
        (Error_Code'Pos (E_Init) = 1,
         "Init_Error should be at position 1");
      Assert
        (Error_Code'Pos (E_Parse) = 2,
         "Parse_Error should be at position 2");
      Assert
        (Error_Code'Pos (E_Data) = 3,
         "Invalid_Data should be at position 3");
      Assert
        (Error_Code'Pos (E_Mem) = 4,
         "Memory_Error should be at position 4");
      Assert
        (Error_Code'Pos (E_Uninit) = 5,
         "Not_Initialized should be at position 5");
   end Test_Error_Code_Values;

   procedure Test_Error_Code_Image (T : in out Test_Case) is
      pragma Unreferenced (T);
      use Asterix;
   begin
      --  Test Image function returns human-readable strings
      Assert
        (Asterix.Errors.Error_Code_Image (OK) = "OK",
         "OK image incorrect");
      Assert
        (Asterix.Errors.Error_Code_Image (Init_Error) =
           "Initialization error",
         "Init_Error image incorrect");
      Assert
        (Asterix.Errors.Error_Code_Image (Parse_Error) = "Parse error",
         "Parse_Error image incorrect");
      Assert
        (Asterix.Errors.Error_Code_Image (Invalid_Data) = "Invalid data",
         "Invalid_Data image incorrect");
      Assert
        (Asterix.Errors.Error_Code_Image (Memory_Error) = "Memory error",
         "Memory_Error image incorrect");
      Assert
        (Asterix.Errors.Error_Code_Image (Not_Initialized) =
           "Parser not initialized",
         "Not_Initialized image incorrect");
   end Test_Error_Code_Image;

end Unit_Tests.Error_Handling;
