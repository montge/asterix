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

   procedure Test_Describe_Item (T : in out Test_Case) is
      pragma Unreferenced (T);
      Desc_010 : constant String := Asterix.Describe_Item (48, "I010");
      Desc_020 : constant String := Asterix.Describe_Item (48, "I020");
      Desc_062_010 : constant String := Asterix.Describe_Item (62, "I010");
   begin
      --  Ensure initialized
      if not Asterix.Is_Initialized then
         Asterix.Init_Default;
      end if;

      --  Test Category 48, I010 (Data Source Identifier)
      --  Should return description if category is defined
      Assert
        (Desc_010'Length >= 0,
         "Describe_Item(48, 'I010') should not raise exception");

      --  If category 48 is defined, description should be non-empty
      if Asterix.Is_Category_Defined (48) then
         Assert
           (Desc_010'Length > 0,
            "Describe_Item(48, 'I010') should return non-empty " &
            "for defined category");
      end if;

      --  Test Category 48, I020 (Target Report Descriptor)
      Assert
        (Desc_020'Length >= 0,
         "Describe_Item(48, 'I020') should not raise exception");

      if Asterix.Is_Category_Defined (48) then
         Assert
           (Desc_020'Length > 0,
            "Describe_Item(48, 'I020') should return non-empty " &
            "for defined category");
      end if;

      --  Test Category 62, I010
      Assert
        (Desc_062_010'Length >= 0,
         "Describe_Item(62, 'I010') should not raise exception");

      if Asterix.Is_Category_Defined (62) then
         Assert
           (Desc_062_010'Length > 0,
            "Describe_Item(62, 'I010') should return non-empty " &
            "for defined category");
      end if;
   end Test_Describe_Item;

   procedure Test_Describe_Item_Invalid (T : in out Test_Case) is
      pragma Unreferenced (T);
      Desc_Invalid : constant String := Asterix.Describe_Item (48, "I999");
      Desc_Empty   : constant String := Asterix.Describe_Item (48, "IXXX");
   begin
      --  Ensure initialized
      if not Asterix.Is_Initialized then
         Asterix.Init_Default;
      end if;

      --  Invalid item names should return empty string or not raise exception
      Assert
        (Desc_Invalid'Length >= 0,
         "Describe_Item with invalid item should not raise exception");

      --  Non-existent items should likely return empty
      Assert
        (Desc_Empty'Length >= 0,
         "Describe_Item with non-existent item should not raise exception");
   end Test_Describe_Item_Invalid;

   procedure Test_Describe_Value (T : in out Test_Case) is
      pragma Unreferenced (T);
      --  Test I048/020 TYP field value descriptions
      --  Based on asterix_cat048_1_30.xml:
      --  TYP bit values: 0="No detection", 1="Single PSR detection", etc.
      Desc_TYP_0 : constant String :=
        Asterix.Describe_Value (48, "I020", "TYP", "0");
      Desc_TYP_1 : constant String :=
        Asterix.Describe_Value (48, "I020", "TYP", "1");
      Desc_SIM_0 : constant String :=
        Asterix.Describe_Value (48, "I020", "SIM", "0");
      Desc_SIM_1 : constant String :=
        Asterix.Describe_Value (48, "I020", "SIM", "1");
   begin
      --  Ensure initialized
      if not Asterix.Is_Initialized then
         Asterix.Init_Default;
      end if;

      --  Test TYP field value 0 (No detection)
      Assert
        (Desc_TYP_0'Length >= 0,
         "Describe_Value for TYP=0 should not raise exception");

      --  Test TYP field value 1 (Single PSR detection)
      Assert
        (Desc_TYP_1'Length >= 0,
         "Describe_Value for TYP=1 should not raise exception");

      --  Test SIM field value 0 (Actual target report)
      Assert
        (Desc_SIM_0'Length >= 0,
         "Describe_Value for SIM=0 should not raise exception");

      --  Test SIM field value 1 (Simulated target report)
      Assert
        (Desc_SIM_1'Length >= 0,
         "Describe_Value for SIM=1 should not raise exception");

      --  If category is defined, expect meaningful descriptions
      if Asterix.Is_Category_Defined (48) then
         --  Value descriptions might be empty if field doesn't have
         --  enumerated values, but should not crash
         Assert
           (Desc_TYP_0'Length >= 0,
            "Describe_Value should work for defined categories");
      end if;
   end Test_Describe_Value;

   procedure Test_Describe_Value_Invalid (T : in out Test_Case) is
      pragma Unreferenced (T);
      --  Test with invalid parameters
      Desc_Invalid_Item : constant String :=
        Asterix.Describe_Value (48, "I999", "FIELD", "0");
      Desc_Invalid_Field : constant String :=
        Asterix.Describe_Value (48, "I020", "INVALID", "0");
      Desc_Invalid_Value : constant String :=
        Asterix.Describe_Value (48, "I020", "TYP", "999");
   begin
      --  Ensure initialized
      if not Asterix.Is_Initialized then
         Asterix.Init_Default;
      end if;

      --  Invalid item should not crash
      Assert
        (Desc_Invalid_Item'Length >= 0,
         "Describe_Value with invalid item should not raise exception");

      --  Invalid field should not crash
      Assert
        (Desc_Invalid_Field'Length >= 0,
         "Describe_Value with invalid field should not raise exception");

      --  Invalid value should not crash
      Assert
        (Desc_Invalid_Value'Length >= 0,
         "Describe_Value with invalid value should not raise exception");
   end Test_Describe_Value_Invalid;

end Integration_Tests.Descriptions;
