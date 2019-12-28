Product {
  files: "args/args.hxx"
  Export {
    Depends { name: "cpp" }
    cpp.includePaths: product.sourceDirectory + "/args"
  }
}
