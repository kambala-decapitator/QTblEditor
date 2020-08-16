import qbs.Utilities

CppApplication {
  consoleApplication: true
  files: "test.cpp"

  Group {
    files: "test.pch"
    fileTags: "cpp_pch_src"
  }

  cpp.cxxLanguageVersion: "c++17"
  cpp.defines: ["DATA_PATH=" + Utilities.cStringQuote(sourceDirectory + "/data")]
  
  Properties {
    condition: qbs.toolchain.contains("msvc")
    cpp.defines: outer.concat("_SILENCE_CXX17_CODECVT_HEADER_DEPRECATION_WARNING")
  }

  Depends { name: "catch" }
  Depends { name: "tbl" }
}
