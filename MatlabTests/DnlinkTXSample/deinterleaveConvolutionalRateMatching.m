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
% function deinterleaveConvolutionalRateMatching(aBitArray, nBlockLength, nNumberBlocks)
% 
% Implements Inverse of Rate matching block as defined in 36.212, Section 5.1.4.2
% 
% Input: 
% - aBitArray: Demodulated bit array from the receiver: Soft Values from
% Demodulation are welcome. ToDo: Placeholder argument for Soft Values
% - nBlockLength: Size of the block that was repeated while performing rate-matching(e.g. 120 (3x40) in case of PBCH)
% 
% Output:
% - systematic repetition of de-interleaved codeword for nNumberBlocks
% times
%
function E=deinterleaveConvolutionalRateMatching(aDecodedBitArray, nBlockLength, nNumberBlocks)

aBitMatrix = zeros(3,nBlockLength);
P = [1, 17, 9, 25, 5, 21, 13, 29, 3, 19, 11, 27, 7, 23, 15, 31, ...
     0, 16, 8, 24, 4, 20, 12, 28, 2, 18, 10, 26, 6, 22, 14, 30];
C = length(P);
R = ceil(size(aBitMatrix,2)/C);
ND = R*C-size(aBitMatrix,2);

W = zeros(1, 3*R*C);

% DEBUG
%aBitMatrix=[1:40; 41:80; 81:120];

for j=0:size(aBitMatrix,1)-1
  Y = [zeros(1,ND)-1, aBitMatrix(j +1,:)];
  Y = reshape(Y, C, R)';
  Y = Y(:, P+1);
  Y = reshape(Y, R*C, 1)';
  W((0:R*C-1)+j*R*C +1) = Y;
end

nNegOnes = find(W == -1);

W = zeros(1, 3*R*C);
W(nNegOnes) = -1;

P = [16, 0, 24, 8, 20,  4, 28, 12, 18, 2, 26, 10, 22, 6,  30,  14,  17,  1, 25, 9,  21,  5,  29, 13, 19, 3,  27,  11,  23,  7,  31,  15];
C = length(P);
R = ceil(nBlockLength/C);
ND = R*C-size(aBitMatrix,2);

% DEBUG
%aBitMatrix=[1:40; 41:80; 81:120];

k=0; j=0;
while (k < min(nNumberBlocks*nBlockLength*3,length(aDecodedBitArray)) )  
  if (W(j +1) ~= -1)
    W(j +1) = aDecodedBitArray(k +1);
    k = k+1;    
  end
  j = j+1;
 
end

if k < min(nNumberBlocks*nBlockLength*3)
   while (k < min(nNumberBlocks*nBlockLength*3) )  
     if (W(j +1) ~= -1)
        W(j +1) = -100; % Erasure Coding for puncturing
        k = k+1;    
     end
     j = j+1; 
   end    
end

for j=0:size(aBitMatrix,1)-1
  
  Y = W((0:R*C-1)+j*R*C +1);
  Y = reshape(Y, R, C);
  Y = Y(:, P+1);
  %assert(sum(Y(1,1:24)) == -24); % The check is specific to message size of 40
  
  Y = reshape(Y',C*R,1)';
  aBitMatrix(j +1,:) = Y(ND+1:end);
  
end

E=aBitMatrix;




