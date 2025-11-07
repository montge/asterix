{
  "targets": [
    {
      "target_name": "asterix",
      "sources": [
        "src/addon.cpp",
        "src/parser_wrapper.cpp"
      ],
      "include_dirs": [
        "<!@(node -p \"require('node-addon-api').include\")",
        "../src/asterix",
        "../src/engine"
      ],
      "dependencies": [
        "<!(node -p \"require('node-addon-api').gyp\")"
      ],
      "cflags!": [ "-fno-exceptions" ],
      "cflags_cc!": [ "-fno-exceptions" ],
      "cflags_cc": [
        "-std=c++20",
        "-Wall",
        "-Wextra",
        "-Wpedantic",
        "-Wformat-security",
        "-Wformat=2",
        "-Wconversion",
        "-Wsign-conversion"
      ],
      "defines": [
        "NAPI_VERSION=8",
        "NAPI_DISABLE_CPP_EXCEPTIONS"
      ],
      "libraries": [
        "-lexpat"
      ],
      "conditions": [
        ["OS=='linux'", {
          "cflags_cc": [
            "-std=c++23"
          ],
          "defines": [
            "_GLIBCXX_USE_CXX11_ABI=1"
          ],
          "libraries": [
            "<!@(node -p \"require('path').resolve(__dirname, '../install/lib/libasterix.so')\")",
            "-lexpat"
          ],
          "ldflags": [
            "-Wl,-rpath,<!@(node -p \"require('path').resolve(__dirname, '../install/lib')\")"
          ]
        }],
        ["OS=='mac'", {
          "cflags_cc": [
            "-std=c++23"
          ],
          "libraries": [
            "<!@(node -p \"require('path').resolve(__dirname, '../install/lib/libasterix.dylib')\")",
            "-lexpat"
          ],
          "xcode_settings": {
            "GCC_ENABLE_CPP_EXCEPTIONS": "YES",
            "CLANG_CXX_LANGUAGE_STANDARD": "c++23",
            "MACOSX_DEPLOYMENT_TARGET": "11.0",
            "LD_RUNPATH_SEARCH_PATHS": [
              "@loader_path/../../install/lib",
              "<!@(node -p \"require('path').resolve(__dirname, '../install/lib')\")"
            ]
          }
        }],
        ["OS=='win'", {
          "include_dirs": [
            "../install/include",
            "<!@(node -p \"process.env.VCPKG_ROOT ? process.env.VCPKG_ROOT + '/installed/x64-windows-static-md/include' : '../install/include'\")"
          ],
          "msvs_settings": {
            "VCCLCompilerTool": {
              "ExceptionHandling": 1,
              "AdditionalOptions": ["/std:c++20"]
            }
          },
          "defines": [
            "_HAS_EXCEPTIONS=1"
          ],
          "libraries": [
            "-LIBPATH:../install/lib",
            "asterix.lib",
            "<!@(node -p \"process.env.VCPKG_ROOT ? '-LIBPATH:' + process.env.VCPKG_ROOT + '/installed/x64-windows-static-md/lib' : ''\")",
            "<!@(node -p \"process.env.VCPKG_ROOT ? 'libexpatMD.lib' : 'expat.lib'\")"
          ]
        }]
      ]
    }
  ]
}
