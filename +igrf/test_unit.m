cwd = fileparts(mfilename('fullpath'));
run(fullfile(cwd, '../setup.m'))

%% basic
mag = igrf.igrf(datetime(2010,7,2), 65, 85, 0);

assert(abs(mag.north - 9295.415460) < mag.north*0.001, 'north error')
assert(abs(mag.east - 2559.7889298) < mag.east*0.001, 'east error')
assert(abs(mag.down - 59670.379598) < mag.down*0.001, 'down error')
assert(abs(mag.total - 60444.284008) < mag.total*0.001, 'total error')

assert(abs(mag.incl - 80.821575) < mag.incl*0.001, 'inclination error')
assert(abs(mag.decl - 15.396590) < mag.decl*0.001, 'declination error')
