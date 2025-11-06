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
        "-L../install/lib",
        "-lasterix",
        "-lexpat"
      ],
      "conditions": [
        ["OS=='linux'", {
          "cflags_cc": [
            "-std=c++23"
          ],
          "defines": [
            "_GLIBCXX_USE_CXX11_ABI=1"
          ]
        }],
        ["OS=='mac'", {
          "cflags_cc": [
            "-std=c++23"
          ],
          "xcode_settings": {
            "GCC_ENABLE_CPP_EXCEPTIONS": "YES",
            "CLANG_CXX_LANGUAGE_STANDARD": "c++23",
            "MACOSX_DEPLOYMENT_TARGET": "11.0"
          }
        }],
        ["OS=='win'", {
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
            "expat.lib"
          ]
        }]
      ]
    }
  ]
}
