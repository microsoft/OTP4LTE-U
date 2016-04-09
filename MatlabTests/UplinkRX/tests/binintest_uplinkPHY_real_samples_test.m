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
addpath ../../DnlinkTX;
addpath ..;
  

% $$$ f = fopen('bintest_uplinkPHY_real_samples1.infile', 'rb');
% $$$ startUL1 = 352580;   % Real start: 352524
% $$$ startUL2 = 598318;
% $$$ startUL3 = 844060;

% DEBUG
% $$$ f = fopen('out_8RB_1.infile', 'rb');
% $$$ startUL2 = 1540953-224;
% $$$ startPRACH = 1540953-224 - 22*(7*2048 + 6*144+160);

%din = load('test_waveform1.txt')';

f = fopen('binintest_uplinkPHY_real_samples1.infile', 'rb');
%f = fopen('rx_test.infile', 'rb');

%f = fopen('doug_capture_3.infile', 'rb');
din = fread(f, inf, 'int16');
din = din(1:2:end-1) + i*din(2:2:end);
fclose(f);


%startUL = startUL2;
%waveform = din(startUL + (1:30720));
%waveform = din(startPRACH:1.85e6);
waveform = din;
%waveform = din(3*30720/2 + (1:30720/2));

% DEBUG
% First packet (subframe=2, RV=0)
%waveform = din(11*2*(7*2048 + 6*144+160)+(1:30720));
% Second packet (subframe=0, RV=2)
%waveform = din(19*2*(7*2048 + 6*144+160)+(1:30720));


% Writing out a subset:
% $$$ fw = fopen('rx_test.infile', 'wb');
% $$$ wa = reshape([real(waveform), imag(waveform)]', length(waveform)*2,1);
% $$$ fwrite(fw, wa, 'int16');
% $$$ fclose(fw);


% $$$ figure; 
% $$$ clf; 
% $$$ hold on;
% $$$ plot(real(din));
% $$$ y = max(abs(real(din)));
% $$$ cnt = 0;
% $$$ while cnt < length(din)
% $$$   plot([cnt cnt], [-y y], 'r:');       
% $$$   cnt = cnt + 1*(7*2048+6*144+160);
% $$$ end
% $$$ aaa





%%% Code for debugging starting position based on CP auto-correlation 
% $$$ D = [];
% $$$ for d = 1:length(waveform)-141-2048
% $$$   D = [D, waveform(d+(1:140))' * waveform(2048+d+(1:140)) / length(waveform)];
% $$$ end
% $$$ figure; plot(abs(D));
% $$$ hold on
% $$$ for d=0:11
% $$$   plot(2208+d*2192+zeros(1,2), [0, 2e5], 'r:');
% $$$ end
% $$$ 
% $$$ aaa




nrb = 8; % TODO: waveform errors if nrb > 3
ue.NULRB = 50;
ue.NSubframe = 2;
ue.NCellID = 11;
ue.RNTI = 64;
chs.Modulation = 'QPSK';
chs.PRBSet = (0:(nrb-1))';
chs.RV = 0;
trblk = rand(208, 1) < 0.5; % use the correct TB length

nFFT = 2048;
%nFFT = 1024;



% SC-FDMA
rx.grid = demodulate(nFFT, ue.NULRB, waveform);
%assert(all(all(abs(rx.grid - grid) < 5e-13)));


% Estimate channel using DRS
est = estimate(ue, chs, rx.grid);
%DEBUG
%est = ones(size(est))*exp(i*2.56);

% PUSCH
indices = puschi(ue, chs);
rx.precoded = equalize(rx.grid, est, indices);
rx.symbols = deprecode(rx.precoded, nrb);
rx.codeword = psch_decode(ue, ue.RNTI, rx.symbols);
%assert(isequal(rx.codeword, codeword));


figure(1); 
clf(1);
X = 1:96;
subplot(2,1,1); hold on;
plot(X, abs(rx.symbols(X)));
subplot(2,1,2); hold on;
plot(X, angle(rx.symbols(X)));

figure(2); 
clf(2);
X = 1:96;
plot(real(rx.symbols(X)), imag(rx.symbols(X)), '.');


% ULSCH
interleaver = interleaverULSCH(nrb);
rx.e(interleaver) = rx.codeword;
rx.trblk = sch_decode(rx.e', numel(trblk), chs.RV, 1);
%assert(isequal(rx.trblk, trblk));


