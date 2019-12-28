CppApplication {
  consoleApplication: true
  files: "main.cpp"

  Group {
    files: "console-app.pch"
    fileTags: "cpp_pch_src"
  }

  cpp.cxxLanguageVersion: "c++17"

  Depends { name: "args" }
  Depends { name: qbs.buildVariant == "release" ? "tbl-static" : "tbl" }
}
