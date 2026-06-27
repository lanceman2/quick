
# c_args is defined in ../../lib/meson.build


tests = {

  '002_quickdraw': {
    'deps': [built_libs['quickdraw']]
  },

  '005_debug' : {
    'deps': [built_libs['quickdraw'], built_libs['debug']]
  },

  '010_debug_diff_link' : {
    'deps': [built_libs['quickdraw'], built_libs['debug']]
  }
}

foreach name, t: tests
  e = executable(name, name + '.c', link_with: t['deps'], c_args: c_args)
  test(name, e, suite: [ 'quickdraw', 'non_interactive', 'all' ])
endforeach

tests = { }

