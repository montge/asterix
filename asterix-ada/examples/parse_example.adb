--  parse_example.adb
--  Example: Parse ASTERIX data from raw bytes
--
--  Build with: gprbuild -P asterix_decoder.gpr -XASTERIX_BUILD_MODE=debug
--  Run with:   ./obj/debug/parse_example
--
--  Copyright (c) 2024 ASTERIX Contributors
--  SPDX-License-Identifier: GPL-3.0-or-later

with Ada.Text_IO;           use Ada.Text_IO;
with Ada.Strings.Unbounded; use Ada.Strings.Unbounded;
with Asterix;               use Asterix;

procedure Parse_Example is

   --  Sample ASTERIX Category 48 data (minimal valid record)
   --  CAT 48, Length 8, FSPEC 80 (I010), SAC=1, SIC=2
   Sample_Data : constant Byte_Array :=
     (16#30#,  --  Category 48
      16#00#,  --  Length high byte
      16#05#,  --  Length low byte (5 bytes total)
      16#80#,  --  FSPEC: I010 present
      16#01#   --  I010/SAC (partial - this is minimal example)
     );

begin
   Put_Line ("ASTERIX Ada Bindings Example");
   Put_Line ("============================");
   New_Line;

   --  Show version
   Put_Line ("Library version: " & Asterix.Version);
   New_Line;

   --  Initialize the parser
   Put_Line ("Initializing parser...");
   begin
      Asterix.Init_Default;
      Put_Line ("Parser initialized successfully.");
   exception
      when E : others =>
         Put_Line ("Warning: Could not initialize with default config.");
         Put_Line ("This example requires the ASTERIX library to be built.");
         return;
   end;

   --  Check initialization
   if Asterix.Is_Initialized then
      Put_Line ("Parser is ready.");
   else
      Put_Line ("Parser initialization failed.");
      return;
   end if;

   New_Line;

   --  Check if category 48 is defined
   Put_Line ("Checking category definitions...");
   if Asterix.Is_Category_Defined (48) then
      Put_Line ("Category 48 (Monoradar Target Reports): Defined");
   else
      Put_Line ("Category 48: Not defined");
   end if;

   if Asterix.Is_Category_Defined (62) then
      Put_Line ("Category 62 (SDPS Track Messages): Defined");
   else
      Put_Line ("Category 62: Not defined");
   end if;

   New_Line;

   --  Parse sample data
   Put_Line ("Parsing sample ASTERIX data...");
   Put_Line ("Data length:" & Sample_Data'Length'Image & " bytes");

   declare
      Result : Parse_Result;
   begin
      Result := Asterix.Parse (Sample_Data);

      Put_Line ("Parse result:");
      Put_Line ("  Error code: " & Result.Error'Image);
      Put_Line ("  Bytes consumed:" & Result.Bytes_Consumed'Image);

      if Result.Records /= null then
         Put_Line ("  Records parsed:" & Result.Records'Length'Image);

         for I in Result.Records'Range loop
            Put_Line ("  Record" & I'Image & ":");
            Put_Line ("    Category:" & Result.Records (I).Category'Image);
            Put_Line ("    Length:" & Result.Records (I).Length'Image);

            --  Show JSON data (truncated if long)
            declare
               Json : constant String := To_String (Result.Records (I).Json_Data);
            begin
               if Json'Length > 0 then
                  if Json'Length > 100 then
                     Put_Line ("    JSON: " & Json (Json'First .. Json'First + 99) & "...");
                  else
                     Put_Line ("    JSON: " & Json);
                  end if;
               end if;
            end;
         end loop;
      else
         Put_Line ("  No records parsed (data may be incomplete)");
      end if;

      if Length (Result.Error_Message) > 0 then
         Put_Line ("  Error message: " & To_String (Result.Error_Message));
      end if;
   end;

   New_Line;
   Put_Line ("Example completed.");

end Parse_Example;
