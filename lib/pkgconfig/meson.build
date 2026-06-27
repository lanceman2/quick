
pkg = import('pkgconfig')


foreach name, parameters: libs
  pkg.generate(
    libraries : built_libs[name],
    version : parameters['version'],
    name : name,
    filebase : name,
    description : parameters['description'])
endforeach

