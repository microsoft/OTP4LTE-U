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

function grid = demodulate(NDLRB, nFFT, waveform)
    fs = nFFT / 256 * 3.84e6;
    cpLength = [160, 144, 144, 144, 144, 144, 144] * nFFT / 2048;
    nSymbols = floor(length(waveform) / fs / 1e-3 * 14);
    nSC = NDLRB * 12;
    grid = zeros(nSC, nSymbols);
    start = 0;
    for n=0:nSymbols-1
        cp = cpLength(mod(n, 7)+1);
        cpOffset = fix(cp/2);           % skip into middle of CP
        phaseCorrection = exp(-1i*2*pi*(cp-cpOffset)/nFFT*(0:nFFT-1))';
        f = fft(waveform(start+cpOffset+(1:nFFT))) .* phaseCorrection;
        grid(:,n+1) = f([end-nSC/2+1:end, 2:nSC/2+1]);
        start = start + cp + nFFT;      % skip to end of symbol
    end
end