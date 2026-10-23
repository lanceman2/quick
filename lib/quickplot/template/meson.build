
# We need to copy C files to the BUILD directory so that they can be
# accessed by the package programs before they are installed.

installed_template_cfiles = run_command(
  join_paths('..', '..', 'glob_cfiles.bash'),
    check: true).stdout().strip().split()
template_install_dir = join_paths(get_option('libdir'), 'quickplot', 'template')

foreach name_c: installed_template_cfiles

  message('making quickplot plugin template: ' + name_c)
  # Make sure that we are not reusing a library name.
  dso_name = name_c.replace('.c', '')
  assert(not built_libs.has_key(dso_name))

  custom_target('quickplot_c_templates_' + name_c,
    command: [ 'cp' , join_paths(meson.current_source_dir(), name_c),
      meson.current_build_dir() ],
    output: name_c,
    input: name_c,
    install: true,
    install_dir: template_install_dir)

  # "built_libs" started life in ../../lib/meson.build
  built_libs += { dso_name: shared_library(
    dso_name,
    name_c,
    include_directories: include_directories(join_paths('..', '..', '..', 'include')),
    gnu_symbol_visibility: 'hidden',
    c_args: c_args + ['-D' + dso_name + '_BUILD_LIB'],
    dependencies : libm_dep,
    link_with: [built_libs['debug'], built_libs['panels']],
    name_prefix: '',
    install_dir: template_install_dir,
    install: true) }
endforeach

# Clean meson namespaces
installed_template_cfiles = ''
template_install_dir = ''

