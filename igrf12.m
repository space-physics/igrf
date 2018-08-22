function igrf12()
% quick demo calling IGRF12 model from Matlab.
% https://www.scivision.co/matlab-python-user-module-import/

% geographic WGS84 lat,lon,alt
glat = 65.1;
glon = -147.5;
alt_km = 0;
t = '2015-12-13T10';


B = py.igrf12.igrf(t, glat, glon, alt_km);

Bnorth = xarray2mat(B{'north'})
Beast = xarray2mat(B{'east'})
Bdown = xarray2mat(B{'down'})
Btotal = xarray2mat(B{'total'})

end


function V = xarray2mat(V)
  % convert xarray 2-D array to Matlab matrix


V = V.values;
S = V.shape;
V = cell2mat(cell(V.ravel('F').tolist()));

if ~isscalar(V) && ~isvector(V)
  V = reshape(V,[int64(S{1}), int64(S{2})]);
end

end

function I = xarrayind2vector(V,key)

C = cell(V{1}.indexes{key}.values.tolist);  % might be numeric or cell array of strings

if iscellstr(C) || any(class(C{1})=='py.str')
    I=cellfun(@char,C, 'uniformoutput',false);
else
    I = cell2mat();
end % if

end % function
