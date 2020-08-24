program igrf_driver

use igrf, only : igrf13syn
use, intrinsic:: iso_fortran_env, only: stderr=>error_unit, dp=>real64

implicit none (type, external)

real(dp) :: yeardec  !< decimal year e.g. 2015.82
real(dp) :: utsec !< UTC second of day
real(dp) :: alt_km  !< altitude [km]
real(dp) :: glat, glon !< geodetic latitude, longitude [deg]
integer :: isv, itype

character(80) :: argv

real(dp) :: x, y, z, f

! --- command line input
if (command_argument_count() /= 6) then
  write(stderr,*) 'need input parameters: YearDecimal glat glon alt_km isv itype'
  stop 1
endif

call get_command_argument(1, argv)
read(argv,*) yeardec

call get_command_argument(2, argv)
read(argv,*) glat

call get_command_argument(3, argv)
read(argv,*) glon

call get_command_argument(4, argv)
read(argv,*) alt_km

call get_command_argument(5, argv)
read(argv, '(I1)') isv

call get_command_argument(6, argv)
read(argv, '(I1)') itype

! --- execute program
call igrf13syn(isv=isv, date=yeardec, itype=itype, alt=alt_km, colat=90. - glat, elong=glon, &
  x=x, y=y, z=z, f=f)

print '(4ES15.7)', x, y, z, f

end program
