/****************************************************************************/
/*                                                                          */
/*     NGDC's Geomagnetic Field Modeling software for the IGRF and WMM      */
/*                                                                          */
/****************************************************************************/
/*                                                                          */
/*     Disclaimer: This program has undergone limited testing. It is        */
/*     being distributed unoffically. The National Geophysical Data         */
/*     Center does not guarantee it's correctness.                          */
/*                                                                          */
/****************************************************************************/
/*                                                                          */
/*     Version 7.0:                                                         */
/*     - input file format changed to                                       */
/*            -- accept new DGRF2005 coeffs with 0.01 nT precision          */
/*            -- make sure all values are separated by blanks               */
/*            -- swapped n and m: first is degree, second is order          */
/*     - new my_isnan function improves portablility                        */
/*     - corrected feet to km conversion factor                             */
/*     - fixed date conversion errors for yyyy,mm,dd format                 */
/*     - fixed lon/lat conversion errors for deg,min,sec format             */
/*     - simplified leap year identification                                */
/*     - changed comment: units of ddot and idot are arc-min/yr             */
/*     - added note that this program computes the secular variation as     */
/*            the 1-year difference, rather than the instantaneous change,  */
/*            which can be slightly different                               */
/*     - clarified that height is above ellipsoid, not above mean sea level */
/*            although the difference is negligible for magnetics           */
/*     - changed main(argv,argc) to usual definition main(argc,argv)        */
/*     - corrected rounding of angles close to 60 minutes                   */
/*     Thanks to all who provided bug reports and suggested fixes           */
/*                                                                          */
/*                                          Stefan Maus Jan-25-2010         */
/*                                                                          */
/****************************************************************************/
/*                                                                          */
/*     Version 6.1:                                                         */
/*     Included option to read coordinates from a file and output the       */
/*     results to a new file, repeating the input and adding columns        */
/*     for the output                                                       */
/*                                          Stefan Maus Jan-31-2008         */
/*                                                                          */
/****************************************************************************/
/*                                                                          */
/*     Version 6.0:                                                         */
/*     Bug fixes for the interpolation between models. Also added warnings  */
/*     for declination at low H and corrected behaviour at geogr. poles.    */
/*     Placed print-out commands into separate routines to facilitate       */
/*     fine-tuning of the tables                                            */
/*                                          Stefan Maus Aug-24-2004         */
/*                                                                          */
/****************************************************************************/
/*                                                                          */
/*      This program calculates the geomagnetic field values from           */
/*      a spherical harmonic model.  Inputs required by the user are:       */
/*      a spherical harmonic model data file, coordinate preference,        */
/*      altitude, date/range-step, latitude, and longitude.                 */
/*                                                                          */
/*         Spherical Harmonic                                               */
/*         Model Data File       :  Name of the data file containing the    */
/*                                  spherical harmonic coefficients of      */
/*                                  the chosen model.  The model and path   */
/*                                  must be less than PATH chars.           */
/*                                                                          */
/*         Coordinate Preference :  Geodetic (WGS84 latitude and altitude   */
/*                                  above ellipsoid (WGS84),                */
/*                                  or geocentric (spherical, altitude      */
/*                                  measured from the center of the Earth). */
/*                                                                          */
/*         Altitude              :  Altitude above ellipsoid (WGS84). The   */
/*                                  program asks for altitude above mean    */
/*                                  sea level, because the altitude above   */
/*                                  ellipsoid is not known to most users.   */
/*                                  The resulting error is very small and   */
/*                                  negligible for most practical purposes. */
/*                                  If geocentric coordinate preference is  */
/*                                  used, then the altitude must be in the  */
/*                                  range of 6370.20 km - 6971.20 km as     */
/*                                  measured from the center of the earth.  */
/*                                  Enter altitude in kilometers, meters,   */
/*                                  or feet                                 */
/*                                                                          */
/*         Date                  :  Date, in decimal years, for which to    */
/*                                  calculate the values of the magnetic    */
/*                                  field.  The date must be within the     */
/*                                  limits of the model chosen.             */
/*                                                                          */
/*         Latitude              :  Entered in decimal degrees in the       */
/*                                  form xxx.xxx.  Positive for northern    */
/*                                  hemisphere, negative for the southern   */
/*                                  hemisphere.                             */
/*                                                                          */
/*         Longitude             :  Entered in decimal degrees in the       */
/*                                  form xxx.xxx.  Positive for eastern     */
/*                                  hemisphere, negative for the western    */
/*                                  hemisphere.                             */
/*                                                                          */
/****************************************************************************/
/*                                                                          */
/*      Subroutines called :  degrees_to_decimal,julday,getshc,interpsh,    */
/*                            extrapsh,shval3,dihf,safegets                 */
/*                                                                          */
/****************************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <math.h>

int my_isnan(double d)
{
  return (d != d);              /* IEEE: only NaN is not equal to itself */
}

#define NaN log(-1.0)
#define FT2KM (1.0/0.0003048)
#define PI 3.141592654
#define RAD2DEG (180.0/PI)

#ifndef SEEK_SET
#define SEEK_SET 0
#define SEEK_CUR 1
#define SEEK_END 2
#endif

#define IEXT 0
#define FALSE 0
#define TRUE 1                  /* constants */
#define RECL 81

#define MAXINBUFF RECL+14

/** Max size of in buffer **/

#define MAXREAD MAXINBUFF-2
/** Max to read 2 less than total size (just to be safe) **/

#define MAXMOD 30
/** Max number of models in a file **/

#define PATH MAXREAD
/** Max path and filename length **/

#define EXT_COEFF1 (double)0
#define EXT_COEFF2 (double)0
#define EXT_COEFF3 (double)0

#define MAXDEG 13
#define MAXCOEFF (MAXDEG*(MAXDEG+2)+1) /* index starts with 1!, (from old Fortran?) */
double gh1[MAXCOEFF];
double gh2[MAXCOEFF];
double gha[MAXCOEFF];              /* Geomag global variables */
double ghb[MAXCOEFF];
double d=0,f=0,h=0,i=0;
double dtemp,ftemp,htemp,itemp;
double x=0,y=0,z=0;
double xtemp,ytemp,ztemp;

FILE *stream = NULL;                /* Pointer to specified model data file */


/****************************************************************************/
/*                                                                          */
/*                             Program Geomag                               */
/*                                                                          */
/****************************************************************************/
/*                                                                          */
/*      This program, originally written in FORTRAN, was developed using    */
/*      subroutines written by                                              */
/*      A. Zunde                                                            */
/*      USGS, MS 964, Box 25046 Federal Center, Denver, Co.  80225          */
/*      and                                                                 */
/*      S.R.C. Malin & D.R. Barraclough                                     */
/*      Institute of Geological Sciences, United Kingdom.                   */
/*                                                                          */
/*      Translated                                                          */
/*      into C by    : Craig H. Shaffer                                     */
/*                     29 July, 1988                                        */
/*                                                                          */
/*      Rewritten by : David Owens                                          */
/*                     For Susan McLean                                     */
/*                                                                          */
/*      Maintained by: Adam Woods                                           */
/*      Contact      : geomag.models@noaa.gov                               */
/*                     National Geophysical Data Center                     */
/*                     World Data Center-A for Solid Earth Geophysics       */
/*                     NOAA, E/GC1, 325 Broadway,                           */
/*                     Boulder, CO  80303                                   */
/*                                                                          */
/*                                                                          */
/****************************************************************************/
/*                                                                          */
/*      Some variables used in this program                                 */
/*                                                                          */
/*    Name         Type                    Usage                            */
/* ------------------------------------------------------------------------ */
/*                                                                          */
/*   a2,b2      Scalar Double          Squares of semi-major and semi-minor */
/*                                     axes of the reference spheroid used  */
/*                                     for transforming between geodetic or */
/*                                     geocentric coordinates.              */
/*                                                                          */
/*   minalt     Double array of MAXMOD Minimum height of model.             */
/*                                                                          */
/*   altmin     Double                 Minimum height of selected model.    */
/*                                                                          */
/*   altmax     Double array of MAXMOD Maximum height of model.             */
/*                                                                          */
/*   maxalt     Double                 Maximum height of selected model.    */
/*                                                                          */
/*   d          Scalar Double          Declination of the field from the    */
/*                                     geographic north (deg).              */
/*                                                                          */
/*   sdate      Scalar Double          start date inputted                  */
/*                                                                          */
/*   ddot       Scalar Double          annual rate of change of decl.       */
/*                                     (arc-min/yr)                         */
/*                                                                          */
/*   alt        Scalar Double          altitude above WGS84 Ellipsoid       */
/*                                                                          */
/*   epoch      Double array of MAXMOD epoch of model.                      */
/*                                                                          */
/*   ext        Scalar Double          Three 1st-degree external coeff.     */
/*                                                                          */
/*   latitude   Scalar Double          Latitude.                            */
/*                                                                          */
/*   longitude  Scalar Double          Longitude.                           */
/*                                                                          */
/*   gh1        Double array           Schmidt quasi-normal internal        */
/*                                     spherical harmonic coeff.            */
/*                                                                          */
/*   gh2        Double array           Schmidt quasi-normal internal        */
/*                                     spherical harmonic coeff.            */
/*                                                                          */
/*   gha        Double array           Coefficients of resulting model.     */
/*                                                                          */
/*   ghb        Double array           Coefficients of rate of change model.*/
/*                                                                          */
/*   i          Scalar Double          Inclination (deg).                   */
/*                                                                          */
/*   idot       Scalar Double          Rate of change of i (arc-min/yr).    */
/*                                                                          */
/*   igdgc      Integer                Flag for geodetic or geocentric      */
/*                                     coordinate choice.                   */
/*                                                                          */
/*   inbuff     Char a of MAXINBUF     Input buffer.                        */
/*                                                                          */
/*   irec_pos   Integer array of MAXMOD Record counter for header           */
/*                                                                          */
/*   stream  Integer                   File handles for an opened file.     */
/*                                                                          */
/*   fileline   Integer                Current line in file (for errors)    */
/*                                                                          */
/*   max1       Integer array of MAXMOD Main field coefficient.             */
/*                                                                          */
/*   max2       Integer array of MAXMOD Secular variation coefficient.      */
/*                                                                          */
/*   max3       Integer array of MAXMOD Acceleration coefficient.           */
/*                                                                          */
/*   mdfile     Character array of PATH  Model file name.                   */
/*                                                                          */
/*   minyr      Double                  Min year of all models              */
/*                                                                          */
/*   maxyr      Double                  Max year of all models              */
/*                                                                          */
/*   yrmax      Double array of MAXMOD  Max year of model.                  */
/*                                                                          */
/*   yrmin      Double array of MAXMOD  Min year of model.                  */
/*                                                                          */
/****************************************************************************/


