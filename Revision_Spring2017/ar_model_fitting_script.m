%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
% Author: David Fridovich-Keil
% File: model_fitting_script.m
%
% Imports data and runs AR model fitting.
% 
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%

clear all; close all;

data_dir = '../Code_July2016_LBLToMany/TCP_FQON_data/';
trials = 1:3;
validation_id = 4;
destination = 'Boulder';
rtt_col = 3;
k = 8;
lambda = 0.1;

data = [];
for ii = trials
    trial_dir = strcat(data_dir, 'Trial', int2str(ii), '/');
    data = [data, get_data(trial_dir, destination, rtt_col)];
end

coeffs = fit_sparse_ar_model(data, k, lambda)

% Evaluate fit.
validation_dir = strcat(data_dir, 'Trial', int2str(validation_id), '/');
validation_data = get_data(validation_dir, destination, rtt_col);
estimated = apply_ar_model(validation_data, coeffs);

figure;
hold on;
plot(validation_data, 'r-*');
plot(estimated, 'b-o');
grid on;