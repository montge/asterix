--  integration_tests-initialization.adb
--  Integration tests for library initialization
--
--  Copyright (c) 2024 ASTERIX Contributors
--  SPDX-License-Identifier: GPL-3.0-or-later

with AUnit.Assertions; use AUnit.Assertions;
with Asterix;

package body Integration_Tests.Initialization is

   procedure Test_Init_Default (T : in out Test_Case) is
      pragma Unreferenced (T);
   begin
      --  Initialize with default configuration
      Asterix.Init_Default;

      --  Verify initialization succeeded
      Assert
        (Asterix.Is_Initialized,
         "Init_Default should set Is_Initialized to True");
   end Test_Init_Default;

   procedure Test_Is_Initialized (T : in out Test_Case) is
      pragma Unreferenced (T);
   begin
      --  Ensure library is initialized
      if not Asterix.Is_Initialized then
         Asterix.Init_Default;
      end if;

      --  Verify Is_Initialized returns True
      Assert
        (Asterix.Is_Initialized,
         "Is_Initialized should return True after Init_Default");
   end Test_Is_Initialized;

   procedure Test_Version (T : in out Test_Case) is
      pragma Unreferenced (T);
      Ver : constant String := Asterix.Version;
   begin
      --  Version should not be empty
      Assert (Ver'Length > 0, "Version should not be empty");

      --  Version should contain digits (e.g., "2.8.10")
      declare
         Has_Digit : Boolean := False;
      begin
         for C of Ver loop
            if C in '0' .. '9' then
               Has_Digit := True;
               exit;
            end if;
         end loop;
         Assert (Has_Digit, "Version should contain digits");
      end;
   end Test_Version;

end Integration_Tests.Initialization;
