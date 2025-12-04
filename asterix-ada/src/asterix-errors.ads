--  asterix-errors.ads
--  Error handling types for ASTERIX Ada bindings
--
--  Copyright (c) 2024 ASTERIX Contributors
--  SPDX-License-Identifier: GPL-3.0-or-later

package Asterix.Errors is

   pragma Preelaborate;

   --  Exception types
   Initialization_Error : exception;
   Parse_Error          : exception;
   Invalid_Data_Error   : exception;
   Memory_Error         : exception;
   Not_Initialized_Error : exception;

   --  Helper to raise appropriate exception based on error code
   procedure Raise_If_Error
     (Code    : Error_Code;
      Message : String := "");

   --  Convert error code to string
   function Error_Code_Image (Code : Error_Code) return String;

end Asterix.Errors;
