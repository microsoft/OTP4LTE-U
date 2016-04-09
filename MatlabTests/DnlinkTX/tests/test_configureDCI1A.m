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

frame = 0;
subframe = 0;
  
tx.enb = struct('CellRefP',1,'NDLRB',50,'NCellID',11,'NFrame',frame,...
    'NSubframe',subframe,'Ng',1,'PHICHDuration','Normal','CFI',3);
tx.Nid1 = floor(tx.enb.NCellID / 3); 
tx.Nid2 = mod(tx.enb.NCellID, 3);

tx.sib = '684c42821910f66e82119246a80000000000';
tx.rnti = 65535; % SI-RNTI
k = mod(floor(tx.enb.NFrame/2), 4);

%tx.sib = '000000fe9f848000185a7a0220040800d009656aab07b7435ce46080';
%k = 0; % TODO: 36.321 5.3.1 last para; safe to hard-code 0 for our scheduler

rv = mod(ceil(3/2 * k), 4);

lenRB = 12;
startRB = 0;

% Testing each of the three columns in tbs_table

d = configureDCI1A(tx.enb, lenRB, startRB, length(tx.sib) * 4, rv, tx.rnti);
bits = dci(d);

d = configureDCI1A(tx.enb, lenRB, startRB, 224, rv, tx.rnti);
bits = [bits; dci(d)];


tx.rnti = 64; % Sample C-RNTI
d = configureDCI1A(tx.enb, lenRB, startRB, 328, rv, tx.rnti);
bits = [bits; dci(d)];

tx.rnti = 64; % Sample C-RNTI
d = configureDCI1A(tx.enb, lenRB, startRB, 1224, rv, tx.rnti);
bits = [bits; dci(d)];

tx.rnti = 64; % Sample C-RNTI
d = configureDCI1A(tx.enb, lenRB, startRB, 1864, rv, tx.rnti);
bits = [bits; dci(d)];

tx.rnti = 64; % Sample C-RNTI
d = configureDCI1A(tx.enb, 23, startRB, 2024, rv, tx.rnti);
bits = [bits; dci(d)];

tx.rnti = 64; % Sample C-RNTI
d = configureDCI1A(tx.enb,  8, startRB, 1096, rv, tx.rnti);
bits = [bits; dci(d)];


f = fopen('test_configureDCI1A.outfile.ground', 'w');
fprintf(f, '%d,', bits);
fprintf(f, '0,');
fprintf(f, '0,');
fprintf(f, '0');
% $$$ fprintf(f, '%d,', 2);
% $$$ fprintf(f, '%d,', dci.AllocationType);
% $$$ fprintf(f, '%d,', dci.HARQNo);
% $$$ fprintf(f, '%d,', dci.NewData);
% $$$ fprintf(f, '%d,', dci.RV);
% $$$ fprintf(f, '%d,', dci.Allocation.RIV);
% $$$ fprintf(f, '%d,', dci.TPCPUCCH);
% $$$ fprintf(f, '%d,', dci.ModCoding);
fclose(f);

