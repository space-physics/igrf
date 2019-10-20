project('igrf12', 'c',
  default_options : ['default_library=static', 'buildtype=release', 'warning_level=0'])

geomag7 = executable('geomag7', 'src/geomag70.c')

if add_languages('fortran', required: false)
  igrf12 = library('igrf12', 'src/igrf12.f')
  igrf12_exe = executable('igrf12_run', 'src/igrf12_demoprog.f',
    link_with: igrf12)
endif