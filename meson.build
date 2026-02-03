project('girara', 'c',
  version: '2026.02.03',
  meson_version: '>=1.5',
  default_options: ['c_std=c17', 'warning_level=3'],
)

version = meson.project_version()
version_array = version.split('.')

# Rules for so_major and so_minor:
# Before a release perform the following checks against the last release:
# * If a function has been removed or the paramaters of a function have changed
#   bump SOMAJOR and set SOMINOR to 0.
# * If any of the exported datastructures have changed in a incompatible way
#   bump SOMAJOR and set SOMINOR to 0.
# * If a function has been added bump SOMINOR.
so_major = '5'
so_minor = '0'
so_version = '@0@.@1@'.format(so_major, so_minor)

conf_data = configuration_data()
conf_data.set('GVMAJOR', version_array[0])
conf_data.set('GVMINOR', version_array[1])
conf_data.set('GVREV', version_array[2])
conf_data.set('version', version)

cc = meson.get_compiler('c')

prefix = get_option('prefix')

# required attributes
cc.compiles('''#if !__has_attribute(cleanup)
#error attribute cleanup not supported
#endif''', name: '__attribute__(cleanup)', required: true)

# required dependencies
libm = cc.find_library('m', required: false)
glib = dependency('glib-2.0', version: '>=2.72')
gobject = dependency('gobject-2.0', version: '>=2.72')
gio = dependency('gio-2.0', version: '>=2.72')

build_dependencies = [libm, glib, gobject, gio]
pc_requires = ['glib-2.0', 'gobject-2.0', 'gio-2.0']

# supported functions
has_getpwnam_r = cc.has_function('getpwnam_r', prefix: '#define _DEFAULT_SOURCE\n#include <sys/types.h>\n#include <pwd.h>')

# defines
defines = [
  '-D_DEFAULT_SOURCE',
]

if has_getpwnam_r
  defines += ['-DHAVE_GETPWNAM_R']
endif

# compile flags
flags = [
  '-Wmissing-declarations',
  '-Werror=implicit-function-declaration',
  '-Werror=vla',
  '-Werror=int-conversion',
  '-Werror=maybe-uninitialized'
]
flags = cc.get_supported_arguments(flags)

# generate version header file
version_header = configure_file(
  input: 'girara/version.h.in',
  output: 'girara-version.h',
  configuration: conf_data
)
include_directories = [
  include_directories('.')
]

# source files
sources = files(
  'girara/datastructures.c',
  'girara/input-history-io.c',
  'girara/input-history.c',
  'girara/log.c',
  'girara/template.c',
  'girara/utils.c'
)

# header files to install
headers = files(
  'girara/datastructures.h',
  'girara/girara.h',
  'girara/input-history.h',
  'girara/log.h',
  'girara/macros.h',
  'girara/utils.h'
)
headers += version_header

# girara library
girara = library(
  'girara',
  sources,
  dependencies: build_dependencies,
  version: so_version,
  install: true,
  include_directories: include_directories,
  c_args: defines + flags,
  gnu_symbol_visibility: 'hidden'
)
install_headers(headers, subdir: 'girara')

# pkg-config file
pkg = import('pkgconfig')
pkg.generate(
  name: 'girara',
  description: 'Common utilities for zathura',
  url: 'https://pwmt.org/projects/girara',
  version: version,
  libraries: girara,
  requires: pc_requires,
)

girara_dependency = declare_dependency(link_with: girara, include_directories: include_directories)
meson.override_dependency('girara', girara_dependency)

subdir('doc')
subdir('tests')
