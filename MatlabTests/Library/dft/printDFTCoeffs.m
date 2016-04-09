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

function printDFTCoeffs(NN)

%
% When mapping to constants in C we have to map for all j=k*n
% where k=1,2,3 and n=0,1,...,N/4
% In C, we have wFFTLUT<N>_<k> being arr[N/4] iterated over n=0,1,...,N/4
%
W = @(j, N) (exp(-2*pi*i*j/N));


for N=NN
  if N == 12 || N == 24
    c = 3;
  else
    c = 4;
  end
  
  for j=1:c-1
    fprintf('__declspec(selectany) __declspec(align(16)) extern const\n');
    fprintf('COMPLEX16 wFFTLUT%d_%d[] =\n', N, j);
    fprintf('{\n');
    for k=0:(N/c-1)
      re = round(real(W(j*k, N))*2^15);
      im = round(imag(W(j*k, N))*2^15);
      % Check for overflows
      re = max(min(re, 2^15-1), -2^15+1);
      im = max(min(im, 2^15-1), -2^15+1);
      fprintf('\t{ %d, %d },\n', re, im);
    end
  fprintf('};\n');
  fprintf('template<> DSP_INLINE const vcs* FFT_GetTwiddleConst<%d, %d>() { return (vcs*)wFFTLUT%d_%d; }\n', N, j, N, j);
  end
end


for N=NN
  fprintf('__declspec(selectany) extern const short FFT%dLUTMap[] =\n', N);
  fprintf('{\n');
  B = butterfly([1:N])-1;
  for j=1:length(B)
    fprintf('%d, ', B(j));
  end
  fprintf('\n};\n');
  fprintf('template<> DSP_INLINE short FFTLUTMapTable<%d>(int i) { return FFT%dLUTMap[i]; }\n', N, N);
end

