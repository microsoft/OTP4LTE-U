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


% Moved parameters to the test files
% $$$ clear all;
% $$$ rng(0);
% $$$ nrb = 8; % TODO: waveform errors if nrb > 3
% $$$ trblklen = 16;
% $$$ 
% $$$ ue.NULRB = 50;
% $$$ ue.NSubframe = 0;
% $$$ ue.NCellID = 0;
% $$$ ue.RNTI = 0;
% $$$ chs.Modulation = 'QPSK';
% $$$ chs.PRBSet = (0:(nrb-1))';
% $$$ chs.RV = 0;
% $$$ trblk = rand(trblklen, 1) < 0.5; % use a valid TB length

% Transmit %

% ULSCH
e = dlsch(trblk, nrb*2*12*12, chs.RV);
interleaver = interleaverULSCH(nrb);
codeword = e(interleaver);

% PUSCH
symbols = pdsch(ue, ue.RNTI, codeword);
precoded = precode(symbols, nrb);
indices = puschi(ue, chs);

% DRS
drms = drs(ue, chs);
drsIndices = drsi(ue, chs);

% SC-FDMA
grid = zeros(12 * ue.NULRB, 14);
grid(indices) = precoded;
grid(drsIndices) = drms;
nFFT = 2048;
waveform = modulateSCFDMA(nFFT, grid);



% Receive %

% SC-FDMA
rx.grid = demodulate(nFFT, ue.NULRB, waveform);
assert(all(all(abs(rx.grid - grid) < 5e-13)));

% Estimate channel using DRS
est = estimate(ue, chs, rx.grid);

% PUSCH
rx.precoded = equalize(rx.grid, est, indices);
rx.symbols = deprecode(rx.precoded, nrb);
rx.codeword = psch_decode(ue, ue.RNTI, rx.symbols);
assert(isequal(rx.codeword, codeword));


% ULSCH
rx.e(interleaver) = rx.codeword;
rx.trblk = sch_decode(rx.e', numel(trblk), chs.RV);
assert(isequal(rx.trblk, trblk));
