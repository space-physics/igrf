program igrf_driver
!! CLI interface to IGRF13, 12, 11
!! Michael Hirsch

!     colat = colatitude (0-180)
!     elong = east-longitude (0-360)
use, intrinsic :: iso_fortran_env, only : real64, stdout=>output_unit, stderr=>error_unit
implicit none (type, external)

external :: igrf13syn, igrf12syn, igrf11syn

character(256) :: argv, in_nml, out_nml
integer :: i, model
integer, parameter :: itype = 1, &  !< geodetic
    isv = 0 !< main field
real(real64) :: date, alt, clt, xln, x, y, z, f

namelist /in/ model,date, alt, clt, xln
namelist /out/ x, y, z, f

call get_command_argument(1, argv, status=i)
if(i/=0) error stop 'specify input namelist file'
read(argv,'(a256)') in_nml

call get_command_argument(2, argv, status=i)
if(i/=0) error stop 'specify output namelist file'
read(argv,'(a256)') out_nml

open(newunit=i, file=in_nml, action="read")
read(i, nml=in)
close(i)

select case (model)
case (13)
  CALL IGRF13SYN(isv,DATE,ITYPE,ALT,CLT,XLN,X,Y,Z,F)
case (12)
    CALL IGRF12SYN(isv,DATE,ITYPE,ALT,CLT,XLN,X,Y,Z,F)
case (11)
    CALL IGRF11SYN(isv,DATE,ITYPE,ALT,CLT,XLN,X,Y,Z,F)
case default
    write(stderr,*) 'unknown model ', model
    error stop
end select

open(newunit=i, file=out_nml, action="write")
write(i, nml=out)
close(i)

end program
