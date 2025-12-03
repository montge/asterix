--  asterix.adb
--  Implementation of high-level Ada API for ASTERIX decoder
--
--  Copyright (c) 2024 ASTERIX Contributors
--  SPDX-License-Identifier: GPL-3.0-or-later

with Asterix.FFI;          use Asterix.FFI;
with Interfaces.C;         use Interfaces.C;
with Interfaces.C.Strings; use Interfaces.C.Strings;
with System;
with System.Storage_Elements;
with Ada.Unchecked_Deallocation;
with Ada.Unchecked_Conversion;

package body Asterix is

   --  Helper to convert C error codes to Ada
   function To_Error_Code (C_Code : int) return Error_Code is
   begin
      case C_Code is
         when ASTERIX_OK         => return OK;
         when ASTERIX_ERR_INIT   => return Init_Error;
         when ASTERIX_ERR_PARSE  => return Parse_Error;
         when ASTERIX_ERR_INVALID => return Invalid_Data;
         when ASTERIX_ERR_MEMORY => return Memory_Error;
         when others             => return Parse_Error;
      end case;
   end To_Error_Code;

   --  Helper to safely convert chars_ptr to String
   function Safe_Value (Ptr : chars_ptr) return String is
   begin
      if Ptr = Null_Ptr then
         return "";
      else
         return Value (Ptr);
      end if;
   end Safe_Value;

   --  Parse_Result finalization (cleanup)
   overriding procedure Finalize (Self : in out Parse_Result) is
      procedure Free is new Ada.Unchecked_Deallocation
        (Record_Array, Record_Array_Access);
   begin
      if Self.Records /= null then
         Free (Self.Records);
      end if;
   end Finalize;

   --  Initialize with default configuration
   procedure Init_Default is
      C_Path : chars_ptr;
      Result : int;
   begin
      if Initialized then
         return;
      end if;

      --  Try standard paths for asterix.ini
      C_Path := New_String ("../share/asterix/config/asterix.ini");
      Result := Asterix_Init (C_Path);
      Free (C_Path);
      if Result = ASTERIX_OK then
         Initialized := True;
         return;
      end if;

      C_Path := New_String ("./asterix/config/asterix.ini");
      Result := Asterix_Init (C_Path);
      Free (C_Path);
      if Result = ASTERIX_OK then
         Initialized := True;
         return;
      end if;

      C_Path := New_String ("../asterix/config/asterix.ini");
      Result := Asterix_Init (C_Path);
      Free (C_Path);
      if Result = ASTERIX_OK then
         Initialized := True;
         return;
      end if;

      C_Path := New_String ("/usr/share/asterix/config/asterix.ini");
      Result := Asterix_Init (C_Path);
      Free (C_Path);
      if Result = ASTERIX_OK then
         Initialized := True;
         return;
      end if;

      --  If we get here, initialization failed
      raise Program_Error with
        "Failed to initialize ASTERIX: " & Get_Last_Error;
   end Init_Default;

   --  Initialize with custom configuration file
   procedure Init (Config_Path : String) is
      C_Path : chars_ptr := New_String (Config_Path);
      Result : int;
   begin
      Result := Asterix_Init (C_Path);
      Free (C_Path);

      if Result /= ASTERIX_OK then
         raise Program_Error with
           "Failed to initialize ASTERIX: " & Get_Last_Error;
      end if;

      Initialized := True;
   end Init;

   --  Check if parser is initialized
   function Is_Initialized return Boolean is
   begin
      return Asterix_Is_Initialized = 1;
   end Is_Initialized;

   --  Parse ASTERIX data with default options
   function Parse (Data : Byte_Array) return Parse_Result is
   begin
      return Parse (Data, Default_Options);
   end Parse;

   --  Parse ASTERIX data with custom options
   function Parse
     (Data    : Byte_Array;
      Options : Parse_Options) return Parse_Result
   is
   begin
      return Parse_With_Offset
        (Data       => Data,
         Offset     => Options.Offset,
         Max_Blocks => Options.Max_Blocks,
         Verbose    => Options.Verbose);
   end Parse;

   --  Parse with offset (for incremental parsing)
   function Parse_With_Offset
     (Data       : Byte_Array;
      Offset     : Natural;
      Max_Blocks : Natural := 0;
      Verbose    : Boolean := True) return Parse_Result
   is
      use System.Storage_Elements;

      Result      : Parse_Result;
      C_Result    : AsterixParseResult_Ptr;
      Verbose_Int : constant int := (if Verbose then 1 else 0);
   begin
      if not Is_Initialized then
         Result.Error := Not_Initialized;
         Result.Error_Message :=
           To_Unbounded_String ("Parser not initialized");
         return Result;
      end if;

      if Data'Length = 0 then
         Result.Error := Invalid_Data;
         Result.Error_Message := To_Unbounded_String ("Empty data");
         return Result;
      end if;

      --  Call C API
      C_Result := Asterix_Parse_With_Offset
        (Data       => Data (Data'First)'Address,
         Length     => size_t (Data'Length),
         Offset     => size_t (Offset),
         Max_Blocks => size_t (Max_Blocks),
         Verbose    => Verbose_Int);

      if C_Result = null then
         Result.Error := Memory_Error;
         Result.Error_Message :=
           To_Unbounded_String ("Null result from parser");
         return Result;
      end if;

      --  Convert result
      Result.Error := To_Error_Code (C_Result.Error_Code);
      Result.Error_Message := To_Unbounded_String
        (Safe_Value (C_Result.Error_Message));
      Result.Bytes_Consumed := Natural (C_Result.Bytes_Consumed);

      --  Convert records if any
      if C_Result.Count > 0 and then C_Result.Records /= null then
         Result.Records :=
           new Record_Array (1 .. Positive (C_Result.Count));

         declare
            C_Rec_Addr : System.Address := C_Result.Records.all'Address;
            C_Rec_Size : constant Storage_Offset :=
              AsterixRecord'Size / 8;
         begin
            for I in Result.Records'Range loop
               --  Read record at current address
               declare
                  type Rec_Access is access all AsterixRecord;
                  function To_Rec is new Ada.Unchecked_Conversion
                    (System.Address, Rec_Access);
                  pragma Warnings (Off, To_Rec);
                  C_Rec : constant Rec_Access := To_Rec (C_Rec_Addr);
                  Json  : constant String := Safe_Value (C_Rec.Json_Data);
               begin
                  Result.Records (I) := Asterix_Record'
                    (Category  => Category_Number (C_Rec.Category),
                     Length    => Natural (C_Rec.Length),
                     Timestamp =>
                       Timestamp_Microseconds (C_Rec.Timestamp_Us),
                     Json_Data => To_Unbounded_String (Json),
                     CRC       => Interfaces.Unsigned_32 (C_Rec.CRC));
               end;

               --  Move to next record in C array
               C_Rec_Addr := C_Rec_Addr + C_Rec_Size;
            end loop;
         end;
      end if;

      --  Free C result
      Asterix_Free_Result (C_Result);

      return Result;
   end Parse_With_Offset;

   --  Get description for a category
   function Describe_Category (Category : Category_Number) return String is
      C_Result : chars_ptr;
      Desc     : Unbounded_String;
   begin
      C_Result := Asterix_Describe
        (Category => int (Category),
         Item     => Null_Ptr,
         Field    => Null_Ptr,
         Value    => Null_Ptr);

      if C_Result = Null_Ptr then
         return "";
      end if;

      Desc := To_Unbounded_String (Value (C_Result));
      Asterix_Free_String (C_Result);

      return To_String (Desc);
   end Describe_Category;

   --  Get description for an item in a category
   function Describe_Item
     (Category : Category_Number;
      Item     : String) return String
   is
      C_Item   : chars_ptr := New_String (Item);
      C_Result : chars_ptr;
      Desc     : Unbounded_String;
   begin
      C_Result := Asterix_Describe
        (Category => int (Category),
         Item     => C_Item,
         Field    => Null_Ptr,
         Value    => Null_Ptr);

      Free (C_Item);

      if C_Result = Null_Ptr then
         return "";
      end if;

      Desc := To_Unbounded_String (Value (C_Result));
      Asterix_Free_String (C_Result);

      return To_String (Desc);
   end Describe_Item;

   --  Get description for a field value
   function Describe_Value
     (Category : Category_Number;
      Item     : String;
      Field    : String;
      Value    : String) return String
   is
      C_Item   : chars_ptr := New_String (Item);
      C_Field  : chars_ptr := New_String (Field);
      C_Val    : chars_ptr := New_String (Value);
      C_Result : chars_ptr;
      Desc     : Unbounded_String;
   begin
      C_Result := Asterix_Describe
        (Category => int (Category),
         Item     => C_Item,
         Field    => C_Field,
         Value    => C_Val);

      Free (C_Item);
      Free (C_Field);
      Free (C_Val);

      if C_Result = Null_Ptr then
         return "";
      end if;

      Desc := To_Unbounded_String (Interfaces.C.Strings.Value (C_Result));
      Asterix_Free_String (C_Result);

      return To_String (Desc);
   end Describe_Value;

   --  Check if a category is defined
   function Is_Category_Defined (Category : Category_Number) return Boolean is
   begin
      return Asterix_Is_Category_Defined (int (Category)) = 1;
   end Is_Category_Defined;

   --  Get library version
   function Version return String is
      C_Version : constant chars_ptr := Asterix_Version;
   begin
      return Safe_Value (C_Version);
   end Version;

   --  Get last error message (if any)
   function Get_Last_Error return String is
      C_Error : constant chars_ptr := Asterix_Get_Last_Error;
   begin
      return Safe_Value (C_Error);
   end Get_Last_Error;

end Asterix;
