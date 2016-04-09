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

% input is time aligned in waveform


nrb = 8; % TODO: waveform errors if nrb > 3
ue.NULRB = 50;
ue.NSubframe = 2;
ue.NCellID = 11;
ue.RNTI = 64;
chs.Modulation = 'QPSK';
chs.PRBSet = (0:(nrb-1))';
chs.RV = 0;
trblk = rand(208, 1) < 0.5; % use the correct TB length
nFFT = 1024;


% Subsample waveform
waveform_sampled = waveform(1:2:end);


% SC-FDMA
rx.grid = demodulate(nFFT, ue.NULRB, waveform_sampled);


% Estimate channel using DRS
est = estimate(ue, chs, rx.grid);

% PUSCH
indices = puschi(ue, chs);
rx.precoded = equalize(rx.grid, est, indices);
rx.symbols = deprecode(rx.precoded, nrb);
rx.codeword = psch_decode(ue, ue.RNTI, rx.symbols);


% ULSCH
interleaver = interleaverULSCH(nrb);
rx.e(interleaver) = rx.codeword;
rx.trblk = sch_decode(rx.e', numel(trblk), chs.RV, 1);


% $$$ figure(1); 
% $$$ clf(1);
% $$$ X = 1:96;
% $$$ subplot(2,1,1); hold on;
% $$$ plot(X, abs(rx.symbols(X)));
% $$$ subplot(2,1,2); hold on;
% $$$ plot(X, angle(rx.symbols(X)));
% $$$ 
% $$$ figure(2); 
% $$$ clf(2);
% $$$ X = 1:96;
% $$$ plot(real(rx.symbols(X)), imag(rx.symbols(X)), '.');


