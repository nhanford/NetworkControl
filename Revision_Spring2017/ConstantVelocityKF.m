%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
% Author: David Fridovich-Keil
% File: ConstantVelocityKF.m
%
% Implements a constant-velocity Kalman filter. Follows the variable naming
% conventions of https://en.wikipedia.org/wiki/Kalman_filter
% 
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%

classdef ConstantVelocityKF < handle
    properties
        x_; % Current state estimate.
        P_; % Current covariance estimate.
        
        F_; % Constant velocity dynamics.
        H_; % Observation matrix.
        B_; % Control matrix.
        Q_; % Process noise covariance.
        R_; % Measurement noise covariance.
    end
    
    methods
        % Constructor. Initialize to zero state and infinite covariance.
        function kf = ConstantVelocityKF()
            kf.x_ = [0; 0];
            kf.P_ = [1e16, 1e16; 1e16, 1e16];
            
            kf.F_ = [2, -1; 1, 0];
            kf.H_ = [1, 0];
            kf.B_ = [0; 0];
            kf.Q_ = 10.0 * eye(2);
            kf.R_ = 1.0;
        end
        
        % Predict.
        function [x, P] = Predict(kf, u)
            if nargin == 1
                u = 0;
            end
            
            kf.x_ = kf.F_ * kf.x_ + kf.B_ * u;
            kf.P_ = kf.F_ * kf.P_ * kf.F_' + kf.Q_;
            
            x = kf.x_;
            P = kf.P_;
        end
        
        % Update.
        function [x, P] = Update(kf, z)
            y = z - kf.H_ * kf.x_;
            S = kf.H_ * kf.P_ * kf.H_' + kf.R_;
            K = kf.P_ * kf.H_' * inv(S);
            
            kf.x_ = kf.x_ + K * y;
            kf.P_ = (eye(2) - K * kf.H_) * kf.P_;
            
            x = kf.x_;
            P = kf.P_;
        end
    end
end