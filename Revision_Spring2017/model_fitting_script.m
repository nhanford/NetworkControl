%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
% Author: David Fridovich-Keil
% File: model_fitting_script.m
%
% Imports data and runs AR model fitting.
% 
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%

data_dir = '../Code_July2016_LBLToMany/TCP_FQON_data/';
trials = 1:1;
validation_id = 1;
destination = 'Amsterdam';
rtt_col = 3;
k = 20;
lambda = 1.0;

data = [];
for ii = trials
    trial_dir = strcat(data_dir, 'Trial', int2str(ii), '/');
    data = [data, get_data(trial_dir, destination, rtt_col)];
end

coeffs = fit_sparse_ar_model(data, k, lambda)

% Evaluate fit.
validation_dir = strcat(data_dir, 'Trial', int2str(validation_id), '/');
data = get_data(validation_dir, destination, rtt_col);
estimated = apply_ar_model(data, coeffs);

figure;
hold on;
plot(data, 'r-*');
plot(estimated, 'b-o');
grid on;