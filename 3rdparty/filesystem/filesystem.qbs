Product {
  files: "filesystem/include/ghc/*"
  Export {
    Depends { name: "cpp" }
    cpp.includePaths: exportingProduct.sourceDirectory + "/filesystem/include"
  }
}
