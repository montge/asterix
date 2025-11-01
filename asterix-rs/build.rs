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
    } else {
        compile_cpp_from_source();
    }

    // Generate CXX bridge code
    // Note: The actual FFI bridge will be in ffi_wrapper.cpp which wraps C++ classes
    cxx_build::bridge("src/ffi.rs")
        .file("src/ffi_wrapper.cpp")
        .include("../src/asterix")
        .include("../src/engine")
        .flag_if_supported("-std=c++17")
        .flag_if_supported("-fPIC")
        .warnings(false) // Suppress warnings from C++ code
        .compile("asterix_ffi_bridge");

    // Set library search path
    let out_dir = env::var("OUT_DIR").unwrap();
    println!("cargo:rustc-link-search=native={}", out_dir);

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

fn compile_cpp_from_source() {
    let asterix_root = PathBuf::from(env::var("CARGO_MANIFEST_DIR").unwrap())
        .parent()
        .unwrap()
        .to_path_buf();

    let asterix_src = asterix_root.join("src").join("asterix");
    let engine_src = asterix_root.join("src").join("engine");

    // Build C++ core library
    let mut cc_build = cc::Build::new();

    cc_build
        .cpp(true)
        .std("c++17")
        .flag_if_supported("-fPIC")
        .flag_if_supported("-fstack-protector-strong")
        .flag_if_supported("-D_FORTIFY_SOURCE=2")
        .include(&asterix_src)
        .include(&engine_src)
        .warnings(false); // Suppress warnings from legacy C++ code

    // Add all ASTERIX core C++ files
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
            cc_build.file(source_path);
        } else {
            panic!("Required source file not found: {}", source_path.display());
        }
    }

    // Compile the ASTERIX core library
    cc_build.compile("asterix_core");

    // Tell cargo to link against the compiled library
    println!("cargo:rustc-link-lib=static=asterix_core");
}

fn link_system_library() {
    // Attempt to use system-installed ASTERIX library
    // This is for advanced users who have built and installed the library system-wide
    if let Err(e) = pkg_config::probe_library("asterix") {
        eprintln!("Warning: Could not find system ASTERIX library: {}", e);
        eprintln!("Falling back to compilation from source...");
        compile_cpp_from_source();
    } else {
        println!("cargo:rustc-link-lib=asterix");
    }
}
