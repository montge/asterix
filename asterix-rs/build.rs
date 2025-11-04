// Build script for compiling C++ ASTERIX core and generating FFI bridge

use std::env;
use std::path::PathBuf;

fn main() {
    let target_os = env::var("CARGO_CFG_TARGET_OS").unwrap();
    let _target_family = env::var("CARGO_CFG_TARGET_FAMILY").unwrap();

    println!("cargo:rerun-if-changed=src/ffi.rs");
    println!("cargo:rerun-if-changed=src/ffi_wrapper.h");
    println!("cargo:rerun-if-changed=src/ffi_wrapper.cpp");
    println!("cargo:rerun-if-changed=../src/asterix/");

    // Determine if we should use system library or compile from source
    let use_system_lib = env::var("ASTERIX_USE_SYSTEM_LIB")
        .map(|v| v == "1")
        .unwrap_or(false);

    if use_system_lib {
        link_system_library();
        // Generate CXX bridge code that links against system library
        cxx_build::bridge("src/ffi.rs")
            .file("src/ffi_wrapper.cpp")
            .include("src")
            .include("../src/asterix")
            .include("../src/engine")
            .flag_if_supported("-std=c++17")
            .flag_if_supported("-fPIC")
            .warnings(false)
            .compile("asterix_ffi_bridge");
    } else {
        // When compiling from source, add all CPP files to the cxx_build
        // to ensure symbols are available (avoids static library link order issues)
        compile_cpp_with_ffi_bridge();
    }

    // Set library search path
    let out_dir = env::var("OUT_DIR").unwrap();
    println!("cargo:rustc-link-search=native={out_dir}");

    // Link against expat
    match target_os.as_str() {
        "windows" => {
            println!("cargo:rustc-link-lib=expat");
        }
        _ => {
            // Use pkg-config to find expat on Unix-like systems
            if pkg_config::probe_library("expat").is_err() {
                println!("cargo:rustc-link-lib=expat");
            }
        }
    }
}

fn compile_cpp_with_ffi_bridge() {
    // Compile all C++ source files together with the FFI bridge
    // This avoids static library link order issues
    let asterix_root = PathBuf::from(env::var("CARGO_MANIFEST_DIR").unwrap())
        .parent()
        .unwrap()
        .to_path_buf();

    let asterix_src = asterix_root.join("src").join("asterix");
    let engine_src = asterix_root.join("src").join("engine");

    // Generate CXX bridge code and compile everything together
    let mut bridge = cxx_build::bridge("src/ffi.rs");
    bridge
        .file("src/ffi_wrapper.cpp")
        .include("src") // Include our own src directory for ffi_wrapper.h
        .include(&asterix_src)
        .include(&engine_src)
        .flag_if_supported("-std=c++17")
        .flag_if_supported("-fPIC")
        .warnings(false); // Suppress warnings from C++ code

    // Add all ASTERIX core C++ files to the same compilation unit
    let asterix_sources = [
        "AsterixData.cpp",
        "AsterixDefinition.cpp",
        "Category.cpp",
        "DataBlock.cpp",
        "DataItem.cpp",
        "DataItemBits.cpp",
        "DataItemDescription.cpp",
        "DataItemFormat.cpp",
        "DataItemFormatBDS.cpp",
        "DataItemFormatCompound.cpp",
        "DataItemFormatExplicit.cpp",
        "DataItemFormatFixed.cpp",
        "DataItemFormatRepetitive.cpp",
        "DataItemFormatVariable.cpp",
        "DataRecord.cpp",
        "InputParser.cpp",
        "Tracer.cpp",
        "UAP.cpp",
        "UAPItem.cpp",
        "Utils.cpp",
        "WiresharkWrapper.cpp",
        "XMLParser.cpp",
    ];

    for source in &asterix_sources {
        let source_path = asterix_src.join(source);
        if source_path.exists() {
            bridge.file(source_path);
        } else {
            panic!("Required source file not found: {}", source_path.display());
        }
    }

    // Add engine source files for global variables
    let engine_sources = ["globals.cpp"];

    for source in &engine_sources {
        let source_path = engine_src.join(source);
        if source_path.exists() {
            bridge.file(source_path);
        } else {
            panic!(
                "Required engine source file not found: {}",
                source_path.display()
            );
        }
    }

    // Compile everything together into one library
    bridge.compile("asterix_ffi_bridge");
}

fn link_system_library() {
    // Attempt to use system-installed ASTERIX library
    // This is for advanced users who have built and installed the library system-wide
    if let Err(e) = pkg_config::probe_library("asterix") {
        eprintln!("Warning: Could not find system ASTERIX library: {e}");
        eprintln!("Falling back to compilation from source...");
        compile_cpp_with_ffi_bridge();
    } else {
        println!("cargo:rustc-link-lib=asterix");
    }
}
