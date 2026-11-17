project(
  'SDL_layer', 'c',
  version: '1.0.1',
  license: 'zlib',
  meson_version: '>=1.8.0',
  default_options: [
    'c_std=c11',
    'warning_level=3',
    'werror=false',
  ]
)
subdir('headers')
subdir('sources')
install_headers(headers)

wayland = import('wayland')
sources += [
  wayland.scan_xml('wayland/xdg-shell.xml', client: true, server: false),
  wayland.scan_xml('wayland/layer-shell.xml', client: true, server: false),
]

sdl_layer_lib = library(
  'SDL3_layer',
  install: true,
  version: meson.project_version(),
  sources: [headers, sources],
  dependencies: [
    dependency('SDL3', required: true, fallback: 'sdl3'),
    dependency('wayland-client', required: true),
  ],
  include_directories: include_directories('headers'),
  c_args: ['-fvisibility=hidden'],
)

sdl_layer_dep = declare_dependency(
  link_with: sdl_layer_lib,
  include_directories: include_directories('headers'),
)

import('pkgconfig').generate(
  sdl_layer_lib,
  name: 'sdl3-layer',
  filebase: 'sdl3-layer',
  description: 'Wayland (wlroots) layer shell for SDL3',
  version: meson.project_version(),
  requires_private: [
    'sdl3',
    'wayland-client'
  ],
)

if get_option('examples') and not meson.is_subproject()
  subdir('examples')
endif
