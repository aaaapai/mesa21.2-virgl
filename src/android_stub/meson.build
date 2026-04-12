if with_android_stub
  stub_libs = []

  foreach lib : ['hardware', 'backtrace', 'cutils', 'log', 'nativewindow', 'sync']
    stub_libs += static_library(
      lib,
      files(lib + '_stub.cpp'),
      include_directories : inc_include,
      install : false,
    )
  endforeach

endif
