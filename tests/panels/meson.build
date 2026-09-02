
# c_args is defined in ../../lib/meson.build

common_with = [built_libs['debug'], built_libs['panels']]


tests = {

  'window': { },
  'windows': { },
  'reinit_singleton': { },
  'popup': { },
  'popup_showHideShow': { },
  'widget': { },
  'widgets': { },
  'widgets_cull': {
    'sources': ['widgets.c'],
    'add_c_args': ['-DCULL'] },
  'rand_widgets': { },
  'rand_widgets_cull': {
    'sources': ['rand_widgets.c'],
    'add_c_args': ['-DCULL'] },
  'align_widget': { },
  'draw_widget': { },
  'orphans': { },
  'orphansNoWin': { 'no-interactive': true },
  'cursor': { },
  'hsplitter': {
    'sources': ['splitter.c']},
  'vsplitter': {
    'sources': ['splitter.c'],
    'add_c_args': ['-DVERTICAL'] },
  'findFont': { 'no-interactive': true },
  'myCairoDrawWidget': {
    'dependencies': libcairo_dep },
  'cairoDrawWidget': {
    'dependencies': libcairo_dep },
  'scribble': { },
  'button': { },
  'togglebutton': {
    # FIXME: the colors in this suck
    'sources': ['button.c'],
    'add_c_args': ['-DTOGGLE'] },
  'cairo_joy': {
    'link_with': [],
    'dependencies': [libcairo_dep, libfontconfig_dep] },
  'toggleCheck': { },
  'CairoTextSizeFit': {
    'link_with': [],
    'dependencies': [libcairo_dep, libfontconfig_dep],
    'no-interactive': true },
  'fontconfig': {
    'dependencies': [libcairo_dep, libfontconfig_dep] },
  # load_fontconfig loads a test DSO file
  'load_fontconfig': {
    'dependencies': [libcairo_dep, libfontconfig_dep],
    'link_with': [built_libs['debug']] },
  # Like above but checks more stuff.
  'loadAndUnloadCheckMaps': {
    'dependencies': [libcairo_dep, libfontconfig_dep],
    'link_with': [built_libs['debug']] },
  'label': { },
  'entry': { },
  'labeledButton': { },
  'reinit_singleton_CAIRO': {
    'sources': ['reinit_singleton.c'],
    'add_c_args': ['-DCAIRO'] },
  'gridWindow': { },
  'image': { },
  'gridSpan': { },
  'gridSpan_cull': {
    'sources': ['gridSpan.c'],
    'add_c_args': ['-DCULL'] },
  'GridWithNoChildren': { },
  'menuBar': { },
  'generic': { },
  'check': { },
  'graph': { 
    'dependencies': [libm_dep] },
  'graph4': { 
    'dependencies': [libm_dep] },
  'graph2_hsplitter': { 
    'dependencies': [libm_dep] },
  'graphCairoDrawOver': {
    'dependencies': [libcairo_dep] },
  'graphDrawOver': { },
  'scopeWithMicFFT': {
    'no-non_interactive': true,
    'dependencies': [libm_dep, wl_dep, libfftw3_dep] },
  'scope3stringTune': {
    'no-non_interactive': true,
    'dependencies': [libm_dep, wl_dep] },
  'scope': {
    'no-non_interactive': true,
    'dependencies': [libm_dep] },
  'windowSizingButtons': { },
  'key_events': { }
}


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

  n = 'pn_' + name

  if t.has_key('no-non_interactive')
  else
    # Non-interactive GUI test
    e = executable(n, sources,
      link_with: link_with,
      c_args: c_args + add_c_args,
      dependencies: dependencies)
    test(n, e, suite: [ 'panels', 'non_interactive', 'all' ])
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
  test(n, e, suite: [ 'panels', 'interactive', 'all' ], timeout: 0)
endforeach


# This DSO is loaded in a test non_interactive program
shared_library('fontconfig',
  install: false,
  c_args: c_args,
  sources: [ 'fontconfig.c' ],
  link_with: common_with,
  dependencies: [libcairo_dep, libfontconfig_dep])


# This DSO is loaded in a test interactive program
shared_library('fontconfig_int',
  install: false,
  c_args: c_args + ['-DRUN'],
  sources: ['fontconfig.c'],
  link_with: common_with,
  dependencies: [libcairo_dep, libfontconfig_dep])


tests = { }
common_with = []

