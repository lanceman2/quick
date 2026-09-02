
libpng_dep = dependency('libpng')
libcairo_dep = dependency('cairo')
libfontconfig_dep = dependency('fontconfig')
libwayland_cursor_dep = dependency('wayland-cursor')
libfreetype_dep = dependency('freetype2')
libfftw3_dep = dependency('fftw3')

cc = meson.get_compiler('c')
libm_dep = cc.find_library('m', required : true)

# Compiler options for phtreads seems to be always changing.
thread_dep = dependency('threads')


c_args = []

c_args += '-DWITH_CAIRO'
c_args += '-DWITH_FONTCONFIG'
c_args += '-Werror'
c_args += '-Wall' # Looks like -Wall is already there by default.

if get_option('DEBUG')
  c_args += '-DDEBUG'
  c_args += '-g'
  c_args += '-ggdb'
endif


##################################################################
#  Wayland Client Stuff
#
# Reference: the source to gdk from https://github.com/GNOME/gtk.git
# was helpful to look at.
#
#  WTF: Why is there a simple library pkg-config installation of
#  "wayland-cursor", but "xdg-shell" uses this bull shit?
#
wl_dep = dependency('wayland-client')
wl_mod = import('unstable-wayland')
wl_headers = []
wl_sources = []

proto_sources = [
  {
    'name': 'xdg-shell',
    'stability': 'stable'
  },
  {
    'name': 'xdg-decoration',
    'stability': 'unstable',
    'version': 1
  }
]


foreach p : proto_sources
  if(p.has_key('version'))
    pfile = wl_mod.find_protocol(p.get('name'),
            state: p.get('stability'),
            version: p.get('version'))
  else
    pfile = wl_mod.find_protocol(p.get('name'),
            state: p.get('stability'))
  endif

  gen = wl_mod.scan_xml(pfile)

  assert(gen.length() == 2)
  wl_sources += gen[0]
  wl_headers += gen[1]
endforeach

# End Wayland Client Stuff
##################################################################


# This is an array list of libraries (dictionary) to build and install
# along with any particular parameters for each library.  This may crap
# out if we get too many libraries, but as of now this works.


libs = {

  'debug': {
    'dependencies': [ ],
    'link_with': [],
    'version': '0.0.1',
    'description': 'CPP macro debugging with DSPEW(), ASSERT(), and the like'
  },

  'cmdloptHelp': {
    'dependencies': [ ],
    'link_with': ['debug'],
    'version': '0.0.1',
    'description': 'parsing program command-line main() arguments'
  },

  'quickdraw': {
    'dependencies': [ libm_dep, thread_dep],
    # We will have this 'debug' lib object after it is built in built_libs
    'link_with': [ 'debug' ],
    'version': '0.0.1',
    'description': 'direct drawing to uint32_t array buffers'
  },

  'text': {
    'dependencies': [ libfontconfig_dep ],
    'link_with': [ 'debug', 'quickdraw' ],
    'version': '0.0.1',
    'description': 'libfontconfig and libfreetype wrapper'
  },

  'panels': {
    'dependencies': [libm_dep, wl_dep, libcairo_dep,
      libfontconfig_dep, libwayland_cursor_dep],
    'add_sources': [wl_sources, wl_headers],
    # 'dependencies': [ libm_dep, thread_dep],
    # We will have this 'debug' lib object after it is built in built_libs
    'link_with': [ 'debug', 'quickdraw', 'text' ],
    'version': '0.0.1',
    'description': 'GUI widget API'
  },

  'quickdraw_png': {
    'dependencies': [ libpng_dep ],
    'link_with': [ 'debug' ],
    'version': '0.0.1',
    'description': 'private libquickdraw helper library'
  },

  'quickstream': {
    'dependencies': [ libpng_dep ],
    'link_with': [ 'debug' ],
    'version': '0.0.1',
    'description': 'builds programs with blocks'
  },

  #, add more objects here like above

}

# We keep a list of the built libraries so we can link executables and
# libs that are already built in this software package to them.
#
built_libs = {}


foreach name, libobj: libs
  # Most build systems do not like wildcard generated file lists.
  # This is a very cheesy way to do file globing like: echo dir/*.c
  deps = libobj['dependencies']
  link_with = []
  if libobj.has_key('link_with')
    foreach link: libobj['link_with']
      link_with += built_libs[link]
    endforeach
  endif
  libname = 'lib' + name
  sources = [ run_command('glob_cfiles.bash',
    name, check: true).stdout().strip().split()]
  if libobj.has_key('add_sources')
    sources = [ libobj['add_sources'], sources ]
  endif
  #message('sources =', sources)
  assert(sources.length() >= 0, 'sources.length < 0 for ' + libname)
  built_libs += { name: shared_library(
    name,
    sources,
    include_directories: include_directories(join_paths('..', 'include')),
    gnu_symbol_visibility: 'hidden',
    c_args: c_args + ['-D' + name + '_BUILD_LIB'],
    # FIXME: This is likely only good for GCC compiler
    #link_args: '-Wl,--retain-symbols-file=' +
    #  join_paths(meson.project_source_root(), 'lib', name, 'retain-symbols.txt'),
    dependencies : libobj['dependencies'],
    link_with: link_with,
    install: true) }

  # Done, just printing a message:
  deps = libobj['description']
  if libobj['dependencies'].length() > 0
    deps += ' with dependencies:'
    foreach dep: libobj['dependencies']
      deps += ' ' + dep.name()
    endforeach
  endif
  message('Setup target(' + libname + ')', deps)
endforeach


subdir('pkgconfig')
subdir('quickplot/misc')
subdir('quickplot/template')
subdir('Tuner/misc')

