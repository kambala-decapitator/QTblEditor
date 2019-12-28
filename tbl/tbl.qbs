NativeBinary {
  property stringList exportedDefines: []

  files: [
    "d2color.h",
    "d2color.cpp",
    "exceptions.h",
    "filesystem.h",
    "tbl.h",
    "tbl.cpp",
  ]

  Group {
    files: "../common/common.pch"
    fileTags: "cpp_pch_src"
  }

  cpp.cxxLanguageVersion: "c++17"

  Properties {
    condition: qbs.buildVariant == "release"
    cpp.defines: ["NDEBUG"]
  }

  Depends { name: "cpp" }
  Depends { name: "filesystem" }

  Export {
    Depends { name: "cpp" }
    Depends { name: "filesystem" }

    cpp.defines: product.exportedDefines
    cpp.includePaths: product.sourceDirectory
  }
}
