
#example:  cd BUILD
# meson compile && meson test --suite module:fontRendering  --verbose --setup valgrind -j 1


# c_args is defined in ../../lib/meson.build

common_with = [built_libs['debug']]


tests = {

  'FC_intro': {
    'no-interactive': true,
    'dependencies': [libfontconfig_dep]
  },
  'FC_intro_WithPattern': {
    'sources': ['FC_intro.c'],
    'c_args': [ '-DPATTERN="Mono"' ],
    'no-interactive': true,
    'dependencies': [libfontconfig_dep]
  },
  'FT_intro': {
    'no-interactive': true,
    'dependencies': [libfontconfig_dep]
  },

}

# The following code is very much like ../panels/meson.build
#
########################################################################
# This makes two tests per test listed in tests:
# one that is non-interactive and (optionally) one that is interactive
#
foreach name, t: tests

  link_with = common_with
  if t.has_key('link_with')
    link_with = t.get('link_with')
  endif

  dependencies = []
  if t.has_key('dependencies')
    dependencies = t.get('dependencies')
  endif

  sources =  name + '.c'
  if t.has_key('sources')
    sources = t.get('sources')
  endif

  add_c_args = []
  if t.has_key('add_c_args')
    add_c_args = t.get('add_c_args')
  endif

  n = 'fontRending_' + name

  if t.has_key('no-non_interactive')
  else
    # Non-interactive GUI test
    e = executable(n, sources,
      link_with: link_with,
      c_args: c_args + add_c_args,
      dependencies: dependencies)
    test(n, e, suite: [ 'fontRendering', 'non_interactive', 'all' ])
  endif

  if t.has_key('no-interactive')
    # Skip building the interactive version of this test.
    continue
  endif

  # Interactive GUI test
  n = n + '_int'
  e = executable(n, sources,
    link_with: link_with,
    c_args: c_args + add_c_args + [ '-DRUN'],
    dependencies: dependencies)
  test(n, e, suite: [ 'fontRendering', 'interactive', 'all' ], timeout: 0)
endforeach


tests = { }
common_with = []

