function igrf()
%% IGRF model from Matlab.
% https://www.scivision.dev/matlab-python-user-module-import/
assert(~verLessThan('matlab', '9.5'), 'Matlab >= R2018b required')

% geographic WGS84 lat,lon,alt
glat = 65.1;
glon = -147.5;
alt_km = 0;
t = '2015-12-13T10';


B = py.igrf.igrf(t, glat, glon, alt_km);

Bnorth = xarray2mat(B{'north'})
Beast = xarray2mat(B{'east'})
Bdown = xarray2mat(B{'down'})
Btotal = xarray2mat(B{'total'})

end


function M = xarray2mat(V)
M = double(py.numpy.asfortranarray(V));
end
