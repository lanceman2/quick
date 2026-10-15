
# We need to copy C files to the BUILD directory so that they can be
# accessed by the package programs before they are installed.

installed_template_cfiles = run_command(
  join_paths('..', '..', 'glob_cfiles.bash'),
    check: true).stdout().strip().split()
template_install_dir = join_paths(get_option('libdir'), 'quickplot', 'template')

foreach name: installed_template_cfiles
  message('making: ' + name)
  custom_target('quickplot_c_templates_' + name,
    command: [ 'cp' , join_paths(meson.current_source_dir(), name),
      meson.current_build_dir() ],
    output: name,
    input: name,
    install: true,
    install_dir: template_install_dir)
endforeach

# Clean meson namespaces
installed_template_cfiles = ''
template_install_dir = ''

