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
          "sources": [
            "../src/asterix/AsterixData.cpp",
            "../src/asterix/AsterixDefinition.cpp",
            "../src/asterix/Category.cpp",
            "../src/asterix/DataBlock.cpp",
            "../src/asterix/DataItem.cpp",
            "../src/asterix/DataItemBits.cpp",
            "../src/asterix/DataItemDescription.cpp",
            "../src/asterix/DataItemFormat.cpp",
            "../src/asterix/DataItemFormatBDS.cpp",
            "../src/asterix/DataItemFormatCompound.cpp",
            "../src/asterix/DataItemFormatExplicit.cpp",
            "../src/asterix/DataItemFormatFixed.cpp",
            "../src/asterix/DataItemFormatRepetitive.cpp",
            "../src/asterix/DataItemFormatVariable.cpp",
            "../src/asterix/InputParser.cpp",
            "../src/asterix/Tracer.cpp",
            "../src/asterix/UAP.cpp",
            "../src/asterix/UAPItem.cpp",
            "../src/asterix/Utils.cpp",
            "../src/asterix/XMLParser.cpp"
          ],
          "include_dirs": [
            "../install/include",
            "../src/asterix",
            "../src/engine"
          ],
          "msvs_settings": {
            "VCCLCompilerTool": {
              "ExceptionHandling": 1,
              "AdditionalOptions": ["/std:c++20"]
            }
          },
          "defines": [
            "_HAS_EXCEPTIONS=1",
            "XML_STATIC"
          ],
          "link_settings": {
            "library_dirs": [
              "../install/lib"
            ],
            "libraries": [
              "-lexpat",
              "-lws2_32"
            ]
          }
        }]
      ]
    }
  ]
}
