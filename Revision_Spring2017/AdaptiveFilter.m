%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
% Author: David Fridovich-Keil
% File: ConstantVelocityKF.m
%
% Implements a linear adaptive filter with LMS single-point expectation
% approximation.
% 
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%

classdef AdaptiveFilter < handle
    properties
        a_; % Autoregressive coefficients (on previous latencies).
        b_; % Moving average coefficients (on current/previous controls).
        x_; % Relevant history of previous latencies. Java ArrayList.
        u_; % Relevant history of previous controls. Java ArrayList.
        alpha_; % Learning rate for 'a'.
        beta_; % Learning rate for 'b'.
    end
    
    methods
        % Constructor. Initialize filters with i.i.d. Gaussian.
        % Must provide the following inputs:
        % 'p' = number of autoregressive coefficients
        % 'q' = number of moving average coefficients
        % 'alpha' = learning rate for 'a'
        % 'beta' = learning rate for 'b'
        function lms = AdaptiveFilter(p, q, alpha, beta)
            import java.util.ArrayList
            
            lms.a_ = 0.1 * randn(p, 1);
            lms.b_ = 0.1 * randn(q, 1);
            lms.x_ = ArrayList();
            lms.u_ = ArrayList();
            lms.alpha_ = alpha;
            lms.beta_ = beta;
        end
        
        % Predict using the current coefficients. Must supply a control
        % input 'u'.
        function x_hat = Predict(lms, u)
            x_hat = 0;
            
            % Add 'u' to the control history.
            lms.u_.add(u);

            % Get sizes of histories.
            x_size = lms.x_.size();
            u_size = lms.u_.size();
            
            % Convolve 'a' with latency history.
            for ii = 1:min(x_size, numel(lms.a_))
                x_index = x_size - ii;
                x_hat = x_hat + lms.x_.get(x_index) * lms.a_(ii);
            end
            
            % Convolve 'b' with control history.
            for ii = 1:min(u_size, numel(lms.b_))
                u_index = u_size - ii;
                x_hat = x_hat + lms.u_.get(u_index) * lms.b_(ii);
            end
        end
        
        % Update the coefficients given a prior prediction and the
        % corresponding measurement.
        function Update(lms, x, x_hat)
            error = x_hat - x;
            
            % Get sizes of histories.
            x_size = lms.x_.size();
            u_size = lms.u_.size();
            
            % Gradient descent on 'a'.
            for ii = 1:numel(lms.a_)
                x_index = x_size - ii;
                if x_index < 0
                    break
                end
                
                derivative = error * lms.x_.get(x_index);
                lms.a_(ii) = lms.a_(ii) - lms.alpha_ * derivative;
            end
            
            % Gradient descent on 'a'.
            for ii = 1:numel(lms.b_)
                u_index = u_size - ii;
                if u_index < 0
                    break
                end
                derivative = error * lms.u_.get(u_index);
                lms.b_(ii) = lms.b_(ii) - lms.beta_ * derivative;
            end
            
            % Add measurement to the list.
            lms.x_.add(x);
        end        
    end
end