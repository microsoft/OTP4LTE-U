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

function indices = pdschi(enb, pdsch)
    nSC = enb.NDLRB * 12;
    switch enb.CellRefP
        case 1
            sym0 = (mod(enb.NCellID, 6) + 1):6:nSC; % CRS starting index
            sym4 = (mod(enb.NCellID, 3) + 1):6:nSC;
            crsi = bsxfun(@plus, nSC*[4 7 11], [sym4' sym0' sym4']);
        case 2
            crs_k = mod(enb.NCellID, 3) + 1; % CRS starting index
            crsi = bsxfun(@plus, nSC*[4 7 11], (crs_k:3:nSC)');
    end
    centre = enb.NDLRB * 6 - 36 + (1:72);
    switch enb.NSubframe
        case 0; pssi = bsxfun(@plus, nSC*(5:10), centre'); % PSS, SSS, PBCH
        case 5; pssi = bsxfun(@plus, nSC*(5:6),  centre'); % PSS, SSS
        otherwise; pssi = [];
    end
    SCs = bsxfun(@plus, 12*pdsch.PRBSet', (1:12)');
    REs = bsxfun(@plus, nSC*(enb.CFI:13), SCs(:));
    indices = setdiff(REs, crsi);
    indices = setdiff(indices, pssi);
end
