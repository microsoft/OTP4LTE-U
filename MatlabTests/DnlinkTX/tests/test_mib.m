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

addpath ..

frame = 6;
subframe = 0;

tx.enb = struct('CellRefP',1,'NDLRB',50,'NCellID',11,'NFrame',frame,...
                'NSubframe',subframe,'Ng',1,'PHICHDuration','Normal','CFI',3);
tx.Nid1 = floor(tx.enb.NCellID / 3); tx.Nid2 = mod(tx.enb.NCellID, 3);

% Scheduling. SIB1 fixed in spec, we set SIB2 period in SIB1
tx_pss = mod(tx.enb.NSubframe, 5) == 0;
tx_mib = tx.enb.NSubframe == 0;
tx_sib1 = mod(tx.enb.NFrame, 2) == 0 && tx.enb.NSubframe == 5;
tx_sib2 = mod(tx.enb.NFrame, 8) == 0 && tx.enb.NSubframe == 0;

% BCH
tx.mib = struct('NDLRB',tx.enb.NDLRB,'NFrameDiv4',floor(tx.enb.NFrame/4),...
                'Ng',tx.enb.Ng,'PHICHDuration',tx.enb.PHICHDuration);


m = mib(tx.mib);

f = fopen('test_mib.outfile.ground', 'w');
fprintf(f,'%d,', m);
fclose(f);
