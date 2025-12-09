Product {
  files: "Catch2/single_include/catch2/catch.hpp"
  Export {
    Depends { name: "cpp" }
    cpp.includePaths: exportingProduct.sourceDirectory + "/Catch2/single_include"
  }
}
