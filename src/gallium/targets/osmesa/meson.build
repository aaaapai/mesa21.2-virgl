# Copyright © 2017 Intel Corporation
# SPDX-License-Identifier: MIT

osmesa_link_with = []
osmesa_link_args = []
osmesa_link_deps = []

if with_ld_version_script
  osmesa_link_args += [
    '-Wl,--version-script', join_paths(meson.current_source_dir(), 'osmesa.sym')
  ]
  osmesa_link_deps += files('osmesa.sym')
endif

osmesa_def = custom_target(
  'osmesa.def',
  input: 'osmesa.def.in',
  output : 'osmesa.def',
  command : gen_vs_module_defs_normal_command,
)

libosmesa = shared_library(
  osmesa_lib_name,
  'osmesa_target.c',
  gnu_symbol_visibility : 'hidden',
  link_args : [ld_args_gc_sections, osmesa_link_args],
  vs_module_defs : osmesa_def,
  include_directories : [
    inc_include, inc_src, inc_gallium, inc_gallium_aux, inc_gallium_winsys,
    inc_gallium_drivers,
  ],
  link_depends : osmesa_link_deps,
  link_whole : [libosmesa_st,],
  link_with : [
    libmesa, libgalliumvl,
    libgallium, libglapi, libpipe_loader_static, libws_null, libwsw, libswdri,
    libswkmsdri, osmesa_link_with,
  ],
  dependencies : [
    dep_ws2_32, dep_thread, dep_clock, dep_unwind, driver_swrast, dep_libdrm, dep_llvm, dep_thread, idep_mesautil, driver_swrast, driver_r300, driver_r600, driver_radeonsi, driver_nouveau,
    driver_kmsro, driver_v3d, driver_vc4, driver_freedreno, driver_etnaviv,
    driver_tegra, driver_i915, driver_svga, driver_virgl,
    driver_panfrost, driver_iris, driver_lima, driver_zink, driver_d3d12,
    driver_asahi, driver_crocus,
  ],
  name_prefix : host_machine.system() == 'windows' ? '' : [],  # otherwise mingw will create libosmesa.dll
  soversion : '',
  version : '8.0.0',
  darwin_versions : '9.0.0',
  install : true,
)

if with_symbols_check
  test('osmesa-symbols-check',
    symbols_check,
    args : [
      '--lib', libosmesa,
      '--symbols-file', files('osmesa-symbols.txt'),
      symbols_check_args,
    ],
    suite : ['gallium'],
  )
endif

pkg.generate(
  name : 'osmesa',
  description : 'Mesa Off-screen Rendering Library',
  version : '8.0.0',
  libraries : libosmesa,
  libraries_private : gl_priv_libs,
)

if with_tests
  test('osmesa-render',
    executable(
      'osmesa-render',
      'test-render.cpp',
      include_directories : [inc_include, inc_src, inc_mapi, inc_mesa, inc_gallium, inc_gallium_aux],
      link_with: libosmesa,
      dependencies : [idep_gtest, idep_mesautil],
    ),
    suite: 'gallium',
    protocol : 'gtest',
  )
endif
