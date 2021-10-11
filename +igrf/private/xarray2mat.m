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
