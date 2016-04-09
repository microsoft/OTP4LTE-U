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

% regi(enb) returns RE indices in the correct REG mapping order for Resource
% Block 0. This pattern can be repeated for all Resource Blocks using:
%   indices = bsxfun(@plus, 12*(0:enb.NDLRB-1), regi(enb));
% The repeated pattern should be equivalent to ltePDCCHIndices(enb) output,
% except the former will still contain PHICH and PCFICH indices.
function indices = regi(enb)
    assert(enb.CellRefP == 1); % TODO: need to produce indices for extra layers
    L = 1; if enb.CellRefP == 4; L = 2; end
    CFI = enb.CFI; if enb.NDLRB == 6; CFI = CFI + 1; end
    indices = [];
    crs_k = mod(enb.NCellID, 3); % CRS starting index
    res = (0:5)';
    res = res(mod(res, 3) ~= crs_k); % remove cell-specific pilots
    ks = ones(CFI, 1);
    l = 1;
    while ks(CFI) < 12 % pattern repeats after 12 subcarriers
        if l <= L
            indices = [indices enb.NDLRB*12*(l-1) + ks(l) + res];
            ks(l) = ks(l) + 6;
        else
            indices = [indices enb.NDLRB*12*(l-1) + ks(l) + (0:3)'];
            ks(l) = ks(l) + 4;
        end
        if l < CFI
            l = l + 1;
        elseif L < CFI && ks(L) > ks(L+1)
            l = L + 1;
        else
            l = 1;
        end
    end
    indices = indices(:);
end