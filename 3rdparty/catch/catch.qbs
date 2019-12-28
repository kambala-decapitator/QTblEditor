Product {
  files: "Catch2/single_include/catch2/catch.hpp"
  Export {
    Depends { name: "cpp" }
    cpp.includePaths: product.sourceDirectory + "/Catch2/single_include"
  }
}
