project('igrf', 'c',
  default_options : ['default_library=static', 'buildtype=release', 'warning_level=0'])

geomag7 = executable('geomag7', 'geomag70.c')

if add_languages('fortran', required: false)
  igrf13 = library('igrf13', 'igrf13.f')
  igrf13_exe = executable('igrf13_run', 'igrf13_driver.f',
    link_with: igrf13)

  igrf12 = library('igrf12', 'igrf12.f')
  igrf12_exe = executable('igrf12_run', 'igrf12_driver.f',
    link_with: igrf12)
endif
