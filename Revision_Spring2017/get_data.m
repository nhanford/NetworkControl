%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
% Author: David Fridovich-Keil
% File: get_data.m
%
% Function to find and read in data files.
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%

function data = get_data(trial_dir, dest, column)

matches = dir(strcat(trial_dir, '*', dest, '*.csv'));
assert(numel(matches) == 1, 'get_data: Error. Multiple valid matches.');

table = csvread(strcat(trial_dir, matches(1).name), 1);
data = table(:, column);

end