int main(int argc, char**argv)
{
#ifdef MAC
  ccommand(argc, argv);
#endif
  /*  Variable declaration  */

  /* Control variables */
  int   again = 1;
  int   decyears = 3;
  int   units = 4;
  int   decdeg = 3;
  int   range = -1;
  int   counter = 0;
  int   warn_H, warn_H_strong, warn_P;

  int   modelI;             /* Which model (Index) */
  int   nmodel;             /* Number of models in file */
  int   max1[MAXMOD];
  int   max2[MAXMOD];
  int   max3[MAXMOD];
  int   nmax;
  int   igdgc=3;
  int   isyear=-1;
  int   ismonth=-1;
  int   isday=-1;
  int   ieyear=-1;
  int   iemonth=-1;
  int   ieday=-1;
  int   ilat_deg=200;
  int   ilat_min=200;
  int   ilat_sec=200;
  int   ilon_deg=200;
  int   ilon_min=200;
  int   ilon_sec=200;
  int   fileline;
  long  irec_pos[MAXMOD];

  int  coords_from_file = 0;
  int arg_err = 0;
  int need_to_read_model = 1;

  char  mdfile[PATH];
  char  inbuff[MAXINBUFF];
  char  model[MAXMOD][9];
  char *begin;
  char *rest;
  char args[7][MAXREAD];
  int iarg;

  char coord_fname[PATH];
  char out_fname[PATH];
  FILE *coordfile,*outfile;
  int iline=0;
  int read_flag;

  double epoch[MAXMOD];
  double yrmin[MAXMOD];
  double yrmax[MAXMOD];
  double minyr;
  double maxyr;
  double altmin[MAXMOD];
  double altmax[MAXMOD];
  double minalt;
  double maxalt;
  double alt=-999999;
  double sdate=-1;
  double step=-1;
  double syr;
  double edate=-1;
  double latitude=200;
  double longitude=200;
  double ddot;
  double fdot;
  double hdot;
  double idot;
  double xdot;
  double ydot;
  double zdot;
  double warn_H_val, warn_H_strong_val;

  /*  Subroutines used  */

  void print_dashed_line();
  void print_long_dashed_line(void);
  void print_header();
  void print_result(double date, double d, double i, double h, double x, double y, double z, double f);
  void print_header_sv();
  void print_result_sv(double date, double ddot, double idot, double hdot, double xdot, double ydot, double zdot, double fdot);
  void print_result_file(FILE *outf, double d, double i, double h, double x, double y, double z, double f,
                         double ddot, double idot, double hdot, double xdot, double ydot, double zdot, double fdot);
  double degrees_to_decimal();
  double julday();
  int   interpsh();
  int   extrapsh();
  int   shval3();
  int   dihf();
  int   safegets(char *buffer,int n);
  int getshc();


  /* Initializations. */

  inbuff[MAXREAD+1]='\0';  /* Just to protect mem. */
  inbuff[MAXINBUFF-1]='\0';  /* Just to protect mem. */

  for (iarg=0; iarg<argc; iarg++)
    if (argv[iarg] != NULL)
      strncpy(args[iarg],argv[iarg],MAXREAD);

  /* printing out version number and header */
  printf("\n\n Geomag v7.0 - Jan 25, 2010 ");

  if ((argc==2)&&((*(args[1])=='h')||(*(args[1])=='?')||(args[1][1]=='?')))
    {
      printf("\n\nUSAGE:\n");
      printf("interactive:     geomag\n");
      printf("command line:    geomag model_file date coord alt lat lon\n");
      printf("coordinate file: geomag model_file f input_file output_file\n");
      printf("or for help:     geomag h (or ? or -? or /?) \n");
      printf("\n");
      printf("Date and location Formats: \n");
      printf("   Date: xxxx.xxx for decimal  (1947.32)\n");
      printf("         YYYY,MM,DD for year, month, day  (1947,10,13)\n");
      printf("         or start_date-end_date (1943.21-1966.11)\n");
      printf("         or start_date-end_date-step (1943.21-1966.11-1.2)\n");
      printf("   Coord: D - Geodetic (WGS84 latitude and altitude above mean sea level)\n");
      printf("          C - Geocentric (spherical, altitude relative to Earth's center)\n");
      printf("   Altitude: Kxxxxxx.xxx for kilometers  (K1000.13)\n");
      printf("             Mxxxxxx.xxx for meters  (m1389.24)\n");
      printf("             Fxxxxxx.xxx for feet  (F192133.73)\n");
      printf("   Lat/Lon: xxx.xxx for decimal  (-76.53)\n");
      printf("            ddd,mm,ss for degrees, minutes, seconds (-60,23,22)\n");
      printf("            (Lat and Lon must be specified in the same format,\n");
      printf("             for ddd,mm,ss format, two commas each are required  \n");
      printf("              and decimals of arc-seconds are ignored)  \n");
      printf("\nRange (only for interactive and command line options): \n");
      printf("   Date and altitude must fit model.\n");
      printf("   Lat: -90 to 90 (Use - to denote Southern latitude.)\n");
      printf("   Lon: -180 to 180 (Use - to denote Westen longitude.)\n");
      printf("   Minutes and Seconds: -59 to 59.\n");
      printf("\nNote for command line option: \n");
      printf("   All arguments are optional but must preserve order.\n");
      printf("   You can only omit arguments AT THE END, not in between.\n");
      printf("   Invalid arguments are assumed to be 0.\n");
      printf("\nNote that this program computes secular variation as the \n");
      printf("   change over the coming year, rather than instantaneous,\n");
      printf("   which can be different for declination near magnetic poles.\n");
      exit(2);
    } /* help */

  if ((argc==5)&&(*(args[2])=='f'))
    {
      printf("\n\n 'f' switch: converting file with multiple locations.\n");
      printf("     The first five output columns repeat the input coordinates.\n");
      printf("     Then follows D, I, H, X, Y, Z, and F.\n");
      printf("     Finally the SV: dD, dI, dH, dX, dY, dZ,  and dF\n");
      printf("     The units are the same as when the program is\n");
      printf("     run in command line or interactive mode.\n\n");
      coords_from_file = 1;
      strncpy(coord_fname,args[3],MAXREAD);
      coordfile=fopen(coord_fname, "rt");
      strncpy(out_fname,args[4],MAXREAD);
      outfile=fopen(out_fname, "w");
      fprintf(outfile,"Date Coord-System Altitude Latitude Longitude D_deg D_min I_deg I_min H_nT X_nT Y_nT Z_nT F_nT dD_min dI_min dH_nT dX_nT dY_nT dZ_nT dF_nT\n");
    } /* file option */

  if (argc>=3 && argc !=5 && *(args[2])=='f')
    {
      printf("\n\nERROR in 'f' switch option: wrong number of arguments\n");
    }


  if ((argc==3)||(argc==4)||(argc==6))
    {
      printf("\n\nUSAGE:\n");
      printf("interactive:     geomag\n");
      printf("command line:    geomag model_file date coord alt lat lon\n");
      printf("coordinate file: geomag model_file f input_file output_file\n");
      printf("or for help:     geomag h \n\n");
    } /* missing arguments (not fatal) */

  while (again == 1)
    {
      if (coords_from_file)
        {
          argc = 7;
          read_flag = fscanf(coordfile,"%s%s%s%s%s%*[^\n]",args[2],args[3],args[4],args[5],args[6]);
          if (read_flag == EOF) goto reached_EOF;
          fprintf(outfile,"%s %s %s %s %s ",args[2],args[3],args[4],args[5],args[6]);fflush(outfile);
          iline++;
        } /* coords_from_file */

      /* Switch on how many arguments are supplied. */
      /* Note that there are no 'breaks' after the cases, so these are entry points */
      switch(argc)
        {
        case 7 : strncpy(inbuff, args[6], MAXREAD);
          if ((rest=strchr(inbuff, ',')))     /* If it contains a comma */
            {
              decdeg=2;                        /* Then not decimal degrees */
              begin=inbuff;
              rest[0]='\0';                    /* Chop into sub string */
              rest++;                          /* Move to next substring */
              ilon_deg=atoi(begin);
              begin=rest;
              if ((rest=strchr(begin, ',')))
                {
                  rest[0]='\0';
                  rest++;
                  ilon_min=atoi(begin);
                  ilon_sec=atoi(rest);
                }
              else
                {
                  ilon_min=0;
                  ilon_sec=0;
                }
            }
          else
            {
              decdeg=1;                        /* Else it's decimal */
              longitude=atof(args[6]);
            }

        case 6 : strncpy(inbuff, args[5], MAXREAD);
          if ((rest=strchr(inbuff, ',')))
            {
              decdeg=2;
              begin=inbuff;
              rest[0]='\0';
              rest++;
              ilat_deg=atoi(begin);
              begin=rest;
              if ((rest=strchr(begin, ',')))
                {
                  rest[0]='\0';
                  rest++;
                  ilat_min=atoi(begin);
                  ilat_sec=atoi(rest);
                }
              else
                {
                  ilat_min=0;
                  ilat_sec=0;
                }
            }
          else
            {
              decdeg=1;
              latitude=atof(args[5]);
            }

        case 5 : strncpy(inbuff, args[4], MAXREAD);
          inbuff[0]=toupper(inbuff[0]);
          if (inbuff[0]=='K') units=1;
          else if (inbuff[0]=='M') units=2;
          else if (inbuff[0]=='F') units=3;
          if (strlen(inbuff)>1)
            {
              inbuff[0]='\0';
              begin=inbuff+1;
              alt=atof(begin);
            }

        case 4 : strncpy(inbuff, args[3], MAXREAD);
          inbuff[0]=toupper(inbuff[0]);
          if (inbuff[0]=='D') igdgc=1;
          else if (inbuff[0]=='C') igdgc=2;

        case 3 : strncpy(inbuff, args[2], MAXREAD);
          if ((rest=strchr(inbuff, '-')))   /* If it contains a dash */
            {
              range = 2;                     /* They want a range */
              rest[0]='\0';                  /* Sep dates */
              rest++;
              begin=rest;
              if ((rest=strchr(begin, '-')))    /* If it contains 2 dashs */
                {
                  rest[0]='\0';                  /* Sep step */
                  rest++;
                  step=atof(rest);               /* Get step size */
                }
              if ((rest=strchr(begin, ',')))    /* If it contains a comma */
                {
                  decyears=2;                    /* It's not decimal years */
                  rest[0]='\0';
                  rest++;
                  ieyear=atoi(begin);
                  begin=rest;
                  if ((rest=strchr(begin, ',')))
                    {
                      rest[0]='\0';
                      rest++;
                      iemonth=atoi(begin);
                      ieday=atoi(rest);
                    }
                  else
                    {
                      iemonth=0;
                      ieday=0;
                    }
                  if ((rest=strchr(inbuff, ',')))
                    {
                      begin=inbuff;
                      rest[0]='\0';
                      rest++;
                      isyear=atoi(begin);
                      begin=rest;
                      if ((rest=strchr(begin, ',')))
                        {
                          rest[0]='\0';
                          rest++;
                          ismonth=atoi(begin);
                          isday=atoi(rest);
                        }
                      else
                        {
                          ismonth=0;
                          isday=0;
                        }
                    }
                  else
                    {
                      sdate=atof(inbuff);
                    }
                }
              else
                {
                  decyears=1;                    /* Else it's decimal years */
                  sdate=atof(inbuff);
                  edate=atof(begin);
                }
            }
          else
            {
              range = 1;
              if ((rest=strchr(inbuff, ',')))   /* If it contains a comma */
                {
                  decyears=2;                    /* It's not decimal years */
                  begin=inbuff;
                  rest[0]='\0';
                  rest++;
                  isyear=atoi(begin);
                  begin=rest;
                  if ((rest=strchr(begin, ',')))
                    {
                      rest[0]='\0';
                      rest++;
                      ismonth=atoi(begin);
                      isday=atoi(rest);
                    }
                  else
                    {
                      ismonth=0;
                      isday=0;
                    }
                  sdate = julday(ismonth,isday,isyear);
                }
              else
                {
                  decyears=1;                    /* Else it's decimal years */
                  sdate=atof(args[2]);
                }
            }
          if (sdate==0)
            {                        /* If date not valid */
              decyears=-1;
              range=-1;
            }

        case 2 :
          if (need_to_read_model)
            {
              strncpy(mdfile,args[1],MAXREAD);
              stream=fopen(mdfile, "rt");
            }
          break;
        }

      if (range == 2 && coords_from_file)
        {
          printf("Error in line %1d, date = %s: date ranges not allowed for file option\n\n",iline,args[2]);
          exit(2);
        }

      /*  Obtain the desired model file and read the data  */

      warn_H = 0;
      warn_H_val = 99999.0;
      warn_H_strong = 0;
      warn_H_strong_val = 99999.0;
      warn_P = 0;

      if (need_to_read_model)
        {
          while (stream == NULL)
            {
              printf("\n\n");
              printf("What is the name of the model data file to be opened? ==> ");
              safegets(inbuff,MAXREAD);
              strcpy(mdfile, inbuff);
              if (!(stream = fopen(mdfile, "rt")))
                printf("\nError opening file %s.", mdfile);
            }
          rewind(stream);

          fileline = 0;                            /* First line will be 1 */
          modelI = -1;                             /* First model will be 0 */
          while (fgets(inbuff,MAXREAD,stream))     /* While not end of file
                                                   * read to end of line or buffer */
            {
              fileline++;                           /* On new line */


              if (strlen(inbuff) != RECL)       /* IF incorrect record size */
                {
                  printf("Corrupt record in file %s on line %d.\n", mdfile, fileline);
                  fclose(stream);
                  exit(5);
                }

              /* old statement Dec 1999 */
              /*       if (!strncmp(inbuff,"    ",4)){         /* If 1st 4 chars are spaces */
              /* New statement Dec 1999 changed by wmd  required by year 2000 models */
              if (!strncmp(inbuff,"   ",3))         /* If 1st 3 chars are spaces */
                {
                  modelI++;                           /* New model */

                  if (modelI > MAXMOD)                /* If too many headers */
                    {
                      printf("Too many models in file %s on line %d.", mdfile, fileline);
                      fclose(stream);
                      exit(6);
                    }

                  irec_pos[modelI]=ftell(stream);
                  /* Get fields from buffer into individual vars.  */
                  sscanf(inbuff, "%s%lg%d%d%d%lg%lg%lg%lg", model[modelI], &epoch[modelI],
                         &max1[modelI], &max2[modelI], &max3[modelI], &yrmin[modelI],
                         &yrmax[modelI], &altmin[modelI], &altmax[modelI]);

                  /* Compute date range for all models */
                  if (modelI == 0)                    /*If first model */
                    {
                      minyr=yrmin[0];
                      maxyr=yrmax[0];
                    }
                  else
                    {
                      if (yrmin[modelI]<minyr)
                        {
                          minyr=yrmin[modelI];
                        }
                      if (yrmax[modelI]>maxyr){
                        maxyr=yrmax[modelI];
                      }
                    } /* if modelI != 0 */

                } /* If 1st 3 chars are spaces */

            } /* While not end of model file */

          nmodel = modelI + 1;
          fclose(stream);

          /* if date specified in command line then warn if past end of validity */

          if ((sdate>maxyr)&&(sdate<maxyr+1))
            {
              printf("\nWarning: The date %4.2f is out of range,\n", sdate);
              printf("         but still within one year of model expiration date.\n");
              printf("         An updated model file is available before 1.1.%4.0f\n",maxyr);
            }

        } /*   if need_to_read_model */

      /*  Take in field data  */

      /* Get date */

      if (coords_from_file && !arg_err && (decyears != 1 && decyears != 2))
        {printf("\nError: unrecognized date %s in coordinate file line %1d\n\n",args[2],iline); arg_err = 1;}

      while ((decyears!=1)&&(decyears!=2))
        {
          printf("\nHow would you like to enter the date?\n");
          printf("       1) In decimal years.\n");
          printf("       2) In year, month, and day.\n");
          printf("\n                            ==> ");
          safegets(inbuff, MAXREAD);
          decyears=atoi(inbuff);
        }

      if (coords_from_file && !arg_err && range != 1)
        {printf("\nError: unrecognized date %s in coordinate file line %1d\n\n",args[2],iline); arg_err = 1;}

      while ((range!=1)&&(range!=2))
        {
          printf("\nWould you like output for a single date or for a range of dates?\n");
          printf("       1) A single date.\n");
          printf("       2) A range of dates.\n");
          printf("\n                            ==> ");
          safegets(inbuff, MAXREAD);
          range=atoi(inbuff);
        }


      if (range == 1)
        {
          if (coords_from_file && !arg_err && (sdate < minyr || sdate > maxyr+1))
            {printf("\nError: unrecognized date %s in coordinate file line %1d\n\n",args[2],iline); arg_err = 1;}

          while ((sdate<minyr)||(sdate>maxyr+1))
            {
              if (decyears==1)
                {
                  printf("\nEnter the decimal date (%4.2f to %4.0f): ",minyr, maxyr);
                  safegets(inbuff, MAXREAD);
                  sdate=atof(inbuff);
                }
              else
                {
                  while ((isyear>(int)maxyr+1)||(isyear<(int)minyr))
                    {
                      printf("\nEnter the date (%4.2f to %4.2f)\n ", minyr, maxyr);
                      printf("\n   Year (%d to %d): ",(int)minyr,(int)maxyr);
                      safegets(inbuff, MAXREAD);
                      isyear=atoi(inbuff);
                    }

                  while ((ismonth>12)||(ismonth<1))
                    {
                      printf("\n   Month (1-12): ");
                      safegets(inbuff, MAXREAD);
                      ismonth=atoi(inbuff);
                    }

                  while ((isday>31)||(isday<1))
                    {
                      printf("\n   Day (1-31): ");
                      safegets(inbuff, MAXREAD);
                      isday=atoi(inbuff);
                    }

                  sdate = julday(ismonth,isday,isyear);
                }
              if ((sdate<minyr)||(sdate>=maxyr+1))
                {
                  ismonth=isday=isyear=0;
                  printf("\nError: The date %4.2f is out of range.\n", sdate);
                }

              if ((sdate>maxyr)&&(sdate<maxyr+1))
                {
                  printf("\nWarning: The date %4.2f is out of range,\n", sdate);
                  printf("         but still within one year of model expiration date.\n");
                  printf("         An updated model file is available before 1.1.%4.0f\n",maxyr);
                }

            } /* if single date */
        } /* (range == 1) */
      else
        {
          while ((sdate<minyr)||(sdate>maxyr))
            {
              if (decyears==1)
                {
                  printf("\nEnter the decimal start date (%4.2f to %4.0f): ",minyr, maxyr);
                  safegets(inbuff, MAXREAD);
                  sdate=atof(inbuff);
                }
              else
                {
                  while ((isyear>(int)maxyr)||(isyear<(int)minyr))
                    {
                      ismonth=isday=isyear=0;
                      printf("\nEnter the start date (%4.2f to %4.2f)\n ", minyr, maxyr);
                      printf("\n   Year (%d to %d): ",(int)minyr,(int)(maxyr));
                      safegets(inbuff, MAXREAD);
                      isyear=atoi(inbuff);
                    }
                  while ((ismonth>12)||(ismonth<1))
                    {
                      printf("\n   Month (1-12): ");
                      safegets(inbuff, MAXREAD);
                      ismonth=atoi(inbuff);
                    }
                  while ((isday>31)||(isday<1))
                    {
                      printf("\n   Day (1-31): ");
                      safegets(inbuff, MAXREAD);
                      isday=atoi(inbuff);
                    }

                  sdate = julday(ismonth,isday,isyear);

                  if ((sdate<minyr)||(sdate>maxyr))
                    {
                      printf("\nThe start date %4.2f is out of range.\n", sdate);
                    }
                }
            } /* WHILE ((sdate<minyr)||(sdate>maxyr)) */

          while ((edate<=sdate)||(edate>maxyr+1))
            {
              if (decyears==1)
                {
                  printf("\nEnter the decimal end date (%4.2f to %4.0f): ",sdate, maxyr);
                  safegets(inbuff, MAXREAD);
                  edate=atof(inbuff);
                }
              else
                {
                  while ((ieyear>(int)maxyr)||(ieyear<(int)sdate))
                    {
                      iemonth=ieday=ieyear=0;
                      printf("\nEnter the end date (%4.2f to %4.0f)\n ", sdate, maxyr);
                      printf("\n   Year (%d to %d): ",(int)sdate,(int)(maxyr));
                      safegets(inbuff, MAXREAD);
                      ieyear=atoi(inbuff);
                    }

                  while ((iemonth>12)||(iemonth<1))
                    {
                      printf("\n   Month (1-12): ");
                      safegets(inbuff, MAXREAD);
                      iemonth=atoi(inbuff);
                    }

                  while ((ieday>31)||(ieday<1))
                    {
                      printf("\n   Day (1-31): ");
                      safegets(inbuff, MAXREAD);
                      ieday=atoi(inbuff);
                    }

                  edate = julday(iemonth,ieday,ieyear);

                }

              if ((edate<sdate)||(edate>maxyr+1))
                {
                  printf("\nThe date %4.2f is out of range.\n", edate);
                }

              if (edate>maxyr && edate<=maxyr+1)
                {
                  printf("\nWarning: The end date %4.2f is out of range,\n", edate);
                  printf("         but still within one year of model expiration date.\n");
                  printf("         An updated model file is available before 1.1.%4.0f\n",maxyr);
                }
            } /* while ((edate<=sdate)||(edate>maxyr+1)) */

          while ((step<=0)||(step>(edate-sdate)))
            {
              printf("\nEnter the step size in years. (0 to %4.2f): ",edate-sdate);
              safegets(inbuff, MAXREAD);
              step=atof(inbuff);
            }

        } /* if (range == 2) */


      /* Pick model */
      for (modelI=0; modelI<nmodel; modelI++)
        if (sdate<yrmax[modelI]) break;
      if (modelI == nmodel) modelI--;           /* if beyond end of last model use last model */

      /* Get altitude min and max for selected model. */
      minalt=altmin[modelI];
      maxalt=altmax[modelI];

      /* Get Coordinate prefs */

      if (coords_from_file && !arg_err && (igdgc != 1 && igdgc != 2))
          {printf("\nError: unrecognized coordinate system %s in coordinate file line %1d\n\n",args[3],iline); arg_err = 1;}

      while ((igdgc!=1)&&(igdgc!=2))
        {
          printf("\n\nEnter Coordinate Preferences:");
          printf("\n    1) Geodetic (WGS84 latitude and altitude above mean sea level)");
          printf("\n    2) Geocentric (spherical, altitude relative to Earth's center)\n");
          printf("\n                            ==> ");
          safegets(inbuff, MAXREAD);
          igdgc=atoi(inbuff);
        }

      /* If needed modify ranges to reflect coords. */
      if (igdgc==2)
        {
          minalt+=6371.2;  /* Add radius to ranges. */
          maxalt+=6371.2;
        }

      /* Get unit prefs */
      if (igdgc==1)
        {
          if (coords_from_file && !arg_err && (units > 3 || units < 1))
            {printf("\nError: unrecognized altitude units %s in coordinate file line %1d\n\n",args[4],iline); arg_err = 1;}

          while ((units>3)||(units<1))
            {
              printf("\n\nEnter Unit Preferences:");
              printf("\n       1) Kilometers");
              printf("\n       2) Meters");
              printf("\n       3) Feet\n");
              printf("\n                            ==> ");
              safegets(inbuff, MAXREAD);
              units=atoi(inbuff);
            }
        }
      else units = 1; /* geocentric always in km */

      /* Do unit conversions if neccessary */
      if (units==2)
        {
          minalt*=1000.0;
          maxalt*=1000.0;
        }
      else if (units==3)
        {
          minalt*=FT2KM;
          maxalt*=FT2KM;
        }

      /* Get altitude */

      if (coords_from_file && !arg_err && (alt < minalt || alt > maxalt))
        {printf("\nError: unrecognized altitude %s in coordinate file line %1d\n\n",args[4],iline); arg_err = 1;}

      while ((alt<minalt)||(alt>maxalt))
        {
          if (igdgc==2) printf("\n\nEnter geocentric altitude in km (%.2f to %.2f): ", minalt, maxalt);
          if (igdgc==1 && units==1) printf("\n\nEnter geodetic altitude above mean sea level in km (%.2f to %.2f): ", minalt, maxalt);
          if (igdgc==1 && units==2) printf("\n\nEnter geodetic altitude above mean sea level in meters (%.2f to %.2f): ", minalt, maxalt);
          if (igdgc==1 && units==3) printf("\n\nEnter geodetic altitude above mean sea level in feet (%.2f to %.2f): ", minalt, maxalt);
          safegets(inbuff, MAXREAD);
          alt=atof(inbuff);
        }

      /* Convert altitude to km */
      if (units==2)
        {
          alt *= 0.001;
        }
      else if (units==3)
        {
          alt /= FT2KM;
        }

      /* Get lat/long prefs */

      if (coords_from_file && !arg_err && (decdeg != 1 && decdeg != 2))
        {printf("\nError: unrecognized lat %s or lon %s in coordinate file line %1d\n\n",args[5],args[6],iline); arg_err = 1;}

      while ((decdeg!=1)&&(decdeg!=2))
        {
          printf("\n\nHow would you like to enter the latitude and longitude?:");
          printf("\n       1) In decimal degrees.");
          printf("\n       2) In degrees, minutes, and seconds.\n");
          printf("\n                            ==> ");
          safegets(inbuff, MAXREAD);
          decdeg=atoi(inbuff);
        }


      /* Get lat/lon */

      if (decdeg==1)
        {
          if (coords_from_file && !arg_err && (latitude < -90 || latitude > 90))
            {printf("\nError: unrecognized latitude %s in coordinate file line %1d\n\n",args[6],iline); arg_err = 1;}

          while ((latitude<-90)||(latitude>90))
            {
              printf("\n\nEnter the decimal latitude (-90 to 90) (- for Southern hemisphere).\n");
              safegets(inbuff, MAXREAD);
              latitude=atof(inbuff);
            }

          if (coords_from_file && !arg_err && (longitude < -180 || longitude > 180))
            {printf("\nError: unrecognized longitude %s in coordinate file line %1d\n\n",args[6],iline); arg_err = 1;}

          while ((longitude<-180)||(longitude>180))
            {
              printf("\n\nEnter the decimal longitude (-180 to 180) (- for Western hemisphere).\n");
              safegets(inbuff, MAXREAD);
              longitude=atof(inbuff);
            }
        } /* if (decdeg==1) */
      else
        {
          latitude=degrees_to_decimal(ilat_deg,ilat_min,ilat_sec);
          longitude=degrees_to_decimal(ilon_deg,ilon_min,ilon_sec);

          if (coords_from_file && !arg_err && (latitude < -90 || latitude > 90))
            {printf("\nError: unrecognized latitude %s in coordinate file line %1d\n\n",args[6],iline); arg_err = 1;}

          while ((latitude<-90)||(latitude>90))
            {
              ilat_deg=ilat_min=ilat_sec=200;
              printf("\n\nEnter the decimal latitude (-90 to 90) (- for Southern hemisphere).\n");
              while ((ilat_deg<-90)||(ilat_deg>90))
                {
                  printf("\nDegrees (-90 to 90): ");
                  safegets(inbuff, MAXREAD);
                  ilat_deg=atoi(inbuff);
                }
              while ((ilat_min<-59)||(ilat_min>59))
                {
                  printf("\nMinutes (-59 to 59): ");
                  safegets(inbuff, MAXREAD);
                  ilat_min=atoi(inbuff);
                }
              while ((ilat_sec<-59)||(ilat_sec>59))
                {
                  printf("\nSeconds (-59 to 59): ");
                  safegets(inbuff, MAXREAD);
                  ilat_sec=atoi(inbuff);
                }

              latitude=degrees_to_decimal(ilat_deg,ilat_min,ilat_sec);

              if ((latitude<-90)||(latitude>90))
                {
                  printf("\nThe latitude %3.2f is out of range", latitude);
                }
            } /* while ((latitude<-90)||(latitude>90)) */

          if (coords_from_file && !arg_err && (longitude < -180 || longitude > 180))
            {printf("\nError: unrecognized longitude %s in coordinate file line %1d\n\n",args[6],iline); arg_err = 1;}

          while ((longitude<-180)||(longitude>180))
            {
              ilon_deg=ilon_min=ilon_sec=200;
              printf("\n\nEnter the decimal longitude (-180 to 180) (- for Western hemisphere).\n");
              while ((ilon_deg<-180)||(ilon_deg>180))
                {
                  printf("\nDegrees (-180 to 180): ");
                  safegets(inbuff, MAXREAD);
                  ilon_deg=atoi(inbuff);
                }
              while ((ilon_min<-59)||(ilon_min>59))
                {
                  printf("\nMinutes (0 to 59): ");
                  safegets(inbuff, MAXREAD);
                  ilon_min=atoi(inbuff);
                }
              while ((ilon_sec<-59)||(ilon_sec>59))
                {
                  printf("\nSeconds (0 to 59): ");
                  safegets(inbuff, MAXREAD);
                  ilon_sec=atoi(inbuff);
                }

              longitude=degrees_to_decimal(ilon_deg,ilon_min,ilon_sec);

              if ((longitude<-180)||(longitude>180))
                {
                  printf("\nThe longitude %3.2f is out of range", longitude);
                }
            } /* while ((longitude<-180)||(longitude>180)) */
        } /* if (decdeg != 1) */

      /** This will compute everything needed for 1 point in time. **/


      if (max2[modelI] == 0)
        {
          getshc(mdfile, 1, irec_pos[modelI], max1[modelI], 1);
          getshc(mdfile, 1, irec_pos[modelI+1], max1[modelI+1], 2);
          nmax = interpsh(sdate, yrmin[modelI], max1[modelI],
                          yrmin[modelI+1], max1[modelI+1], 3);
          nmax = interpsh(sdate+1, yrmin[modelI] , max1[modelI],
                          yrmin[modelI+1], max1[modelI+1],4);
        }
      else
        {
          getshc(mdfile, 1, irec_pos[modelI], max1[modelI], 1);
          getshc(mdfile, 0, irec_pos[modelI], max2[modelI], 2);
          nmax = extrapsh(sdate, epoch[modelI], max1[modelI], max2[modelI], 3);
          nmax = extrapsh(sdate+1, epoch[modelI], max1[modelI], max2[modelI], 4);
        }


      /* Do the first calculations */
      shval3(igdgc, latitude, longitude, alt, nmax, 3,
             IEXT, EXT_COEFF1, EXT_COEFF2, EXT_COEFF3);
      dihf(3);
      shval3(igdgc, latitude, longitude, alt, nmax, 4,
             IEXT, EXT_COEFF1, EXT_COEFF2, EXT_COEFF3);
      dihf(4);


      ddot = ((dtemp - d)*RAD2DEG);
      if (ddot > 180.0) ddot -= 360.0;
      if (ddot <= -180.0) ddot += 360.0;
      ddot *= 60.0;

      idot = ((itemp - i)*RAD2DEG)*60;
      d = d*(RAD2DEG);   i = i*(RAD2DEG);
      hdot = htemp - h;   xdot = xtemp - x;
      ydot = ytemp - y;   zdot = ztemp - z;
      fdot = ftemp - f;

      /* deal with geographic and magnetic poles */

      if (h < 100.0) /* at magnetic poles */
        {
          d = NaN;
          ddot = NaN;
          /* while rest is ok */
        }

      if (h < 1000.0)
        {
          warn_H = 0;
          warn_H_strong = 1;
          if (h<warn_H_strong_val) warn_H_strong_val = h;
        }
      else if (h < 5000.0 && !warn_H_strong)
        {
          warn_H = 1;
          if (h<warn_H_val) warn_H_val = h;
        }

      if (90.0-fabs(latitude) <= 0.001) /* at geographic poles */
        {
          x = NaN;
          y = NaN;
          d = NaN;
          xdot = NaN;
          ydot = NaN;
          ddot = NaN;
          warn_P = 1;
          warn_H = 0;
          warn_H_strong = 0;
          /* while rest is ok */
        }

      /** Above will compute everything for 1 point in time.  **/


      /*  Output the final results. */

      if (coords_from_file)
        {
          print_result_file(outfile, d, i, h, x, y, z, f,ddot,idot,hdot,xdot,ydot,zdot,fdot);
        }
      else
        {
          printf("\n\n\n  Model: %s \n", model[modelI]);
          if (decdeg==1)
            {
              printf("  Latitude: %4.2f deg\n", latitude);
              printf("  Longitude: %4.2f deg\n", longitude);
            }
          else
            {
              printf("  Latitude: %d deg, %d min, %d sec\n",
                     ilat_deg,ilat_min, ilat_sec);
              printf("  Longitude: %d deg,  %d min, %d sec\n",
                     ilon_deg, ilon_min, ilon_sec);
            }
          printf("  Altitude: ");
          if (units==1)
            printf("%.2f km\n", alt);
          else if (units==2)
            printf("%.2f meters\n", alt*1000.0);
          else
            printf("%.2f ft\n", (alt*FT2KM));

          if (range==1)
            {
              printf("  Date of Interest: ");
              if (decyears==1)
                printf(" %4.2f\n\n", sdate);
              else
                printf("%d-%d-%d (yyyy-mm-dd)\n\n",  isyear, ismonth, isday);

              print_header();
              print_result(sdate,d, i, h, x, y, z, f);
              print_long_dashed_line();
              print_header_sv();
              print_result_sv(sdate,ddot,idot,hdot,xdot,ydot,zdot,fdot);
              print_dashed_line();

            } /* if range == 1 */
          else
            {
              printf("  Range of Interest: ");
              if (decyears==1)
                printf("%4.2f to %4.2f, step %4.2f\n\n",sdate, edate, step);
              else
                printf("%d-%d-%d to %d-%d-%d (yyyy-mm-dd), step %4.2f (years)\n\n", isyear, ismonth, isday,  ieyear, iemonth, ieday, step);

              print_header();
              print_result(sdate,d, i, h, x, y, z, f);

              for(syr=sdate+step;syr<(edate+step);syr+=step)
                {
                  if ((syr>edate)&&(edate!=(syr-step)))
                    {
                      syr=edate;
                      print_long_dashed_line();
                    }

                  /* Do the calculations */

                  for (counter=0;counter<step;counter++)
                    {
                      if (max2[modelI] == 0){       /*If not last element in array */
                        if (syr>yrmin[modelI+1]){  /* And past model boundary */
                          modelI++;              /* Get next model */
                        }
                      }
                    } /* for counter */

                  if (max2[modelI] == 0)       /*If still not last element */
                    {
                      getshc(mdfile, 1, irec_pos[modelI], max1[modelI], 1);
                      getshc(mdfile, 1, irec_pos[modelI+1], max1[modelI+1], 2);
                      nmax = interpsh(syr, yrmin[modelI], max1[modelI],
                                      yrmin[modelI+1], max1[modelI+1], 3);
                      nmax = interpsh(syr+1, yrmin[modelI] , max1[modelI],
                                      yrmin[modelI+1], max1[modelI+1],4);
                    }
                  else
                    {
                      getshc(mdfile, 1, irec_pos[modelI], max1[modelI], 1);
                      getshc(mdfile, 0, irec_pos[modelI], max2[modelI], 2);
                      nmax = extrapsh(syr, epoch[modelI], max1[modelI],
                                      max2[modelI], 3);
                      nmax = extrapsh(syr+1, epoch[modelI], max1[modelI],
                                      max2[modelI], 4);
                    }
                  shval3(igdgc, latitude, longitude, alt, nmax, 3,
                         IEXT, EXT_COEFF1, EXT_COEFF2, EXT_COEFF3);
                  dihf(3);
                  shval3(igdgc, latitude, longitude, alt, nmax, 4,
                         IEXT, EXT_COEFF1, EXT_COEFF2, EXT_COEFF3);
                  dihf(4);

                  ddot = ((dtemp - d)*RAD2DEG);
                  if (ddot > 180.0) ddot -= 360.0;
                  if (ddot <= -180.0) ddot += 360.0;
                  ddot *= 60.0;

                  idot = ((itemp - i)*RAD2DEG)*60.;
                  d = d*(RAD2DEG);   i = i*(RAD2DEG);
                  hdot = htemp - h;   xdot = xtemp - x;
                  ydot = ytemp - y;   zdot = ztemp - z;
                  fdot = ftemp - f;

                  /* deal with geographic and magnetic poles */

                  if (h < 100.0) /* at magnetic poles */
                    {
                      d = NaN;
                      ddot = NaN;
                      /* while rest is ok */
                    }

                  if (90.0-fabs(latitude) <= 0.001) /* at geographic poles */
                    {
                      x = NaN;
                      y = NaN;
                      d = NaN;
                      xdot = NaN;
                      ydot = NaN;
                      ddot = NaN;
                      warn_P = 1;
                      warn_H = 0;
                      warn_H_strong = 0;
                      /* while rest is ok */
                    }

                  print_result(syr, d, i, h, x, y, z, f);
                } /* for syr */

              print_long_dashed_line();
              print_header_sv();
              print_result_sv(edate,ddot,idot,hdot,xdot,ydot,zdot,fdot);
              print_dashed_line();
            } /* if range > 1 */

          if (warn_H)
            {
              printf("\nWarning: The horizontal field strength at this location is only %6.1f nT\n",warn_H_val);
              printf("         Compass readings have large uncertainties in areas where H is\n");
              printf("         smaller than 5000 nT\n\n");
            }
          if (warn_H_strong)
            {
              printf("\nWarning: The horizontal field strength at this location is only %6.1f nT\n",warn_H_strong_val);
              printf("         Compass readings have VERY LARGE uncertainties in areas where H is\n");
              printf("         smaller than 1000 nT\n\n");
            }
          if (warn_P)
            {
              printf("\nWarning: Location is at geographic pole where X, Y, and declination are not computed\n\n");
            }
        } /* if not coords_from_file */

      if (coords_from_file)
        again = !feof(coordfile) && !arg_err;
      else
        {
          if (argc==7) again = 0; /* run command line only once */
          else
            do
              {
                printf("\n  Enter");
                printf("\n     0) to quit.");
                printf("\n     1) to select a new model input file.");
                printf("\n     2) to compute for a new point using same data file.");
                printf("\n\n   ==> ");
                safegets(inbuff, MAXREAD);
                again=atoi(inbuff);
                if (again == 1) { need_to_read_model = 1; stream = NULL;}
                if (again == 2) { need_to_read_model = 0; again = 1;}
              }
            while (again != 0 && again != 1);
        } /* if not coords_from_file */

      if (again == 1)
        {
          /* Reset defaults to catch on all while loops */
          igdgc=decyears=units=decdeg=-1;
          ismonth=isday=isyear=sdate=edate=range=step=-1;
          latitude=ilat_deg=ilat_min=ilat_sec=200;
          longitude=ilon_deg=ilon_min=ilon_sec=200;
          alt=-9999999;
          argc = 1;
        }
    } /* while (again == 1) */

 reached_EOF:
  if (coords_from_file) printf("\n Processed %1d lines\n\n",iline);

  if (coords_from_file && !feof(coordfile) && arg_err) printf("Terminated prematurely due to argument error in coordinate file\n\n");

  if (argc<7)
    {
      printf("\nThe same result could have been generated with the following command:\n");
      printf("%s %s ", args[0], mdfile);
      if (range == 1)
        {
          if (decyears==1)
            {
              printf("%4.2f ", sdate);
            }
          else
            {
              printf("%d,%d,%d ", isyear,ismonth,isday);
            }
        }
      else
        {
          if (decyears==1)
            {
              printf("%4.2f-%4.2f-%4.2f ", sdate,edate,step);
            }
          else
            {
              printf("%d,%d,%d-%d,%d,%d-%4.2f ", isyear,ismonth,isday,ieyear,iemonth,ieday,step);
            }
        }
      if (igdgc==1)
        {
          printf("D ");
        }
      else
        {
          printf("C ");
        }
      if (units==1)
        {
          printf("K%.2f ", alt);
        }
      else if (units==2)
        {
          printf("M%.2f ", alt*1000.0);
        }
      else
        {
          printf("F%.2f ", (alt*FT2KM));
        }
      if (decdeg==1)
        {
          printf("%3.2f %4.2f\n\n", latitude, longitude);
        }
      else
        {
          printf("%d,%d,%d %d,%d,%d\n\n", ilat_deg, ilat_min, ilat_sec,
                 ilon_deg, ilon_min, ilon_sec);
        }
    }
  return 0;
}

