project('girara', 'c',
  version: '2026.01.30',
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
localedir = get_option('localedir')

# required attributes
cc.compiles('''#if !__has_attribute(cleanup)
#error attribute cleanup not supported
#endif''', name: '__attribute__(cleanup)', required: true)

# required dependencies
libm = cc.find_library('m', required: false)
glib = dependency('glib-2.0', version: '>=2.72')
gtk3 = dependency('gtk+-3.0', version: '>=3.24')
pango = dependency('pango', version: '>=1.50')

build_dependencies = [libm, glib, gtk3, pango]
pc_requires = ['glib-2.0', 'gtk+-3.0']

# supported functions
has_getpwnam_r = cc.has_function('getpwnam_r', prefix: '#define _DEFAULT_SOURCE\n#include <sys/types.h>\n#include <pwd.h>')

# defines
defines = [
  '-DGETTEXT_PACKAGE="org.pwmt.girara.gtk3.@0@"'.format(so_major),
  '-DLOCALEDIR="@0@"'.format(join_paths(prefix, localedir)),
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

# optional dependencies
pc_requires_private = ['pango']
jsonc = dependency('json-glib-1.0', required: get_option('json'))

if jsonc.found()
  build_dependencies += jsonc
  defines += '-DWITH_JSON'
  pc_requires_private += 'json-glib-1.0'
endif

# generate version header file
version_header = configure_file(
  input: 'girara/version.h.in',
  output: 'girara-version.h',
  configuration: conf_data
)
include_directories = [
  include_directories('.')
]

subdir('data')
subdir('po')

# source files
sources = files(
  'girara/callbacks.c',
  'girara/commands.c',
  'girara/completion.c',
  'girara/config.c',
  'girara/datastructures.c',
  'girara/entry.c',
  'girara/input-history-io.c',
  'girara/input-history.c',
  'girara/log.c',
  'girara/record.c',
  'girara/session.c',
  'girara/settings.c',
  'girara/shortcuts.c',
  'girara/statusbar.c',
  'girara/template.c',
  'girara/utils.c'
)
sources += girara_css

# header files to install
headers = files(
  'girara/callbacks.h',
  'girara/commands.h',
  'girara/completion.h',
  'girara/config.h',
  'girara/datastructures.h',
  'girara/entry.h',
  'girara/girara.h',
  'girara/input-history.h',
  'girara/log.h',
  'girara/macros.h',
  'girara/record.h',
  'girara/session.h',
  'girara/settings.h',
  'girara/shortcuts.h',
  'girara/statusbar.h',
  'girara/template.h',
  'girara/types.h',
  'girara/utils.h'
)
headers += version_header

# girara library
girara = library(
  'girara-gtk3',
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
  name: 'girara-gtk3',
  description: 'User interface library',
  url: 'https://pwmt.org/projects/girara',
  version: version,
  libraries: girara,
  requires: pc_requires,
  requires_private: pc_requires_private,
)

girara_dependency = declare_dependency(link_with: girara, include_directories: include_directories)
meson.override_dependency('girara-gtk3', girara_dependency)

subdir('doc')
subdir('tests')
