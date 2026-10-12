// Blob-storage stub for `NodeData Node_Data` (packages.cpp dead paths).
// NodeData's ctor/dtor pull in the whole target-table machinery; only
// never-called functions reference the global, so raw storage suffices.
// Separate TU: nodebob.h's `extern NodeData Node_Data;` can't coexist with a
// char[] definition in one translation unit.
char Node_Data[4096] = {};
