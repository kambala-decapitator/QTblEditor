import "tbl.qbs" as Tbl

Tbl {
  property stringList commonDefines: ["TBL_STATIC"]
  exportedDefines: commonDefines

  name: "tbl-static"
  type: "staticlibrary"
  cpp.defines: base.concat(commonDefines)
}
