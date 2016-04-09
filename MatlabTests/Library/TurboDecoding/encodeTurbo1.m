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
% function D=encodeTurbo(aBitArray)
% 
% Turbo encoding (36.212, Section 5.1.3.2)
% 
% Input: 
% - aBitArray: horizontal array of bits to be encoded. Use -1 to denote <NULL> bits.
% 
% Output:
% - 3 horizontal arrays of encoded bits (with nulls denoted by -1), to be combined using interleaveTurboRateMatching
%
% TODO: 
% - Make Sure the other consititunet decoder is disabled while tail
% terminating the first one and viceversa.
function D=encodeTurbo(aBitArray)

%aBitArray = vertcat(randi(2,1,40)-1);
%load aBitArray
% Generate the internal interleaved sequence
aTCInterleave=getTurboCodeInterleaver();
ind = find(aTCInterleave(:,2) == length(aBitArray));
if isempty(ind)
  fprintf('Codeword length unsupported!\n');
  D = [];
  return;
end

%ToDo: Find deinterleavers for f1 and f2 corresponding to various block
%lengths using a simple algorithm presented in 'On quadratic inverses for
%quadratic permutation polynomials over integer rings - 2006 paper Ryu and
%Takesita'. Algorithm in Table-1, page 4.

% (according to 5.1.2 fillers from code block segmentation should be treated in the encoder as any NULL bits)
aNullIndex = find(aBitArray == -1);
aBitArray(aNullIndex) = 0;


f1 = aTCInterleave(ind, 3);
f2 = aTCInterleave(ind, 4);
index = 0:length(aBitArray)-1;
aInterleavedBitArray = aBitArray(mod(f1*index + f2*index.^2, length(aBitArray)) +1);

S1 = zeros(1,3);
S2 = S1;
D = zeros(3, size(aBitArray,2)+4);
K = size(aBitArray,2);

for j=0:size(aBitArray,2)+2 + 3
  nFeedback = mod(S1(2) + S1(3), 2);
  
  if (j < K || j > K + 2)
    nInterleavedFeedback = mod(S2(2) + S2(3), 2);
  end

  if j < size(aBitArray,2)
    nArrayBit = aBitArray(j +1);
    nInterleavedArrayBit = aInterleavedBitArray(j +1);
  else
    if (j <= size(aBitArray,2)+2)
      nArrayBit = nFeedback;      
    else
      nInterleavedArrayBit = nInterleavedFeedback;
    end
  end    

  if (j <= size(aBitArray,2)+2)
    xk = nArrayBit;
    zk = mod(nArrayBit + nFeedback + S1(1) + S1(3) , 2);
  end
  
  if (j < K || j > K + 2)
    zkp = mod(nInterleavedArrayBit + nInterleavedFeedback + S2(1) + S2(3), 2);
    xkp = nInterleavedArrayBit;
  end
  
  if j < K
    D(1, j +1) = xk;
    D(2, j +1) = zk;
    D(3, j +1) = zkp;
  elseif j == K
    % Add the termination bits
    D(0 +1, K +1) = xk;
    D(1 +1, K +1) = zk;
    %D(0 +1, K+2 +1) = xkp;
    %D(1 +1, K+2 +1) = zkp;
  elseif j == K+1
    % Add the termination bits
    D(2 +1, K +1) = xk;
    D(0 +1, K+1 +1) = zk;
    %D(2 +1, K +1) = xk;
    %D(0 +1, K+3 +1) = zkp;
  elseif j == K+2
    % Add the termination bits
    D(1 +1, K+1 +1) = xk;
    D(2 +1, K+1 +1) = zk;
    %D(1 +1, K+3 +1) = xkp;
    %D(2 +1, K+3 +1) = zkp;
  elseif j == K+3
    D(0 +1, K+2 +1) = xkp;
    D(1 +1, K+2 +1) = zkp;    
  elseif j == K+4
    D(2 +1, K+2 +1) = xkp;
    D(0 +1, K+3 +1) = zkp;
  elseif j == K+5
    D(1 +1, K+3 +1) = xkp;
    D(2 +1, K+3 +1) = zkp;
  end
    
  S1 = [mod(nArrayBit + nFeedback, 2), S1(1:end-1)];
  if (j < K || j > K + 2) % This if check is related to tail termination
    S2 = [mod(nInterleavedArrayBit + nInterleavedFeedback, 2), S2(1:end-1)];
  end
  %fprintf('\n\nj=%d\n',j);
  %S2
end


% Null the corresponding null bits
% (according to 5.1.2 fillers from code block segmentation should be treated in the encoder as any NULL bits)
D(1:2, aNullIndex) = -1;


