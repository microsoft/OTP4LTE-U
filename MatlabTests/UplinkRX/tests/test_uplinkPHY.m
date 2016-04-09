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

clear all;
rng(0);

addpath ../../DnlinkTX/;
addpath ..;


nrb = 8; % TODO: waveform errors if nrb > 3
trblklen = 16;

ue.NULRB = 50;
ue.NSubframe = 0;
ue.NCellID = 0;
ue.RNTI = 0;
chs.Modulation = 'QPSK';
%NB: Starting from NB 1 to test the offset as well
chs.PRBSet = 1+(0:(nrb-1))';
chs.RV = 0;
trblk = rand(trblklen, 1) < 0.5; % use a valid TB length


uplinkPHY;


%%% Input files for various stages

fi = fopen('test_uplinkPHY.infile', 'w');

Y = transpose(waveform*2^14*2^7);
Y = floor([real(Y); imag(Y)]);
fprintf(fi,'%d,', Y);
fclose(fi);

copyfile('test_uplinkPHY.infile', 'test_c_uplinkPHY_demodulate.infile');
copyfile('test_uplinkPHY.infile', 'test_c_uplinkPHY_equalize.infile');
copyfile('test_uplinkPHY.infile', 'test_c_uplinkPHY_deprecode.infile');
copyfile('test_uplinkPHY.infile', 'test_uplinkPHY_psch_decode.infile');



%%% Output files for various stages

f = fopen('test_uplinkPHY.outfile.ground', 'w');
fprintf(f, '%d,', rx.trblk);
fclose(f);

f = fopen('test_c_uplinkPHY_demodulate.outfile.ground', 'w');
g=transpose(reshape(rx.grid, size(rx.grid,1)*size(rx.grid,2),1));
fprintf(f, '%d,', round([real(g); imag(g)] * 1e3));
fclose(f);

f = fopen('test_c_uplinkPHY_equalize.outfile.ground', 'w');
fprintf(f, '%d,', round([real(rx.precoded)'; imag(rx.precoded)'] * 9e3));
fclose(f);

f = fopen('test_c_uplinkPHY_deprecode.outfile.ground', 'w');
fprintf(f, '%d,', round([real(rx.symbols)'; imag(rx.symbols)'] * 570 * 10));
fclose(f);

% With interleaving
f = fopen('test_uplinkPHY_psch_decode.outfile.ground', 'w');
fprintf(f, '%d,', rx.e);
fclose(f);
