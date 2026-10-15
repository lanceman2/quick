
executable('MicToScope',
  ['MicToScope.c'],
  c_args: c_args,
  install: true,
  link_with: [built_libs['panels'], built_libs['debug']],
  dependencies: [header_deps['panels.h']],
  install_rpath: join_paths('$ORIGIN', '..', rel_libdir) )

gen_src = custom_target('quickplotOptions.h',
  depends: quickplotHelp,
  # FIXME: I expect this is not portable.
  command: [ 'sh', '-c',
    join_paths('.', 'lib', 'quickplot', 'misc', 'quickplotHelp') +
    ' -c > ' + join_paths('bin', 'quickplotOptions.h')],
  output: ['quickplotOptions.h'])

executable('quickplot',
  # It also depends on the generated file: options.h
  ['quickplot.c', 'findGrapher.c', 'cmdlopt.c', gen_src],
  c_args: c_args,
  install: true,
  link_with: [built_libs['panels'], built_libs['debug']],
  dependencies: [header_deps['panels.h']],
  install_rpath: join_paths('$ORIGIN', '..', rel_libdir) )


gen_src = custom_target('TunerOptions.h',
  depends: TunerHelp,
  # FIXME: I expect this is not portable.
  command: [ 'sh', '-c',
    join_paths('.', 'lib', 'Tuner', 'misc', 'TunerHelp') +
    ' -c > ' + join_paths('bin', 'TunerOptions.h')],
  output: ['TunerOptions.h'])

executable('Tuner',
  # It also depends on the generated file: options.h
  ['Tuner.c', 'cmdlopt.c', gen_src],
  c_args: c_args,
  install: true,
  link_with: [built_libs['debug'], built_libs['panels'],],
  dependencies: [libm_dep],
  install_rpath: join_paths('$ORIGIN', '..', rel_libdir) )

WL_list_registry = executable('WL_list_registry',
  ['WL_list_registry.c'],
  c_args: c_args,
  install: true,
  link_with: [built_libs['debug']],
  dependencies: dependency('wayland-client'),
  install_rpath: join_paths('$ORIGIN', '..', rel_libdir) )


# Don't pollute the variable name space and/or make this crash if another
# meson.build file tries to use it.
gen_src = ''


name = 'MeasureArc'
install_data(name, install_dir: get_option('bindir'))
# Note: If no programs are run from the build directory then this is not
# doing anything for us, and so these "built files" are superfluous.
custom_target(name,
    install: false,
    command: [ 'cp' , join_paths(meson.current_source_dir(), name),
      meson.current_build_dir() ],
    output: name,
    # This seems to be required:
    build_by_default: true)
# Remove (kind-of) meson tmp variable "name" so we don't pollute meson
# name-space.
name = ''

