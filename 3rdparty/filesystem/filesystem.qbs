Product {
  files: "filesystem/include/ghc/*"
  Export {
    Depends { name: "cpp" }
    cpp.includePaths: product.sourceDirectory + "/filesystem/include"
  }
}
