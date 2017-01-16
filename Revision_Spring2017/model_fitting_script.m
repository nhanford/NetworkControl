%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
% Author: David Fridovich-Keil
% File: model_fitting_script.m
%
% Imports data and runs AR model fitting.
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%

data_dir = '../Code_July2016_LBLToMany/TCP_FQON_data/Trial1/';
destination = 'Amsterdam';
rtt_col = 3;
k = 20;
lambda = 1.0;

data = get_data(data_dir, destination, rtt_col);
coeffs = fit_sparse_ar_model(data, k, lambda)