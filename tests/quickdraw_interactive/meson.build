
# c_args is defined in ../../lib/meson.build


tests = {

  '003_simple_show_png' : {
    'deps': [built_libs['quickdraw'], built_libs['debug']]
  },

  '005_rectangle' : {
    'deps': [built_libs['quickdraw'], built_libs['debug']]
  },

  '007_hort_vert_lines' : {
    'deps': [built_libs['quickdraw'], built_libs['debug']]
  },

  '012_lines' : {
    'deps': [built_libs['quickdraw'], built_libs['debug']]
  }

}

foreach name, t: tests
  e = executable(name, name + '.c', link_with: t['deps'], c_args: c_args)
  test(name, e, suite: [ 'quickdraw', 'interactive', 'all' ], timeout: 0)
endforeach

tests = { }

