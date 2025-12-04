--  asterix-spark_types.adb
--  SPARK-verified type implementations
--
--  All functions in this package are proven to be free of:
--    - Buffer overflows
--    - Integer overflows
--    - Division by zero
--    - Array index out of bounds
--
--  Copyright (c) 2024 ASTERIX Contributors
--  SPDX-License-Identifier: GPL-3.0-or-later

package body Asterix.SPARK_Types
  with SPARK_Mode => On
is

   ---------------------------------------------------------------------------
   --  Safe conversion utilities
   ---------------------------------------------------------------------------

   function To_Byte (N : Natural) return Byte is
   begin
      return Byte (N);
   end To_Byte;

   ---------------------------------------------------------------------------
   --  Buffer length calculation
   ---------------------------------------------------------------------------

   function Safe_Length
     (First : Buffer_Index;
      Last  : Buffer_Index) return Buffer_Length
   is
   begin
      return Last - First + 1;
   end Safe_Length;

end Asterix.SPARK_Types;
