%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
% Author: David Fridovich-Keil
% File: kf_model_fitting_script.m
%
% Imports data and runs AR model fitting.
% 
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%

clear all; close all;

data_dir = '../Code_July2016_LBLToMany/TCP_FQON_data/';
trials = 1:4;
destination = 'Boulder';
rtt_col = 3;
time_col = 1;

for ii = trials
    % Extract the data.
    trial_dir = strcat(data_dir, 'Trial', int2str(ii), '/');
    data = get_data(trial_dir, destination, rtt_col);
    time = get_data(trial_dir, destination, time_col);

    % Compute the "predicted" time series.
    kf = ConstantVelocityKF();
    predicted = zeros(size(data));
    predicted_var = zeros(size(data));
    
    for jj = 1:length(predicted)
        [x, P] = kf.Predict(0);
        predicted(jj) = x(1);
        predicted_var(jj) = trace(P);
        
        [x, P] = kf.Update(data(jj));
        %predicted(jj) = x(1);
    end
        
    % Create a figure.
    figure;
    hold on;
    plot(time, data, '--or');
    errorbar(time, predicted, sqrt(predicted_var), ':*b');
    title(strcat(destination, ', Trial ', int2str(ii)));
    xlabel('Time (s)');
    ylabel('Round trip time (us)');
    legend('ground truth', 'prediction');
    grid on;
    hold off;
end