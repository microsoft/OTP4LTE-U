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

function O=butterfly(I)
N = length(I);
if (N == 4)
  O = I([0,2,1,3]+1);
elseif (N == 8)
  O = I([0,4,2,6,1,5,3,7]+1);
elseif (N == 12 || N == 24)
  O = butterfly3(I);
else
  O = butterfly4(I);
end
end

function O=butterfly4(I)
N = length(I);
O = zeros(size(I));
O1=O;
for k = 0:3
  O1(k*N/4+(1:N/4)) = butterfly(I(k*N/4+(1:N/4)));
end
for k = 0:N/4-1
  O(4*k+1) = O1(k+1);
  O(4*k+2) = O1(k+N/2+1);
  O(4*k+3) = O1(k+N/4+1);
  O(4*k+4) = O1(k+3*N/4+1);
end
end

function O=butterfly3(I)
N = length(I);
O = zeros(size(I));
O1=O;
for k = 0:2
  O1(k*N/3+(1:N/3)) = butterfly(I(k*N/3+(1:N/3)));
end
for k = 0:N/3-1
  O(3*k+1) = O1(k+1);
  O(3*k+2) = O1(k+N/3+1);
  O(3*k+3) = O1(k+2*N/3+1);
end
end

  