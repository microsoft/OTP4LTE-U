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

function config = configurePDSCH(enb, dci, rnti)
    config.RNTI = rnti;
    len = floor(dci.Allocation.RIV / enb.NDLRB) + 1;
    start = mod(dci.Allocation.RIV, enb.NDLRB);
    config.PRBSet = (start:start+len-1)';
    config.NLayers = enb.CellRefP;
    assert(isequal(dci.DCIFormat, 'Format1A'));
    config.Modulation = {'QPSK'};
    config.RV = dci.RV;
    config.TxScheme = 'TxDiversity';
    if enb.CellRefP == 1
        config.TxScheme = 'Port0';
    end
    config.NTurboDecIts = 5; % arbitrary, from LST example code
    
    % Removed from this structure
% $$$     if rnti <= 10 || rnti >= 65534 % RA/P/SI-RNTI
% $$$         NPRB = 2 + mod(dci.TPCPUCCH, 2);
% $$$         table = tbs_table();
% $$$         config.trblklen = table(dci.ModCoding+1, NPRB); assert(config.trblklen ~= 0);
% $$$     else
% $$$         config.trblklen = 328; % 328 bits for 12 RBs TODO: full tbs_table
% $$$     end
end
