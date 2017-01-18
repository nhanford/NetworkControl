%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
% Author: David Fridovich-Keil
% File: fit_sparse_ar_model.m
%
% Fits an AR model of degree k, using L1 penalization to enforce sparsity
% prior on coefficients. 'data' is a mxn matrix where n is the number
% of training runs and m is the number of data samples per run.
%
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%

function coeffs = fit_sparse_ar_model(data, k, lambda)

warning('off', 'MATLAB:nargchk:deprecated');

cvx_begin quiet
variable coeffs(k);

% Compute data fidelity term.
expression estimated(numel(data));
for ii = 1:size(data, 2)
        
    for jj = 1:size(data, 1)
        est_idx = (ii - 1) * size(data, 1) + jj;
        estimated(est_idx) = 0;
        for kk = 1:min(k, jj - 1)
            estimated(est_idx) = estimated(est_idx) + ...
                coeffs(kk) * data(jj - kk, ii);
        end
    end
end

fidelity = norm((reshape(data, [numel(data), 1]) - estimated), 2);

% Compute sparsity penalty.
penalty = size(data, 2) * lambda * norm(coeffs, 1);

% Optimize.
minimize(fidelity + penalty);

cvx_end

end
