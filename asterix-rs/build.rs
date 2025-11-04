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
            // Use vcpkg to find expat on Windows
            // This correctly handles vcpkg's libexpatMD.lib naming convention
            // (vcpkg uses libexpatMD.lib instead of expat.lib for x64-windows-static-md triplet)
            match vcpkg::find_package("expat") {
                Ok(lib) => {
                    eprintln!("Successfully found expat via vcpkg:");
                    eprintln!("  Include paths: {:?}", lib.include_paths);
                    eprintln!("  Link paths: {:?}", lib.link_paths);
                }
                Err(e) => {
                    eprintln!("Warning: vcpkg could not find expat: {}", e);
                    eprintln!("Falling back to manual linking (may fail with LNK1181 error)");
                    println!("cargo:rustc-link-lib=expat");
                }
            }
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

    // On Windows, add vcpkg include/lib paths if CMAKE_TOOLCHAIN_FILE is set
    // Note: This runs when building ON Windows (cross-compilation aware)
    if cfg!(windows) || env::var("CARGO_CFG_TARGET_OS").unwrap() == "windows" {
        if let Ok(toolchain) = env::var("CMAKE_TOOLCHAIN_FILE") {
            // Extract vcpkg root from toolchain path
            // CMAKE_TOOLCHAIN_FILE = D:/a/asterix/asterix/vcpkg/scripts/buildsystems/vcpkg.cmake
            if let Some(vcpkg_root) = toolchain
                .strip_suffix("/scripts/buildsystems/vcpkg.cmake")
                .or_else(|| toolchain.strip_suffix("\\scripts\\buildsystems\\vcpkg.cmake"))
            {
                let vcpkg_include = format!("{vcpkg_root}/installed/x64-windows/include");
                let vcpkg_lib = format!("{vcpkg_root}/installed/x64-windows/lib");

                eprintln!("Using vcpkg paths:");
                eprintln!("  Include: {vcpkg_include}");
                eprintln!("  Lib: {vcpkg_lib}");

                bridge.include(&vcpkg_include);
                println!("cargo:rustc-link-search=native={vcpkg_lib}");
            } else {
                eprintln!("Warning: CMAKE_TOOLCHAIN_FILE set but couldn't extract vcpkg root");
                eprintln!("  Toolchain file: {toolchain}");
            }
        } else {
            // Try to find vcpkg in common locations
            eprintln!("Warning: CMAKE_TOOLCHAIN_FILE not set, searching for vcpkg...");

            // Check VCPKG_ROOT environment variable
            if let Ok(vcpkg_root) = env::var("VCPKG_ROOT") {
                let vcpkg_include = format!("{vcpkg_root}/installed/x64-windows/include");
                let vcpkg_lib = format!("{vcpkg_root}/installed/x64-windows/lib");

                eprintln!("Found VCPKG_ROOT:");
                eprintln!("  Include: {vcpkg_include}");
                eprintln!("  Lib: {vcpkg_lib}");

                bridge.include(&vcpkg_include);
                println!("cargo:rustc-link-search=native={vcpkg_lib}");
            } else {
                eprintln!("Warning: Neither CMAKE_TOOLCHAIN_FILE nor VCPKG_ROOT set on Windows");
                eprintln!("  Expat headers may not be found!");
            }
        }
    }

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
            // Debug output to understand cross-compile environment
            eprintln!("ERROR: Source file not found: {}", source_path.display());
            eprintln!("  asterix_src: {}", asterix_src.display());
            eprintln!("  asterix_root: {}", asterix_root.display());
            eprintln!("  Looking for: {source}");
            eprintln!(
                "  CARGO_MANIFEST_DIR: {}",
                env::var("CARGO_MANIFEST_DIR").unwrap()
            );
            if let Ok(current) = env::current_dir() {
                eprintln!("  Current dir: {}", current.display());
            }

            // List what files ARE in asterix_src
            if let Ok(entries) = std::fs::read_dir(&asterix_src) {
                eprintln!("  Files in asterix_src:");
                for entry in entries.flatten() {
                    eprintln!("    - {}", entry.file_name().to_string_lossy());
                }
            } else {
                eprintln!("  Could not read asterix_src directory!");
            }

            // Also check parent directories
            eprintln!("  Parent directory structure:");
            if let Ok(parent_entries) = std::fs::read_dir(asterix_root.join("src")) {
                eprintln!("  Files in src/:");
                for entry in parent_entries.flatten() {
                    eprintln!("    - {}", entry.file_name().to_string_lossy());
                }
            }

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
