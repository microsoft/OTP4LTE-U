%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
% MIT License
%
% Copyright (c) 2016 Microsoft
%
% Permission is hereby granted, free of charge, to any person obtaining a copy
% of this software and associated documentation files (the "Software"), to deal
% in the Software without restriction, including without limitation the rights
% to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
% copies of the Software, and to permit persons to whom the Software is
% furnished to do so, subject to the following conditions:
%
% The above copyright notice and this permission notice shall be included in
% all copies or substantial portions of the Software.
%
% THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
% IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
% FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
% AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
% LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
% OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
% THE SOFTWARE.
%
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%

function est = estimate(ue, chs, grid)
    % Demodulation reference signal interpolation.
    % TODO: multiple antennae, account for DC, virtual pilots, average several subframes
    dims = size(grid);
    nSC = dims(1); nSym = dims(2);
    drms = drs(ue, chs);
    drsIndices = drsi(ue, chs);
    pEst = zeros(dims); % Really so we can do nice plots
    pEst(drsIndices) = grid(drsIndices) ./ drms;

    est = zeros(size(grid));
    ind = reshape((repmat(chs.PRBSet * 12, 1, 12) + repmat(1:12, length(chs.PRBSet), 1))', 1, 12*length(chs.PRBSet));
    for j=1:14
      est(ind, j) = pEst(drsIndices(1:length(ind)));
    end
end