# SPARK Formal Verification Guide

This guide explains how to install and use SPARK (GNATprove) for formal verification
of the ASTERIX Ada bindings.

## Why SPARK for ASTERIX?

ASTERIX is used in safety-critical Air Traffic Management systems. SPARK provides:

- **Absence of runtime errors**: Proves no buffer overflows, division by zero, etc.
- **Data flow analysis**: Verifies no uninitialized variable reads
- **Functional correctness**: Proves code matches specification
- **Information flow**: Proves no unintended data leakage

## Installing GNATprove

### Option 1: Via Alire (Recommended)

```bash
# Update Alire to latest version first
# Download from: https://alire.ada.dev/

# Add gnatprove to your project
cd asterix-ada
alr with gnatprove

# Configure environment
eval "$(alr printenv)"

# Verify installation
gnatprove --version
```

### Option 2: GNAT Community Edition (Archived)

The GNAT Community Edition (2021) included GNATprove but is no longer maintained.
Archives may be available at: https://github.com/alire-project/GNAT-FSF-builds

### Option 3: Build from Source

```bash
# Prerequisites
sudo apt-get install -y \
    why3 \
    alt-ergo \
    cvc4 \
    z3

# Clone SPARK2014
git clone https://github.com/AdaCore/spark2014.git
cd spark2014

# Follow build instructions in README
make
```

### Option 4: Commercial SPARK Pro

For production safety-critical systems, contact AdaCore:
https://www.adacore.com/sparkpro

## SPARK Architecture for ASTERIX

```
+--------------------------------------------------+
|                  User Application                |
+--------------------------------------------------+
|              asterix.ads (SPARK_Mode)           |
|         High-level API with contracts            |
|    Pre/Post conditions, type invariants          |
+--------------------------------------------------+
|            asterix-ffi.ads (No SPARK)           |
|         C FFI bindings - external calls          |
|           Cannot be formally verified            |
+--------------------------------------------------+
|              C++ ASTERIX Library                 |
|         (External, not SPARK-verifiable)         |
+--------------------------------------------------+
```

### What Can Be Proven

| Component | SPARK Status | Notes |
|-----------|--------------|-------|
| `asterix.ads/adb` | Partial | Contracts verified, FFI calls assumed |
| `asterix-ffi.ads` | No | External C imports, cannot prove |
| `asterix-records.ads` | Yes | Pure Ada types, fully provable |
| `asterix-errors.ads` | Yes | Exception handling, provable |

## Running GNATprove

```bash
cd asterix-ada

# Analyze specific file
gnatprove -P asterix_decoder.gpr --mode=check asterix.ads

# Full proof (may take time)
gnatprove -P asterix_decoder.gpr --mode=all

# Generate proof report
gnatprove -P asterix_decoder.gpr --report=all
```

## SPARK Annotations

### Contract Example

```ada
function Parse (Data : Byte_Array) return Parse_Result
  with
    Pre  => Data'Length > 0 and Data'Length <= Max_Message_Size,
    Post => Parse'Result.Error = OK or else
            Parse'Result.Records = null;
```

### SPARK_Mode Pragma

```ada
package Asterix
  with SPARK_Mode => On
is
   -- SPARK-verified code here

   procedure Safe_Operation (X : in out Integer)
     with
       Pre  => X >= 0,
       Post => X >= 0;

private
   pragma SPARK_Mode (Off);
   -- Non-SPARK code (FFI calls, etc.)
end Asterix;
```

## Verification Levels

| Level | Command | What It Checks |
|-------|---------|----------------|
| Check | `--mode=check` | SPARK subset compliance |
| Flow | `--mode=flow` | Data flow, initialization |
| Prove | `--mode=prove` | Absence of runtime errors |
| All | `--mode=all` | Complete verification |

## DO-178C/DO-278A Compliance

For aviation certification, SPARK proof evidence can support:

- **MC/DC Coverage**: Contracts provide decision coverage
- **Absence of Runtime Errors**: Direct proof evidence
- **Data Coupling Analysis**: Flow analysis provides this
- **Code Review Evidence**: Contracts document intent

See `docs/DO-278_IMPLEMENTATION_GUIDE.md` for certification guidance.

## Troubleshooting

### "Cannot find prover"

Install the required provers:
```bash
sudo apt-get install alt-ergo cvc4 z3
```

### "SPARK_Mode not allowed"

Some constructs are not SPARK-compatible:
- Access types (pointers)
- Exception handlers with "when others"
- Tasking (without Ravenscar)

### "Unproved check"

Add loop invariants or strengthen preconditions:
```ada
for I in Data'Range loop
   pragma Loop_Invariant (I >= Data'First);
   -- ...
end loop;
```

## Resources

- [SPARK User's Guide](https://docs.adacore.com/spark2014-docs/html/ug/)
- [SPARK Reference Manual](https://docs.adacore.com/spark2014-docs/html/lrm/)
- [AdaCore SPARK Training](https://learn.adacore.com/courses/intro-to-spark/)
- [SPARK GitHub](https://github.com/AdaCore/spark2014)