void print_dashed_line(void)
{
  printf(" -------------------------------------------------------------------------------\n");
  return;
}


void print_long_dashed_line(void)
{
  printf(" - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -\n");
  return;
}

void print_header(void)
{
  print_dashed_line();
  printf("   Date          D           I           H        X        Y        Z        F\n");
  printf("   (yr)      (deg min)   (deg min)     (nT)     (nT)     (nT)     (nT)     (nT)\n");
  return;
}

void print_result(double date, double d, double i, double h, double x, double y, double z, double f)
{
  int   ddeg,ideg;
  double dmin,imin;

      /* Change d and i to deg and min */

  ddeg=(int)d;
  dmin=(d-(double)ddeg)*60;
  if (d > 0 && dmin >= 59.5)
    {
      dmin -= 60.0;
      ddeg++;
    }
  if (d < 0 && dmin <= -59.5)
    {
      dmin += 60.0;
      ddeg--;
    }

  if (ddeg!=0) dmin=fabs(dmin);
  ideg=(int)i;
  imin=(i-(double)ideg)*60;
  if (i > 0 && imin >= 59.5)
    {
      imin -= 60.0;
      ideg++;
    }
  if (i < 0 && imin <= -59.5)
    {
      imin += 60.0;
      ideg--;
    }
  if (ideg!=0) imin=fabs(imin);


  if (my_isnan(d))
    {
      if (my_isnan(x))
        printf("  %4.2f       NaN    %4dd %3.0fm  %8.1f      NaN      NaN %8.1f %8.1f\n",date,ideg,imin,h,z,f);
      else
        printf("  %4.2f       NaN    %4dd %3.0fm  %8.1f %8.1f %8.1f %8.1f %8.1f\n",date,ideg,imin,h,x,y,z,f);
    }
  else
    printf("  %4.2f  %4dd %3.0fm  %4dd %3.0fm  %8.1f %8.1f %8.1f %8.1f %8.1f\n",date,ddeg,dmin,ideg,imin,h,x,y,z,f);
  return;
} /* print_result */

