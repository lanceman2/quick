
# A list of all the library header files with some particulars, if any are
# needed.
#
headers = { 
  'quickdraw': {},
  'debug': {},
  'panels': {},
  'panels_drawingUtils': {},
  'text': {}
  #, add more objects like above
}

header_deps = {}


foreach name, obj: headers
  header = name + '.h'
  install_headers(header)
  #
  # This is a little kludgey.  Meson forces us to do strange things.
  #
  # We wish to make this software usable when it is not installed so:
  #
  # Copy the from the source to the build.  We (may or may not) need it
  # for running and or testing code before anything is installed.  We have
  # programs that compile code (using header files) when they run, be they
  # installed or not.
  #
  # Note: If no programs are run from the build directory then this is not
  # doing anything for us, and so these "built files" are superfluous.
  #
  target = custom_target('header_' + header,
    # The header files are installed from the install_header() call above.
    install: false,
    command: [ 'cp' , join_paths(meson.current_source_dir(), header),
      meson.current_build_dir() ],
    depend_files: [header],
    output: header,
    # This seems to be required:
    build_by_default: true)

  header_deps += { header: declare_dependency(sources : target) }

endforeach

