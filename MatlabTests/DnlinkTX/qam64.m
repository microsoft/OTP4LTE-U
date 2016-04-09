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

function symbols = qam64(bits)
    lut = [3 + 3j;
           3 + 1j;
           1 + 3j;
           1 + 1j;
           3 + 5j;
           3 + 7j;
           1 + 5j;
           1 + 7j;
           5 + 3j;
           5 + 1j;
           7 + 3j;
           7 + 1j;
           5 + 5j;
           5 + 7j;
           7 + 5j;
           7 + 7j;
           3 - 3j;
           3 - 1j;
           1 - 3j;
           1 - 1j;
           3 - 5j;
           3 - 7j;
           1 - 5j;
           1 - 7j;
           5 - 3j;
           5 - 1j;
           7 - 3j;
           7 - 1j;
           5 - 5j;
           5 - 7j;
           7 - 5j;
           7 - 7j;
          -3 + 3j;
          -3 + 1j;
          -1 + 3j;
          -1 + 1j;
          -3 + 5j;
          -3 + 7j;
          -1 + 5j;
          -1 + 7j;
          -5 + 3j;
          -5 + 1j;
          -7 + 3j;
          -7 + 1j;
          -5 + 5j;
          -5 + 7j;
          -7 + 5j;
          -7 + 7j;
          -3 - 3j;
          -3 - 1j;
          -1 - 3j;
          -1 - 1j;
          -3 - 5j;
          -3 - 7j;
          -1 - 5j;
          -1 - 7j;
          -5 - 3j;
          -5 - 1j;
          -7 - 3j;
          -7 - 1j;
          -5 - 5j;
          -5 - 7j;
          -7 - 5j;
          -7 - 7j;] / sqrt(42);
    values = reshape(bits, 6, [])' * [32;16;8;4;2;1];
    symbols = lut(values + 1);
end