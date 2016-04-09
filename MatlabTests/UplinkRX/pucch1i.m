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

function indices = pucch1i(ue, chs, drs)
    m = floor(chs.ResourceIdx / (3 * 12 / chs.DeltaShift)) + chs.ResourceSize; % assume Ncs = 0
    if mod(m, 2) == 0
        n0 = floor(m/2);
        n1 = ue.NULRB - 1 - floor(m/2);
    else
        n0 = ue.NULRB - 1 - floor(m/2);
        n1 = floor(m/2);
    end
    nSC = ue.NULRB * 12;
    if drs
        symbols = [2 3 4];
    else
        symbols = [0 1 5 6];
    end
    slot0 = n0*12 + bsxfun(@plus, nSC*symbols, (1:12)');
    slot1 = n1*12 + bsxfun(@plus, nSC*(symbols+7), (1:12)');
    indices = [slot0(:); slot1(:)];
end
