%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
% Author: David Fridovich-Keil
% File: fit_sparse_ar_model.m
%
% Fits an AR model of degree k, using L1 penalization to enforce sparsity
% prior on coefficients. 'data' is a mxn matrix where n is the number
% of training runs and m is the number of data samples per run.
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%

function coeffs = fit_sparse_ar_model(data, k, lambda)

warning('off', 'MATLAB:nargchk:deprecated');

cvx_begin
variable coeffs(k)

% Compute data fidelity term.
fidelity = 0;
for ii = 1:size(data, 2)
    estimated = apply_ar_model(data(:, ii), coeffs);
    fidelity = fidelity + square_pos(norm((data(:, ii) - estimated), 2));
end

% Compute sparsity penalty.
penalty = size(data, 2) * size(data, 2) * lambda * lambda * ...
    square_pos(norm(coeffs, 1));

% Optimize.
minimize(fidelity + penalty);

cvx_end

end