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

frame = 0;
subframe = 0;

tx.enb = struct('CellRefP',1,'NDLRB',50,'NCellID',11,'NFrame',frame,...
                'NSubframe',subframe,'Ng',1,'PHICHDuration','Normal','CFI',3);
grid = [];


k = mod(floor(tx.enb.NFrame/2), 4);
rv = mod(ceil(3/2 * k), 4);
tx.rnti = 65535; % SI-RNTI
lenRB = 12;
startRB = 5;
blklen = 328;

tx.dci = configureDCI1A(tx.enb, lenRB, startRB, blklen, rv, tx.rnti);
tx.pdschConfig = configurePDSCH(tx.enb, tx.dci, tx.rnti);
tx.pdschi = pdschi(tx.enb, tx.pdschConfig);
tx.grid = zeros(50*12,14);
tx.grid(tx.pdschi) = (1:length(tx.pdschi)) + 1i*(1:length(tx.pdschi));
grid = [grid, tx.grid];



k = mod(floor(tx.enb.NFrame/2), 4);
rv = mod(ceil(3/2 * k), 4);
tx.rnti = 64; % C-RNTI
lenRB = 8;
startRB = 0;
blklen = 208;

tx.dci = configureDCI1A(tx.enb, lenRB, startRB, blklen, rv, tx.rnti);
tx.pdschConfig = configurePDSCH(tx.enb, tx.dci, tx.rnti);
tx.pdschi = pdschi(tx.enb, tx.pdschConfig);
tx.grid = zeros(50*12,14);
tx.grid(tx.pdschi) = (1:length(tx.pdschi)) + 1i*(1:length(tx.pdschi));
grid = [grid, tx.grid];



k = mod(floor(tx.enb.NFrame/2), 4);
rv = mod(ceil(3/2 * k), 4);
tx.rnti = 64; % C-RNTI
lenRB = 23;
startRB = 0;
blklen = 2024;

tx.dci = configureDCI1A(tx.enb, lenRB, startRB, blklen, rv, tx.rnti);
tx.pdschConfig = configurePDSCH(tx.enb, tx.dci, tx.rnti);
tx.pdschi = pdschi(tx.enb, tx.pdschConfig);
tx.grid = zeros(50*12,14);
tx.grid(tx.pdschi) = (1:length(tx.pdschi)) + 1i*(1:length(tx.pdschi));
grid = [grid, tx.grid];



k = mod(floor(tx.enb.NFrame/2), 4);
rv = mod(ceil(3/2 * k), 4);
tx.rnti = 64; % C-RNTI
lenRB = 23;
startRB = 10;
blklen = 2024;

tx.dci = configureDCI1A(tx.enb, lenRB, startRB, blklen, rv, tx.rnti);
tx.pdschConfig = configurePDSCH(tx.enb, tx.dci, tx.rnti);
tx.pdschi = pdschi(tx.enb, tx.pdschConfig);
tx.grid = zeros(50*12,14);
tx.grid(tx.pdschi) = (1:length(tx.pdschi)) + 1i*(1:length(tx.pdschi));
grid = [grid, tx.grid];



p = real(grid);
f = fopen('test_addPDSCHToSubframe.outfile.ground', 'w');
fprintf(f,'%d,', p);
fclose(f);


