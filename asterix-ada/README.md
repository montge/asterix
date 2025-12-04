# ASTERIX Ada Bindings

Ada language bindings for the ASTERIX ATM surveillance protocol decoder.

## Overview

This package provides idiomatic Ada bindings to the ASTERIX decoder library,
enabling Ada applications to parse ASTERIX (All Purpose STructured EUROCONTROL
SuRveillance Information EXchange) protocol data.

**Safety Features:**
- Pre/Post contracts for API validation
- SPARK-compatible annotations for formal verification
- Bounded types to prevent buffer overflows
- Automatic resource management (RAII pattern)

## Requirements

- **GNAT**: GCC Ada compiler (13.0+ recommended)
- **GPRbuild**: GNAT Project Manager build tool
- **Alire** (optional): Ada package manager
- **GNATprove** (optional): For SPARK formal verification
- **ASTERIX C++ library**: Built from parent project

## Building

### Prerequisites

First, build the ASTERIX C++ library from the parent directory:

```bash
cd ..
cmake -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build --parallel
cmake --install build --prefix install
```

### Build Ada Bindings

```bash
cd asterix-ada

# Create output directories
mkdir -p obj/debug obj/release lib/debug lib/release bin

# Build library (debug mode)
gprbuild -P asterix_decoder.gpr -XASTERIX_BUILD_MODE=debug

# Build library (release mode)
gprbuild -P asterix_decoder.gpr -XASTERIX_BUILD_MODE=release

# Run code quality checks
./scripts/check.sh
```

### Using Alire

```bash
cd asterix-ada
alr build
```

## SPARK Formal Verification

For safety-critical applications, this package supports SPARK formal verification.

### Installing GNATprove

```bash
# Via Alire (recommended)
alr with gnatprove
eval "$(alr printenv)"

# Verify installation
gnatprove --version
```

### Running Verification

```bash
# Check SPARK subset compliance
gnatprove -P asterix_decoder.gpr --mode=check

# Full proof (absence of runtime errors)
gnatprove -P asterix_decoder.gpr --mode=all
```

See [SPARK_GUIDE.md](SPARK_GUIDE.md) for detailed verification instructions.

## Usage

```ada
with Asterix;
with Ada.Text_IO;

procedure Example is
   Data   : constant Asterix.Byte_Array := (16#30#, 16#00#, 16#05#, ...);
   Result : Asterix.Parse_Result;
begin
   --  Initialize with default configuration
   Asterix.Init_Default;

   --  Parse ASTERIX data
   Result := Asterix.Parse (Data);

   --  Process results
   if Result.Records /= null then
      for Rec of Result.Records.all loop
         Ada.Text_IO.Put_Line ("Category:" & Rec.Category'Image);
      end loop;
   end if;
end Example;
```

## API Reference

### Initialization

| Function | Description | Contract |
|----------|-------------|----------|
| `Init_Default` | Initialize with default paths | Post: Is_Initialized |
| `Init (Path)` | Initialize with custom config | Pre: Path'Length > 0 |
| `Is_Initialized` | Check if parser is ready | - |

### Parsing

| Function | Description | Contract |
|----------|-------------|----------|
| `Parse (Data)` | Parse with default options | Pre: Data'Length > 0 |
| `Parse (Data, Options)` | Parse with custom options | Pre: Data'Length > 0 |
| `Parse_With_Offset (...)` | Incremental parsing | Pre: Offset < Data'Length |

### Descriptions

- `Describe_Category (Category)` - Get category description
- `Describe_Item (Category, Item)` - Get item description
- `Describe_Value (Category, Item, Field, Value)` - Get value description

### Utilities

- `Is_Category_Defined (Category)` - Check if category is loaded
- `Version` - Get library version
- `Get_Last_Error` - Get last error message

## Types

### Constants

```ada
Max_Message_Size : constant := 65536;   --  Maximum ASTERIX message
Max_Path_Length  : constant := 4096;    --  Maximum config path
Max_Blocks       : constant := 10000;   --  Maximum data blocks
```

### Asterix_Record

```ada
type Asterix_Record is record
   Category  : Category_Number;         --  ASTERIX category (1-255)
   Length    : Natural;                 --  Record length in bytes
   Timestamp : Timestamp_Microseconds;  --  Timestamp (microseconds)
   Json_Data : Unbounded_String;        --  Parsed data as JSON
   CRC       : Unsigned_32;             --  CRC checksum
end record;
```

### Parse_Options

```ada
type Parse_Options is record
   Verbose    : Boolean := True;   --  Include descriptions
   Max_Blocks : Natural := 0;      --  0 = parse all blocks
   Offset     : Natural := 0;      --  Start offset in bytes
end record;
```

### Error_Code

```ada
type Error_Code is
  (OK,
   Init_Error,
   Parse_Error,
   Invalid_Data,
   Memory_Error,
   Not_Initialized);
```

## Code Quality

Run the quality checks before committing:

```bash
./scripts/check.sh
```

This verifies:
- Compilation with strict warnings (`-gnatwa -gnatwe`)
- GNAT style checks
- Line length limits (79 chars max)
- SPARK annotation presence (informational)

## Directory Structure

```
asterix-ada/
├── alire.toml              -- Alire package manifest
├── asterix_decoder.gpr     -- GNAT project file
├── README.md               -- This file
├── SPARK_GUIDE.md          -- SPARK verification guide
├── scripts/
│   └── check.sh            -- Code quality checks
├── src/
│   ├── asterix.ads         -- Main API (with contracts)
│   ├── asterix.adb         -- Main API implementation
│   ├── asterix-ffi.ads     -- FFI bindings to C
│   ├── asterix-records.ads -- Record type definitions
│   ├── asterix-errors.ads  -- Error handling
│   └── asterix-errors.adb  -- Error handling implementation
├── examples/
│   ├── examples.gpr        -- Examples project file
│   └── parse_example.adb   -- Basic parsing example
└── tests/
    └── (future unit tests)
```

## License

GPL-3.0-or-later (same as parent ASTERIX project)

## See Also

- [ASTERIX Project](https://github.com/montge/asterix)
- [EUROCONTROL ASTERIX](https://www.eurocontrol.int/asterix)
- [SPARK User's Guide](https://docs.adacore.com/spark2014-docs/html/ug/)
- [Ada Programming](https://learn.adacore.com/)
- [Alire Package Manager](https://alire.ada.dev/)
