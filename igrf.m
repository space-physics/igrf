function B = igrf(time, glat, glon, alt_km)
%% IGRF model from Matlab.
arguments
  time (1,1) datetime
  glat (1,1) {mustBeNumeric,mustBeFinite}
  glon (1,1) {mustBeNumeric,mustBeFinite}
  alt_km (1,1) {mustBeNumeric,mustBeFinite}
end
% https://www.scivision.dev/matlab-python-user-module-import/

dat = py.igrf.igrf(time, glat, glon, alt_km);

B.north = xarray2mat(dat{'north'});
B.east = xarray2mat(dat{'east'});
B.down = xarray2mat(dat{'down'});
B.total = xarray2mat(dat{'total'});

end


function M = xarray2mat(V)
M = double(py.numpy.asfortranarray(V));
end
