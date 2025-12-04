--  asterix-errors.adb
--  Implementation of error handling for ASTERIX Ada bindings
--
--  Copyright (c) 2024 ASTERIX Contributors
--  SPDX-License-Identifier: GPL-3.0-or-later

package body Asterix.Errors is

   --  Helper to raise appropriate exception based on error code
   procedure Raise_If_Error
     (Code    : Error_Code;
      Message : String := "")
   is
      Msg : constant String :=
        (if Message = "" then Error_Code_Image (Code) else Message);
   begin
      case Code is
         when OK =>
            null;  --  No error
         when Init_Error =>
            raise Initialization_Error with Msg;
         when Asterix.Parse_Error =>
            raise Asterix.Errors.Parse_Error with Msg;
         when Invalid_Data =>
            raise Invalid_Data_Error with Msg;
         when Asterix.Memory_Error =>
            raise Asterix.Errors.Memory_Error with Msg;
         when Not_Initialized =>
            raise Not_Initialized_Error with Msg;
      end case;
   end Raise_If_Error;

   --  Convert error code to string
   function Error_Code_Image (Code : Error_Code) return String is
   begin
      case Code is
         when OK              => return "OK";
         when Init_Error      => return "Initialization error";
         when Asterix.Parse_Error => return "Parse error";
         when Invalid_Data    => return "Invalid data";
         when Asterix.Memory_Error => return "Memory error";
         when Not_Initialized => return "Parser not initialized";
      end case;
   end Error_Code_Image;

end Asterix.Errors;