void print_header_sv(void)
{
  printf("   Date         dD          dI           dH       dX       dY       dZ       dF\n");
  printf("   (yr)      (min/yr)    (min/yr)    (nT/yr)  (nT/yr)  (nT/yr)  (nT/yr)  (nT/yr)\n");
} /* print_header_sv */

void print_result_sv(double date, double ddot, double idot, double hdot, double xdot, double ydot, double zdot, double fdot)
{
  if (my_isnan(ddot))
    {
      if (my_isnan(xdot))
        printf("  %4.2f        NaN   %7.1f     %8.1f      NaN      NaN %8.1f %8.1f\n",date,idot,hdot,zdot,fdot);
      else
        printf("  %4.2f        NaN   %7.1f     %8.1f %8.1f %8.1f %8.1f %8.1f\n",date,idot,hdot,xdot,ydot,zdot,fdot);
    }
  else
    printf("  %4.2f   %7.1f    %7.1f     %8.1f %8.1f %8.1f %8.1f %8.1f\n",date,ddot,idot,hdot,xdot,ydot,zdot,fdot);
  return;
} /* print_result_sv */

void print_result_file(FILE *outf, double d, double i, double h, double x, double y, double z, double f,
                       double ddot, double idot, double hdot, double xdot, double ydot, double zdot, double fdot)
{
  int   ddeg,ideg;
  double dmin,imin;

  /* Change d and i to deg and min */

  ddeg=(int)d;
  dmin=(d-(double)ddeg)*60;
  if (ddeg!=0) dmin=fabs(dmin);
  ideg=(int)i;
  imin=(i-(double)ideg)*60;
  if (ideg!=0) imin=fabs(imin);

  if (my_isnan(d))
    {
      if (my_isnan(x))
        fprintf(outf," NaN        %4dd %2.0fm  %8.1f      NaN      NaN %8.1f %8.1f",ideg,imin,h,z,f);
      else
        fprintf(outf," NaN        %4dd %2.0fm  %8.1f %8.1f %8.1f %8.1f %8.1f",ideg,imin,h,x,y,z,f);
    }
  else
    fprintf(outf," %4dd %2.0fm  %4dd %2.0fm  %8.1f %8.1f %8.1f %8.1f %8.1f",ddeg,dmin,ideg,imin,h,x,y,z,f);

  if (my_isnan(ddot))
    {
      if (my_isnan(xdot))
        fprintf(outf,"      NaN  %7.1f     %8.1f      NaN      NaN %8.1f %8.1f\n",idot,hdot,zdot,fdot);
      else
        fprintf(outf,"      NaN  %7.1f     %8.1f %8.1f %8.1f %8.1f %8.1f\n",idot,hdot,xdot,ydot,zdot,fdot);
    }
  else
    fprintf(outf," %7.1f   %7.1f     %8.1f %8.1f %8.1f %8.1f %8.1f\n",ddot,idot,hdot,xdot,ydot,zdot,fdot);
  return;
} /* print_result_file */


