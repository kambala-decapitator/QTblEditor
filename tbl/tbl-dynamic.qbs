import "tbl.qbs" as Tbl

Tbl {
  name: "tbl"
  type: "dynamiclibrary"
  cpp.defines: base.concat("TBL_LIBRARY")
}
