%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
% Author: David Fridovich-Keil
% File: apply_ar_model.m
%
% Applies an AR model with the specified coefficients to the given time
% series 'data.'
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%

function estimated = apply_ar_model(data, coeffs)

estimated = zeros(size(data));

for ii = 1:length(data)
    for jj = 1:min(length(coeffs), ii - 1)
        estimated(ii) = estimated(ii) + coeffs(jj) * data(ii - jj);
    end
end

end
