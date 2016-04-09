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

function [y] = fft3x512(x)
%Radix 3 fft for PRACH reception purposes
%[y] = fft3x512BW(x)
%x must be length 1536 (baseband PRACH sampled at 1.92MHz)

N=1536; %final DFT length
N3 = N/3; %partial fft length, 512
if length(x) ~= N
  error('fft3x512BW is only to be used for length 1536 fft' )
end;

%split input into 3 sequences of 512
y1 = x(1:3:end);
y2 = x(2:3:end);
y3 = x(3:3:end);

y = zeros(size(x)) ;

%integer calculations, assume 16-bit IQ and 16-bit weights, and scale output power to below input power, note fft scales down by 1/n
    scale = 10923;
    halfScale = 16384;
    scaleBits = 15;
    fftScale = 512;

    % Code of sort floor(  ( halfScale + scale * round(fft(y1)/fftScale) )/2^scaleBits  ); 
    % is for rounding and we currently don't implement it in Ziria

    % yf1 is scaled differently than yf2 and yf3 because the scaling for the others come in wVec and w2Vec
    yf1 = floor(  ( halfScale + scale * round(fft(y1)/fftScale) )/2^scaleBits  ); 
    yf2 = round(fft(y2)/fftScale);                 % round(fft(y2)/(fftScale)) ~= Sora's FFT
    yf3 = round(fft(y3)/fftScale); 
    
    %this carries out y(n) = y1f(mod((n-1),N3)+1) + y2f(mod((n-1),N3)+1)*exp(-j*2*pi*(n-1)/N) + y3f(mod((n-1),N3)+1)*exp(-j*2*2*pi*(n-1)/N);
    wVec = round(scale*exp(-1i*(2*pi/N)*[0:(N-1)])); %constant
%    w2Vec = wVec .* wVec;
    w2Vec = round(scale*exp(-1i*(2*2*pi/N)*[0:(N-1)])); %constant
    %these are fixed tables that can be used on target
    
    %limit
    %wVec = complex(  max(-scale, min(scale-1, real(wVec))), max(-scale, min(scale-1, imag(wVec))) );
    %w2Vec = complex(  max(-scale, min(scale-1, real(w2Vec))), max(-scale, min(scale-1, imag(w2Vec))) );
    
    outVec3 = [0:(N3-1) 0:(N3-1) 0:(N3-1)];
    y =  yf1(1+outVec3) +...
         floor((halfScale + yf2(1+outVec3) .* wVec) / 2^scaleBits ) +... 
         floor((halfScale + yf3(1+outVec3) .* w2Vec) / 2^scaleBits );
       

end
