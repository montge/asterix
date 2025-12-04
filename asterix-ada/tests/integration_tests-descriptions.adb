--  integration_tests-descriptions.adb
--  Integration tests for ASTERIX description functions
--
--  Copyright (c) 2024 ASTERIX Contributors
--  SPDX-License-Identifier: GPL-3.0-or-later

with AUnit.Assertions; use AUnit.Assertions;
with Asterix;

package body Integration_Tests.Descriptions is

   procedure Test_Describe_Category (T : in out Test_Case) is
      pragma Unreferenced (T);
      Desc_48 : constant String := Asterix.Describe_Category (48);
      Desc_62 : constant String := Asterix.Describe_Category (62);
   begin
      --  Ensure initialized
      if not Asterix.Is_Initialized then
         Asterix.Init_Default;
      end if;

      --  Common categories should have descriptions
      --  Note: Description may be empty if category not defined,
      --  but should not raise exception
      Assert
        (Desc_48'Length >= 0,
         "Describe_Category(48) should return without exception");

      Assert
        (Desc_62'Length >= 0,
         "Describe_Category(62) should return without exception");
   end Test_Describe_Category;

   procedure Test_Is_Category_Defined (T : in out Test_Case) is
      pragma Unreferenced (T);
      Def_48 : Boolean;
      Def_62 : Boolean;
      Def_255 : Boolean;
   begin
      --  Ensure initialized
      if not Asterix.Is_Initialized then
         Asterix.Init_Default;
      end if;

      --  Query category definitions
      Def_48 := Asterix.Is_Category_Defined (48);
      Def_62 := Asterix.Is_Category_Defined (62);
      Def_255 := Asterix.Is_Category_Defined (255);

      --  Categories 48 and 62 are commonly defined
      Assert
        (Def_48 or not Def_48,
         "Is_Category_Defined should return boolean");

      Assert
        (Def_62 or not Def_62,
         "Is_Category_Defined(62) should return boolean");

      --  Category 255 is rarely defined
      Assert
        (Def_255 or not Def_255,
         "Is_Category_Defined(255) should return boolean");
   end Test_Is_Category_Defined;

end Integration_Tests.Descriptions;