/****************************************************************************/
/*                                                                          */
/*                       Subroutine safegets                                */
/*                                                                          */
/****************************************************************************/
/*                                                                          */
/*  Gets characters from stdin untill it has reached n characters or \n,    */
/*     whichever comes first.  \n is converted to \0.                       */
/*                                                                          */
/*  Input: n - Integer number of chars                                      */
/*         *buffer - Character array ptr which can contain n+1 characters   */
/*                                                                          */
/*  Output: size - integer size of sting in buffer                          */
/*                                                                          */
/*  Note: All strings will be null terminated.                              */
/*                                                                          */
/*  By: David Owens                                                         */
/*      dio@ngdc.noaa.gov                                                   */
/****************************************************************************/

int safegets(char *buffer,int n){
  char *ptr;                    /** ptr used for finding '\n' **/

  buffer[0] = '\0';
  fgets(buffer,n,stdin);        /** Get n chars **/
  buffer[n+1]='\0';             /** Set last char to null **/
  ptr=strchr(buffer,'\n');      /** If string contains '\n' **/
  if (ptr!=NULL){                /** If string contains '\n' **/
    ptr[0]='\0';               /** Change char to '\0' **/
    if (buffer[0] == '\0') printf("\n ... no entry ...\n");
  }

  return strlen(buffer);        /** Return the length **/
}


