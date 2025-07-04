# Copyright © 2017 Intel Corporation
# SPDX-License-Identifier: MIT

inc_include = [include_directories('.')]
inc_d3d9 = include_directories('D3D9')
inc_winddk = include_directories('winddk')

# Most things assume that Android headers are in the default include
# path when compiling for Android so add the stub headers to
# inc_include when using the stub.
if with_android_stub
  inc_include += [include_directories('android_stub')]
endif

if not with_glvnd and host_machine.system() != 'windows'
  if with_gles1 or with_gles2 or with_opengl or with_egl
    install_headers('KHR/khrplatform.h', subdir : 'KHR')
  endif

  if with_gles1
    install_headers(
      'GLES/egl.h',
      'GLES/gl.h',
      'GLES/glext.h',
      'GLES/glplatform.h',
      subdir : 'GLES',
    )
  endif

  if with_gles2
    install_headers(
      'GLES2/gl2.h',
      'GLES2/gl2ext.h',
      'GLES2/gl2platform.h',
      subdir : 'GLES2',
    )
    install_headers(
      'GLES3/gl3.h',
      'GLES3/gl31.h',
      'GLES3/gl32.h',
      'GLES3/gl3ext.h',
      'GLES3/gl3platform.h',
      subdir : 'GLES3',
    )
  endif

  if with_opengl
    install_headers(
      'GL/gl.h',
      'GL/glcorearb.h',
      'GL/glext.h',
      subdir : 'GL',
    )
  endif

  if with_glx != 'disabled'
    install_headers(
      'GL/glx.h',
      'GL/glxext.h',
      subdir : 'GL')
  endif

  if with_egl
    install_headers(
      'EGL/egl.h',
      'EGL/eglext.h',
      'EGL/eglplatform.h',
      subdir : 'EGL',
    )
  endif
endif

# Non-upstream headers
if with_egl
  install_headers(
    'EGL/eglext_angle.h',
    'EGL/eglmesaext.h',
    subdir : 'EGL',
  )
endif

if with_osmesa
  install_headers('GL/osmesa.h', subdir : 'GL')
endif

if with_dri
  install_headers('GL/internal/dri_interface.h', subdir : 'GL/internal')
endif

opencl_headers = files(
  'CL/cl.h',
  'CL/cl_d3d10.h',
  'CL/cl_d3d11.h',
  'CL/cl_dx9_media_sharing.h',
  'CL/cl_dx9_media_sharing_intel.h',
  'CL/cl_egl.h',
  'CL/cl_ext.h',
  'CL/cl_ext_intel.h',
  'CL/cl_function_types.h',
  'CL/cl_gl.h',
  'CL/cl_gl_ext.h',
  'CL/cl_half.h',
  'CL/cl_icd.h',
  'CL/cl_layer.h',
  'CL/cl_platform.h',
  'CL/cl_va_api_media_sharing_intel.h',
  'CL/cl_version.h',
  'CL/opencl.h',
)
