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


function [aDecodedMessage,aDecodedMessage2, aLLR1, aLLR2]=decodeTurbo(D,nL,nM,nSigma)

pDebug = 0;
% f1=3;
% f2=10;
% f1inv=27;
% f2inv=10;
aTCInterleave=getTurboCodeInterleaver();
ind = find(aTCInterleave(:,2) == nL);

if isempty(ind)
  fprintf('Codeword length unsupported!\n');
  D = [];
  return;
end

f1 = aTCInterleave(ind, 3);
f2 = aTCInterleave(ind, 4);

%assert(size(D,2) == 44);
eLLR1 = zeros(1,nL+nM); % This is not just random assignment. This says that the initial aPriori information is zero.
eLLR2 = eLLR1;
eDeinterleavedLLR2 = eLLR2; % No need to deinterleave now because everything is 0

for nIteration=1:5
  [aDecodedMessage, aLLR1, eLLR1]=decodeBCJRSoft(D, nL, nM, nSigma, eDeinterleavedLLR2);
  
  if (pDebug)
    fprintf('\n\naLLR1 absolute sum = %f\n', sum(abs(aLLR1))/length(aLLR1));
  end

  D2=zeros(size(D));
  d3 = D(3,:);
  d1 = D(1,1:nL);
  index = 0:(nL-1);
  interleaver = mod(f1*index + f2*index.^2, nL) +1;
  aInterleavedRxArray = d1(interleaver);
  eLLR1 = eLLR1(1:nL);
  eInterLeavedLLR1 = eLLR1(interleaver);
  eInterLeavedLLR1 = horzcat(eInterLeavedLLR1,zeros(1,nM));% I think is is best to set tail LLR to zero because tail is different for both consitituent decoders
  D2(1,1:nL)=aInterleavedRxArray;
  D2(2,1:nL)=d3(1:nL);
  aTail2 = vertcat(D(:,end-1), D(:,end));
  D2(:,nL+1)=aTail2(1:3);
  D2(:,nL+2)=aTail2(4:6);

  %aDecodedMessage2=decodeHardViterbiLTECCC(D2, size(aBitArray,2), 3);
  [aDecodedMessage2, aLLR2, eLLR2] = decodeBCJRSoft(D2, nL, nM, nSigma, eInterLeavedLLR1);
  aDecodedMessage2 = aDecodedMessage2(1:nL);
  aLLR2 = aLLR2(1:nL);
  eLLR2 = eLLR2(1:nL);
  aDecodedMessage2(interleaver) = aDecodedMessage2;
  aLLR2(interleaver) = aLLR2;
  eDeinterleavedLLR2(interleaver) = eLLR2;
  eDeinterleavedLLR2 = horzcat(eDeinterleavedLLR2,zeros(1,nM));% Set tail LLR to zero;
  
  if (pDebug)
    fprintf('aLLR2 absolute sum = %f\n', sum(abs(aLLR2))/length(aLLR2));
  end
  
end

% fprintf('\n\n The SNR is %d db\n', 10*log10(var(aTxSignal)/var(noise))); 
% fprintf('\n\n number of bit flips in the code is %d, percentage is %f\n', length(find(aRp ~= aY)), length(find(aRp ~= aY))/length(aRp));
% fprintf('\n\n number of bit flips in the message after decoding is %d \n', length(find(aDecodedMessage(1:length(aBitArray)) ~= aBitArray)));
% fprintf('\n\n number of bit flips in the message2 after decoding is %d \n', length(find(aDecodedMessage2(1:length(aBitArray)) ~= aBitArray)));

