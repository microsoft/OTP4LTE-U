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

function test_pdcchi
    warning('off','lte:defaultValue')
    enb.NDLRB = 50;
    enb.NCellID = 11;
    enb.CFI = 3;
    enb.PHICHDuration = 'Normal';
    enb.Ng = 'One';
    enb.NSubframe = 0;
    enb.CellRefP = 1;
    ue.RNTI = 64;
    ue.PDCCHFormat = 0; % num CCEs = 2**ue.PDCCHFormat
    
    dcibits = [0 0 0 0 1 0 1 0 1 1 1 1 1 0 0 0 0 0 0 0 1 0 0 0 0 0 0]';
    [dcistr, dcibits] = lteDCI(enb, dcibits); % just for verification of bits
    assert(isequal(dcistr.DCIFormat, 'Format0'));
    assert(dcistr.Allocation.RIV == 351);

    numCCEs = floor(1500/36);
    for cce = 0:numCCEs-1
        test_cce(enb, ue, dcibits, cce);
    end
end

function test_cce(enb, ue, dcibits, cce)
    % Create grid with toolbox.
    cw = lteDCIEncode(ue, dcibits);
    pdcchBits = -1*ones(3000, 1);
    pdcchBits(72*cce+(1:72)) = cw;
    sym = ltePDCCH(enb, pdcchBits);
    ind = ltePDCCHIndices(enb);
    grid = zeros(600, 14);
    grid(ind) = sym;
    
    % Create grid with our functions.
    enb.Ng = 1;
    e = dcch(dcibits, ue.RNTI);
    my_sym = pdcch(enb, e, cce);
    my_ind = pdcchi(enb);
    my_grid = zeros(600, 14);
    my_grid(my_ind(36*cce+(1:36))) = my_sym;
    
    surf(abs(grid - my_grid)); shading flat;
    assert(isequal(find(grid), find(my_grid)));
    assert(isequal(grid, my_grid));
end