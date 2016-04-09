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

%
% When mapping to constants in C we have to map for all j=k*n
% where k=1,2,3 and n=0,1,...,N/4
% In C, we have wFFTLUT<N>_<k> being arr[N/4] iterated over n=0,1,...,N/4
%
W = @(j, N) (exp(-2*pi*i*j/N));


% C code:
% wFFTLUT<N>_<k> = W(k*(0:3), N)*2^15

% Different Radix algorithms:
% http://cache.freescale.com/files/dsp/doc/app_note/AN3680.pdf

%X = randi(2,1,1000)*2-3 + i*(randi(2,1,1000)*2-3);
%X = X/sqrt(2);
%for x=X
%  x=x*2^13;
%  fprintf('%d, %d, \n', round(real(x)), round(imag(x)));
%end

%N = 48;
N = 32;
X = load('../UplinkRX/tests/test_idfft.infile');
X = X(1:(2*N))/2^13;
X = X(1:2:end) + i*X(2:2:end);

Y = X;


if 0
arr = 0:3;                 % To mimic SSE operations (though it is not important)
for np=1:N/3/length(arr)
  n = (np-1)*length(arr)+1;
  a = Y(n+arr);
  b = Y(n+arr+N/3);
  c = Y(n+arr+2*N/3);
  Y(n+arr) = a+b+c;
  Y(n+arr+N/3) = (a+b*exp(-i*2*pi/3)+c*exp(-i*4*pi/3)) .* W((n-1+arr), N);
  Y(n+arr+2*N/3) = (a+b*exp(-i*4*pi/3)+c*exp(-i*2*pi/3)) .* W(2*(n-1+arr), N);
end
Ys=Y;

c = 3;
for l=0:c-1
  Y(l*N/c+(1:N/c)) = fft(Y(l*N/c+(1:N/c)));
end
Yf=Y;

intrlv = zeros(N,1);
for l=0:N/c-1
  intrlv(l*c+(1:c)) = l+(0:c-1)*(N/c);
end
Y = Y(intrlv+1);


Y
fft(X)
else
  Y = fft(X);
end


% Compare with Ziria
N=length(Y);
A = load('../UplinkRX/tests/test_idfft.outfile');
A=A(1:2:end)+i*A(2:2:end);
%c=512;    %N=12
%c=256;    %N=24
%c=128;
figure; subplot(2,1,1); plot(1:N, real(Y*c), 1:N, real(A)); subplot(2,1,2); plot(1:N, imag(Y*c), 1:N, imag(A))


aaa



arr = 0:3;

for n=1:N/4/4
  a = Y(n+arr);
  b = Y(n+N/4+arr);
  c = Y(n+N/2+arr);
  d = Y(n+3*N/4+arr);

  ac = a+c;
  bd = b+d;
  a_c = a-c;
  b_d = b-d;
  
  Y(n+arr) = ac+bd;
  Y(n+N/4+arr) = (ac-bd) .* W(2*(n-1+arr), N);
  Y(n+N/2+arr) = (a_c - i*b_d) .* W(1*(n-1+arr), N);
  Y(n+3*N/4+arr) = (a_c + i*b_d) .* W(3*(n-1+arr), N);
end

Ys = Y;

Y(1:N/4) = fft(Y(1:N/4));
Y(N/4 + (1:N/4)) = fft(Y(N/4 + (1:N/4)));
Y(N/2 + (1:N/4)) = fft(Y(N/2 + (1:N/4)));
Y(3*N/4 + (1:N/4)) = fft(Y(3*N/4 + (1:N/4)));

intrlv = [ 0,    8,    4,    12,    2,    10,    6,    14,    1,    9,    5,    13,    3,    11,    7,    15] + 1;
Y = Y(intrlv);

Y*512
fft(X)*512