/****************************************************************************/
/*                                                                          */
/*                       Subroutine degrees_to_decimal                      */
/*                                                                          */
/****************************************************************************/
/*                                                                          */
/*     Converts degrees,minutes, seconds to decimal degrees.                */
/*                                                                          */
/*     Input:                                                               */
/*            degrees - Integer degrees                                     */
/*            minutes - Integer minutes                                     */
/*            seconds - Integer seconds                                     */
/*                                                                          */
/*     Output:                                                              */
/*            decimal - degrees in decimal degrees                          */
/*                                                                          */
/*     C                                                                    */
/*           C. H. Shaffer                                                  */
/*           Lockheed Missiles and Space Company, Sunnyvale CA              */
/*           August 12, 1988                                                */
/*                                                                          */
/****************************************************************************/

double degrees_to_decimal(int degrees,int minutes,int seconds)
{
  double deg;
  double min;
  double sec;
  double decimal;

  deg = degrees;
  min = minutes/60.0;
  sec = seconds/3600.0;

  decimal = fabs(sec) + fabs(min) + fabs(deg);

  if (deg < 0) {
    decimal = -decimal;
  } else if (deg == 0){
    if (min < 0){
      decimal = -decimal;
    } else if (min == 0){
      if (sec<0){
        decimal = -decimal;
      }
    }
  }

  return(decimal);
}

/****************************************************************************/
/*                                                                          */
/*                           Subroutine julday                              */
/*                                                                          */
/****************************************************************************/
/*                                                                          */
/*     Computes the decimal day of year from month, day, year.              */
/*     Supplied by Daniel Bergstrom                                         */
/*                                                                          */
/* References:                                                              */
/*                                                                          */
/* 1. Nachum Dershowitz and Edward M. Reingold, Calendrical Calculations,   */
/*    Cambridge University Press, 3rd edition, ISBN 978-0-521-88540-9.      */
/*                                                                          */
/* 2. Claus Tøndering, Frequently Asked Questions about Calendars,          */
/*    Version 2.9, http://www.tondering.dk/claus/calendar.html              */
/*                                                                          */
/****************************************************************************/

double julday(month, day, year)
     int month;
     int day;
     int year;
{
  int days[12] = { 0, 31, 59, 90, 120, 151, 181, 212, 243, 273, 304, 334};

  int leap_year = (((year % 4) == 0) &&
                   (((year % 100) != 0) || ((year % 400) == 0)));

  double day_in_year = (days[month - 1] + day + (month > 2 ? leap_year : 0));

  return ((double)year + (day_in_year / (365.0 + leap_year)));
}


/****************************************************************************/
/*                                                                          */
/*                           Subroutine getshc                              */
/*                                                                          */
/****************************************************************************/
/*                                                                          */
/*     Reads spherical harmonic coefficients from the specified             */
/*     model into an array.                                                 */
/*                                                                          */
/*     Input:                                                               */
/*           stream     - Logical unit number                               */
/*           iflag      - Flag for SV equal to ) or not equal to 0          */
/*                        for designated read statements                    */
/*           strec      - Starting record number to read from model         */
/*           nmax_of_gh - Maximum degree and order of model                 */
/*                                                                          */
/*     Output:                                                              */
/*           gh1 or 2   - Schmidt quasi-normal internal spherical           */
/*                        harmonic coefficients                             */
/*                                                                          */
/*     FORTRAN                                                              */
/*           Bill Flanagan                                                  */
/*           NOAA CORPS, DESDIS, NGDC, 325 Broadway, Boulder CO.  80301     */
/*                                                                          */
/*     C                                                                    */
/*           C. H. Shaffer                                                  */
/*           Lockheed Missiles and Space Company, Sunnyvale CA              */
/*           August 15, 1988                                                */
/*                                                                          */
/****************************************************************************/


