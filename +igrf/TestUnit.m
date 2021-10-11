classdef TestUnit < matlab.unittest.TestCase

methods(TestMethodSetup)

function tset(tc)
cwd = fileparts(mfilename('fullpath'));
run(fullfile(cwd, '../setup.m'))
end
end

methods (Test)

function test_basic(tc)

%% basic
mag = igrf.igrf(datetime(2010,7,2), 65, 85, 0);

tc.verifyEqual(mag.north, 9295.415460, "RelTol", 0.001, 'north error')
tc.verifyEqual(mag.east, 2559.7889298, "RelTol", 0.001, 'east error')
tc.verifyEqual(mag.down, 59670.379598, "RelTol", 0.001, 'down error')
tc.verifyEqual(mag.total, 60444.284008, "RelTol", 0.001, 'total error')

tc.verifyEqual(mag.incl, 80.821575, "RelTol", 0.001, 'inclination error')
tc.verifyEqual(mag.decl, 15.396590, "RelTol", 0.001, 'declination error')

end
end
end
