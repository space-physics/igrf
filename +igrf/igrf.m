function B = igrf(time, glat, glon, alt_km)
%% IGRF model from Matlab.
arguments
  time (1,1) datetime
  glat (1,1) {mustBeNumeric,mustBeFinite}
  glon (1,1) {mustBeNumeric,mustBeFinite}
  alt_km (1,1) {mustBeNumeric,mustBeFinite}
end
% https://www.scivision.dev/matlab-python-user-module-import/

dat = py.igrf.igrf(datestr(time), glat, glon, alt_km);

B.north = xarray2mat(dat{'north'});
B.east = xarray2mat(dat{'east'});
B.down = xarray2mat(dat{'down'});
B.total = xarray2mat(dat{'total'});

B.incl = xarray2mat(dat{'incl'});
B.decl = xarray2mat(dat{'decl'});

end


function M = xarray2mat(V)
% M = double(py.numpy.asfortranarray(V));
V = V.values;
S = V.shape;
V = cell2mat(cell(V.ravel('F').tolist()));
if length(S) == 1
  M = V(1);
else
  M = reshape(V,[int64(S{1}), int64(S{2})]);
end
end
