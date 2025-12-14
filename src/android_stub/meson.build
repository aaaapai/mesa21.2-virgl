if with_android_stub
  stub_libs = []

  foreach lib : ['hardware', 'backtrace', 'cutils', 'nativewindow', 'sync']
    stub_libs += static_library(
      lib,
      files(lib + '_stub.cpp'),
      include_directories : inc_include,
      install : false,
    )
  endforeach

  dep_android = declare_dependency(
    link_with : stub_libs,
    dependencies : [
      dependency('log'),
    ]
  )
endif
