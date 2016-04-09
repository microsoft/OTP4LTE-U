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
trblklen = 208;

ue.NULRB = 50;
ue.NSubframe = 0;
ue.NCellID = 11;
ue.RNTI = 64;
chs.Modulation = 'QPSK';
%NB: Starting from NB 1 to test the offset as well
chs.PRBSet = 1+(0:(nrb-1))';
chs.RV = 0;
trblk = rand(trblklen, 1) < 0.5; % use a valid TB length
trblk(1:208) = [0,0,1,0,0,0,0,0,0,0,0,0,0,1,1,0,0,0,0,1,1,1,1,1,0,1,0,1,0,0,0,0,...
                1,0,0,0,1,1,1,0,0,1,0,1,1,1,0,0,1,1,0,1,0,0,1,1,1,0,1,1,1,1,1,0,...
                0,1,1,1,0,1,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,...
                0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,...
                0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,...
                0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,...
                0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0];


fi = fopen('rx_test.infile', 'w');


ue.NSubframe = 1;
uplinkPHY;
Y = transpose(waveform*2^14*2^7);
Y = floor([real(Y); imag(Y)]);
fprintf(fi,'%d,', Y);


ue.NSubframe = 2;
uplinkPHY;
Y = transpose(waveform*2^14*2^7);
Y = floor([real(Y); imag(Y)]);
fprintf(fi,'%d,', Y);

ue.NSubframe = 3;
uplinkPHY;
Y = transpose(waveform*2^14*2^7);
Y = floor([real(Y); imag(Y)]);
fprintf(fi,'%d,', Y);


fclose(fi);



f = fopen('rx_test1.outfile.ground', 'w');
fprintf(f, '%d,', rx.trblk);
fclose(f);