int getshc(file, iflag, strec, nmax_of_gh, gh)
char file[PATH];
int iflag;
long int  strec;
int       nmax_of_gh;
int       gh;
{
  char  inbuff[MAXINBUFF];
  char irat[9];
  int ii,m,n,mm,nn;
  int ios;
  int line_num;
  double g,hh;
  double trash;

  stream = fopen(file, "rt");
  if (stream == NULL)
    {
      printf("\nError on opening file %s", file);
    }
  else
    {
      ii = 0;
      ios = 0;
      fseek(stream,strec,SEEK_SET);
      for ( nn = 1; nn <= nmax_of_gh; ++nn)
        {
          for (mm = 0; mm <= nn; ++mm)
            {
              if (iflag == 1)
                {
                  fgets(inbuff, MAXREAD, stream);
                  sscanf(inbuff, "%d%d%lg%lg%lg%lg%s%d",
                         &n, &m, &g, &hh, &trash, &trash, irat, &line_num);
                }
              else
                {
                  fgets(inbuff, MAXREAD, stream);
                  sscanf(inbuff, "%d%d%lg%lg%lg%lg%s%d",
                         &n, &m, &trash, &trash, &g, &hh, irat, &line_num);
                }
              if ((nn != n) || (mm != m))
                {
                  ios = -2;
                  fclose(stream);
                  return(ios);
                }
              ii = ii + 1;
              switch(gh)
                {
                case 1:  gh1[ii] = g;
                  break;
                case 2:  gh2[ii] = g;
                  break;
                default: printf("\nError in subroutine getshc");
                  break;
                }
              if (m != 0)
                {
                  ii = ii+ 1;
                  switch(gh)
                    {
                    case 1:  gh1[ii] = hh;
                      break;
                    case 2:  gh2[ii] = hh;
                      break;
                    default: printf("\nError in subroutine getshc");
                      break;
                    }
                }
            }
        }
    }
  fclose(stream);
  return(ios);
}


/****************************************************************************/
/*                                                                          */
/*                           Subroutine extrapsh                            */
/*                                                                          */
/****************************************************************************/
/*                                                                          */
/*     Extrapolates linearly a spherical harmonic model with a              */
/*     rate-of-change model.                                                */
/*                                                                          */
/*     Input:                                                               */
/*           date     - date of resulting model (in decimal year)           */
/*           dte1     - date of base model                                  */
/*           nmax1    - maximum degree and order of base model              */
/*           gh1      - Schmidt quasi-normal internal spherical             */
/*                      harmonic coefficients of base model                 */
/*           nmax2    - maximum degree and order of rate-of-change model    */
/*           gh2      - Schmidt quasi-normal internal spherical             */
/*                      harmonic coefficients of rate-of-change model       */
/*                                                                          */
/*     Output:                                                              */
/*           gha or b - Schmidt quasi-normal internal spherical             */
/*                    harmonic coefficients                                 */
/*           nmax   - maximum degree and order of resulting model           */
/*                                                                          */
/*     FORTRAN                                                              */
/*           A. Zunde                                                       */
/*           USGS, MS 964, box 25046 Federal Center, Denver, CO.  80225     */
/*                                                                          */
/*     C                                                                    */
/*           C. H. Shaffer                                                  */
/*           Lockheed Missiles and Space Company, Sunnyvale CA              */
/*           August 16, 1988                                                */
/*                                                                          */
/****************************************************************************/


int extrapsh(date, dte1, nmax1, nmax2, gh)
double date;
double dte1;
int   nmax1;
int   nmax2;
int   gh;
{
  int   nmax;
  int   k, l;
  int   ii;
  double factor;

  factor = date - dte1;
  if (nmax1 == nmax2)
    {
      k =  nmax1 * (nmax1 + 2);
      nmax = nmax1;
    }
  else
    {
      if (nmax1 > nmax2)
        {
          k = nmax2 * (nmax2 + 2);
          l = nmax1 * (nmax1 + 2);
          switch(gh)
            {
            case 3:  for ( ii = k + 1; ii <= l; ++ii)
                {
                  gha[ii] = gh1[ii];
                }
              break;
            case 4:  for ( ii = k + 1; ii <= l; ++ii)
                {
                  ghb[ii] = gh1[ii];
                }
              break;
            default: printf("\nError in subroutine extrapsh");
              break;
            }
          nmax = nmax1;
        }
      else
        {
          k = nmax1 * (nmax1 + 2);
          l = nmax2 * (nmax2 + 2);
          switch(gh)
            {
            case 3:  for ( ii = k + 1; ii <= l; ++ii)
                {
                  gha[ii] = factor * gh2[ii];
                }
              break;
            case 4:  for ( ii = k + 1; ii <= l; ++ii)
                {
                  ghb[ii] = factor * gh2[ii];
                }
              break;
            default: printf("\nError in subroutine extrapsh");
              break;
            }
          nmax = nmax2;
        }
    }
  switch(gh)
    {
    case 3:  for ( ii = 1; ii <= k; ++ii)
        {
          gha[ii] = gh1[ii] + factor * gh2[ii];
        }
      break;
    case 4:  for ( ii = 1; ii <= k; ++ii)
        {
          ghb[ii] = gh1[ii] + factor * gh2[ii];
        }
      break;
    default: printf("\nError in subroutine extrapsh");
      break;
    }
  return(nmax);
}

/****************************************************************************/
/*                                                                          */
/*                           Subroutine interpsh                            */
/*                                                                          */
/****************************************************************************/
/*                                                                          */
/*     Interpolates linearly, in time, between two spherical harmonic       */
/*     models.                                                              */
/*                                                                          */
/*     Input:                                                               */
/*           date     - date of resulting model (in decimal year)           */
/*           dte1     - date of earlier model                               */
/*           nmax1    - maximum degree and order of earlier model           */
/*           gh1      - Schmidt quasi-normal internal spherical             */
/*                      harmonic coefficients of earlier model              */
/*           dte2     - date of later model                                 */
/*           nmax2    - maximum degree and order of later model             */
/*           gh2      - Schmidt quasi-normal internal spherical             */
/*                      harmonic coefficients of internal model             */
/*                                                                          */
/*     Output:                                                              */
/*           gha or b - coefficients of resulting model                     */
/*           nmax     - maximum degree and order of resulting model         */
/*                                                                          */
/*     FORTRAN                                                              */
/*           A. Zunde                                                       */
/*           USGS, MS 964, box 25046 Federal Center, Denver, CO.  80225     */
/*                                                                          */
/*     C                                                                    */
/*           C. H. Shaffer                                                  */
/*           Lockheed Missiles and Space Company, Sunnyvale CA              */
/*           August 17, 1988                                                */
/*                                                                          */
/****************************************************************************/


int interpsh(date, dte1, nmax1, dte2, nmax2, gh)
     double date;
     double dte1;
     int   nmax1;
     double dte2;
     int   nmax2;
     int   gh;
{
  int   nmax;
  int   k, l;
  int   ii;
  double factor;

  factor = (date - dte1) / (dte2 - dte1);
  if (nmax1 == nmax2)
    {
      k =  nmax1 * (nmax1 + 2);
      nmax = nmax1;
    }
  else
    {
      if (nmax1 > nmax2)
        {
          k = nmax2 * (nmax2 + 2);
          l = nmax1 * (nmax1 + 2);
          switch(gh)
            {
            case 3:  for ( ii = k + 1; ii <= l; ++ii)
                {
                  gha[ii] = gh1[ii] + factor * (-gh1[ii]);
                }
              break;
            case 4:  for ( ii = k + 1; ii <= l; ++ii)
                {
                  ghb[ii] = gh1[ii] + factor * (-gh1[ii]);
                }
              break;
            default: printf("\nError in subroutine extrapsh");
              break;
            }
          nmax = nmax1;
        }
      else
        {
          k = nmax1 * (nmax1 + 2);
          l = nmax2 * (nmax2 + 2);
          switch(gh)
            {
            case 3:  for ( ii = k + 1; ii <= l; ++ii)
                {
                  gha[ii] = factor * gh2[ii];
                }
              break;
            case 4:  for ( ii = k + 1; ii <= l; ++ii)
                {
                  ghb[ii] = factor * gh2[ii];
                }
              break;
            default: printf("\nError in subroutine extrapsh");
              break;
            }
          nmax = nmax2;
        }
    }
  switch(gh)
    {
    case 3:  for ( ii = 1; ii <= k; ++ii)
        {
          gha[ii] = gh1[ii] + factor * (gh2[ii] - gh1[ii]);
        }
      break;
    case 4:  for ( ii = 1; ii <= k; ++ii)
        {
          ghb[ii] = gh1[ii] + factor * (gh2[ii] - gh1[ii]);
        }
      break;
    default: printf("\nError in subroutine extrapsh");
      break;
    }
  return(nmax);
}





/****************************************************************************/
/*                                                                          */
/*                           Subroutine shval3                              */
/*                                                                          */
/****************************************************************************/
/*                                                                          */
/*     Calculates field components from spherical harmonic (sh)             */
/*     models.                                                              */
/*                                                                          */
/*     Input:                                                               */
/*           igdgc     - indicates coordinate system used; set equal        */
/*                       to 1 if geodetic, 2 if geocentric                  */
/*           latitude  - north latitude, in degrees                         */
/*           longitude - east longitude, in degrees                         */
/*           elev      - WGS84 altitude above ellipsoid (igdgc=1), or       */
/*                       radial distance from earth's center (igdgc=2)      */
/*           a2,b2     - squares of semi-major and semi-minor axes of       */
/*                       the reference spheroid used for transforming       */
/*                       between geodetic and geocentric coordinates        */
/*                       or components                                      */
/*           nmax      - maximum degree and order of coefficients           */
/*           iext      - external coefficients flag (=0 if none)            */
/*           ext1,2,3  - the three 1st-degree external coefficients         */
/*                       (not used if iext = 0)                             */
/*                                                                          */
/*     Output:                                                              */
/*           x         - northward component                                */
/*           y         - eastward component                                 */
/*           z         - vertically-downward component                      */
/*                                                                          */
/*     based on subroutine 'igrf' by D. R. Barraclough and S. R. C. Malin,  */
/*     report no. 71/1, institute of geological sciences, U.K.              */
/*                                                                          */
/*     FORTRAN                                                              */
/*           Norman W. Peddie                                               */
/*           USGS, MS 964, box 25046 Federal Center, Denver, CO.  80225     */
/*                                                                          */
/*     C                                                                    */
/*           C. H. Shaffer                                                  */
/*           Lockheed Missiles and Space Company, Sunnyvale CA              */
/*           August 17, 1988                                                */
/*                                                                          */
/****************************************************************************/


