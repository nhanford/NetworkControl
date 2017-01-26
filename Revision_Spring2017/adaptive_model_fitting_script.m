%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
% Author: David Fridovich-Keil
% File: adaptive_model_fitting_script.m
%
% Imports data and runs adaptive filter model fitting.
% 
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%

clear all; close all;

data_dir = '../Code_July2016_LBLToMany/TCP_FQON_data/';
trials = 1:4;
destination = 'Boulder';
rtt_col = 3;
time_col = 1;
p = 3;
q = 0;
alpha = 1e-11;
beta = 1e-11;

for ii = trials
    % Extract the data.
    trial_dir = strcat(data_dir, 'Trial', int2str(ii), '/');
    data = get_data(trial_dir, destination, rtt_col);
    time = get_data(trial_dir, destination, time_col);

    % Compute the "predicted" time series.
    lms = AdaptiveFilter(p, q, alpha, beta);
    predicted = zeros(size(data));
    
    for jj = 1:length(predicted)
        x_hat = lms.Predict(0);
        predicted(jj) = x_hat;
        
        lms.Update(data(jj), x_hat);
    end
        
    % Create a figure.
    figure;
    hold on;
    plot(time(2:end), data(2:end), '--or');
    plot(time(2:end), predicted(2:end), ':*b');
    title(strcat(destination, ', Trial ', int2str(ii)));
    xlabel('Time (s)');
    ylabel('Round trip time (us)');
    legend('ground truth', 'prediction');
    grid on;
    hold off;
end