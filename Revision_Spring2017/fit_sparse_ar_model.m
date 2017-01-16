%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
% Author: David Fridovich-Keil
% File: fit_sparse_ar_model.m
%
% Fits an AR model of degree k, using L1 penalization to enforce sparsity
% prior on coefficients.
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%

function coeffs = fit_sparse_ar_model(data, k, lambda)

warning('off', 'MATLAB:nargchk:deprecated');

cvx_begin
variable coeffs(k)
%variable const

% Compute data fidelity term.
estimated = zeros(size(data));
for ii = 1:length(data)
    for jj = 1:min(k, ii - 1)
        estimated = estimated + ...
            coeffs(jj) * data(ii - jj);% + const / length(data);
    end
end

fidelity = norm((data - estimated), 2);

% Compute sparsity penalty.
penalty = lambda * norm(coeffs, 1);

% Optimize.
minimize(fidelity + penalty);

cvx_end

end