int shval3(igdgc, flat, flon, elev, nmax, gh, iext, ext1, ext2, ext3)
     int   igdgc;
     double flat;
     double flon;
     double elev;
     int   nmax;
     int   gh;
     int   iext;
     double ext1;
     double ext2;
     double ext3;
{
  double earths_radius = 6371.2;
  double dtr = 0.01745329;
  double slat;
  double clat;
  double ratio;
  double aa, bb, cc, dd;
  double sd;
  double cd;
  double r;
  double a2;
  double b2;
  double rr;
  double fm,fn;
  double sl[14];
  double cl[14];
  double p[119];
  double q[119];
  int ii,j,k,l,m,n;
  int npq;
  int ios;
  double argument;
  double power;
  a2 = 40680631.59;            /* WGS84 */
  b2 = 40408299.98;            /* WGS84 */
  ios = 0;
  r = elev;
  argument = flat * dtr;
  slat = sin( argument );
  if ((90.0 - flat) < 0.001)
    {
      aa = 89.999;            /*  300 ft. from North pole  */
    }
  else
    {
      if ((90.0 + flat) < 0.001)
        {
          aa = -89.999;        /*  300 ft. from South pole  */
        }
      else
        {
          aa = flat;
        }
    }
  argument = aa * dtr;
  clat = cos( argument );
  argument = flon * dtr;
  sl[1] = sin( argument );
  cl[1] = cos( argument );
  switch(gh)
    {
    case 3:  x = 0;
      y = 0;
      z = 0;
      break;
    case 4:  xtemp = 0;
      ytemp = 0;
      ztemp = 0;
      break;
    default: printf("\nError in subroutine shval3");
      break;
    }
  sd = 0.0;
  cd = 1.0;
  l = 1;
  n = 0;
  m = 1;
  npq = (nmax * (nmax + 3)) / 2;
  if (igdgc == 1)
    {
      aa = a2 * clat * clat;
      bb = b2 * slat * slat;
      cc = aa + bb;
      argument = cc;
      dd = sqrt( argument );
      argument = elev * (elev + 2.0 * dd) + (a2 * aa + b2 * bb) / cc;
      r = sqrt( argument );
      cd = (elev + dd) / r;
      sd = (a2 - b2) / dd * slat * clat / r;
      aa = slat;
      slat = slat * cd - clat * sd;
      clat = clat * cd + aa * sd;
    }
  ratio = earths_radius / r;
  argument = 3.0;
  aa = sqrt( argument );
  p[1] = 2.0 * slat;
  p[2] = 2.0 * clat;
  p[3] = 4.5 * slat * slat - 1.5;
  p[4] = 3.0 * aa * clat * slat;
  q[1] = -clat;
  q[2] = slat;
  q[3] = -3.0 * clat * slat;
  q[4] = aa * (slat * slat - clat * clat);
  for ( k = 1; k <= npq; ++k)
    {
      if (n < m)
        {
          m = 0;
          n = n + 1;
          argument = ratio;
          power =  n + 2;
          rr = pow(argument,power);
          fn = n;
        }
      fm = m;
      if (k >= 5)
        {
          if (m == n)
            {
              argument = (1.0 - 0.5/fm);
              aa = sqrt( argument );
              j = k - n - 1;
              p[k] = (1.0 + 1.0/fm) * aa * clat * p[j];
              q[k] = aa * (clat * q[j] + slat/fm * p[j]);
              sl[m] = sl[m-1] * cl[1] + cl[m-1] * sl[1];
              cl[m] = cl[m-1] * cl[1] - sl[m-1] * sl[1];
            }
          else
            {
              argument = fn*fn - fm*fm;
              aa = sqrt( argument );
              argument = ((fn - 1.0)*(fn-1.0)) - (fm * fm);
              bb = sqrt( argument )/aa;
              cc = (2.0 * fn - 1.0)/aa;
              ii = k - n;
              j = k - 2 * n + 1;
              p[k] = (fn + 1.0) * (cc * slat/fn * p[ii] - bb/(fn - 1.0) * p[j]);
              q[k] = cc * (slat * q[ii] - clat/fn * p[ii]) - bb * q[j];
            }
        }
      switch(gh)
        {
        case 3:  aa = rr * gha[l];
          break;
        case 4:  aa = rr * ghb[l];
          break;
        default: printf("\nError in subroutine shval3");
          break;
        }
      if (m == 0)
        {
          switch(gh)
            {
            case 3:  x = x + aa * q[k];
              z = z - aa * p[k];
              break;
            case 4:  xtemp = xtemp + aa * q[k];
              ztemp = ztemp - aa * p[k];
              break;
            default: printf("\nError in subroutine shval3");
              break;
            }
          l = l + 1;
        }
      else
        {
          switch(gh)
            {
            case 3:  bb = rr * gha[l+1];
              cc = aa * cl[m] + bb * sl[m];
              x = x + cc * q[k];
              z = z - cc * p[k];
              if (clat > 0)
                {
                  y = y + (aa * sl[m] - bb * cl[m]) *
                    fm * p[k]/((fn + 1.0) * clat);
                }
              else
                {
                  y = y + (aa * sl[m] - bb * cl[m]) * q[k] * slat;
                }
              l = l + 2;
              break;
            case 4:  bb = rr * ghb[l+1];
              cc = aa * cl[m] + bb * sl[m];
              xtemp = xtemp + cc * q[k];
              ztemp = ztemp - cc * p[k];
              if (clat > 0)
                {
                  ytemp = ytemp + (aa * sl[m] - bb * cl[m]) *
                    fm * p[k]/((fn + 1.0) * clat);
                }
              else
                {
                  ytemp = ytemp + (aa * sl[m] - bb * cl[m]) *
                    q[k] * slat;
                }
              l = l + 2;
              break;
            default: printf("\nError in subroutine shval3");
              break;
            }
        }
      m = m + 1;
    }
  if (iext != 0)
    {
      aa = ext2 * cl[1] + ext3 * sl[1];
      switch(gh)
        {
        case 3:   x = x - ext1 * clat + aa * slat;
          y = y + ext2 * sl[1] - ext3 * cl[1];
          z = z + ext1 * slat + aa * clat;
          break;
        case 4:   xtemp = xtemp - ext1 * clat + aa * slat;
          ytemp = ytemp + ext2 * sl[1] - ext3 * cl[1];
          ztemp = ztemp + ext1 * slat + aa * clat;
          break;
        default:  printf("\nError in subroutine shval3");
          break;
        }
    }
  switch(gh)
    {
    case 3:   aa = x;
		x = x * cd + z * sd;
		z = z * cd - aa * sd;
		break;
    case 4:   aa = xtemp;
		xtemp = xtemp * cd + ztemp * sd;
		ztemp = ztemp * cd - aa * sd;
		break;
    default:  printf("\nError in subroutine shval3");
		break;
    }
  return(ios);
}


/****************************************************************************/
/*                                                                          */
/*                           Subroutine dihf                                */
/*                                                                          */
/****************************************************************************/
/*                                                                          */
/*     Computes the geomagnetic d, i, h, and f from x, y, and z.            */
/*                                                                          */
/*     Input:                                                               */
/*           x  - northward component                                       */
/*           y  - eastward component                                        */
/*           z  - vertically-downward component                             */
/*                                                                          */
/*     Output:                                                              */
/*           d  - declination                                               */
/*           i  - inclination                                               */
/*           h  - horizontal intensity                                      */
/*           f  - total intensity                                           */
/*                                                                          */
/*     FORTRAN                                                              */
/*           A. Zunde                                                       */
/*           USGS, MS 964, box 25046 Federal Center, Denver, CO.  80225     */
/*                                                                          */
/*     C                                                                    */
/*           C. H. Shaffer                                                  */
/*           Lockheed Missiles and Space Company, Sunnyvale CA              */
/*           August 22, 1988                                                */
/*                                                                          */
/****************************************************************************/

int dihf (gh)
     int gh;
{
  int ios;
  int j;
  double sn;
  double h2;
  double hpx;
  double argument, argument2;

  ios = gh;
  sn = 0.0001;

  switch(gh)
    {
    case 3:   for (j = 1; j <= 1; ++j)
        {
          h2 = x*x + y*y;
          argument = h2;
          h = sqrt(argument);       /* calculate horizontal intensity */
          argument = h2 + z*z;
          f = sqrt(argument);      /* calculate total intensity */
          if (f < sn)
            {
              d = NaN;        /* If d and i cannot be determined, */
              i = NaN;        /*       set equal to NaN         */
            }
          else
            {
              argument = z;
              argument2 = h;
              i = atan2(argument,argument2);
              if (h < sn)
                {
                  d = NaN;
                }
              else
                {
                  hpx = h + x;
                  if (hpx < sn)
                    {
                      d = PI;
                    }
                  else
                    {
                      argument = y;
                      argument2 = hpx;
                      d = 2.0 * atan2(argument,argument2);
                    }
                }
            }
        }
		break;
    case 4:   for (j = 1; j <= 1; ++j)
        {
          h2 = xtemp*xtemp + ytemp*ytemp;
          argument = h2;
          htemp = sqrt(argument);
          argument = h2 + ztemp*ztemp;
          ftemp = sqrt(argument);
          if (ftemp < sn)
            {
              dtemp = NaN;    /* If d and i cannot be determined, */
              itemp = NaN;    /*       set equal to 999.0         */
            }
          else
            {
              argument = ztemp;
              argument2 = htemp;
              itemp = atan2(argument,argument2);
              if (htemp < sn)
                {
                  dtemp = NaN;
                }
              else
                {
                  hpx = htemp + xtemp;
                  if (hpx < sn)
                    {
                      dtemp = PI;
                    }
                  else
                    {
                      argument = ytemp;
                      argument2 = hpx;
                      dtemp = 2.0 * atan2(argument,argument2);
                    }
                }
            }
        }
		break;
    default:  printf("\nError in subroutine dihf");
		break;
    }
  return(ios);
}
