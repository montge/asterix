--  asterix-records.ads
--  Record type definitions for ASTERIX Ada bindings
--
--  Copyright (c) 2024 ASTERIX Contributors
--  SPDX-License-Identifier: GPL-3.0-or-later

package Asterix.Records is

   pragma Preelaborate;

   --  Re-export types from parent for convenience
   subtype Asterix_Category is Category_Number;
   subtype Asterix_Timestamp is Timestamp_Microseconds;

   --  Data item within a record
   type Data_Item is record
      ID          : Ada.Strings.Unbounded.Unbounded_String;
      Name        : Ada.Strings.Unbounded.Unbounded_String;
      Value       : Ada.Strings.Unbounded.Unbounded_String;
      Description : Ada.Strings.Unbounded.Unbounded_String;
   end record;

   type Data_Item_Array is array (Positive range <>) of Data_Item;
   type Data_Item_Array_Access is access Data_Item_Array;

end Asterix.Records;
