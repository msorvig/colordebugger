TEMPLATE = subdirs
wasm {
    SUBDIRS += web
} else {
    SUBDIRS +=\
      chromaticitydiagram \
      colordebugger\
}
