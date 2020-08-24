cwd = fileparts(mfilename('fullpath'));
addpath(cwd)

env = pyenv;
pylib = fileparts(env.Executable);
if ispc
  cand = fullfile(pylib, 'Library/bin');
else
  cand = fullfile(pylib, 'lib/bin');
end
if isfolder(cand)
  old_path = getenv('PATH');
  if ~contains(old_path, cand)
    setenv('PATH', cand + pathsep + old_path)
  end
end
