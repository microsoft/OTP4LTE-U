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

rng(0);
tbs = 5504;


nBlockLength = tbs;
nCodeRate = 1/3;
aEb_No = 0:(0.5):10;



aBitArray = vertcat(randi(2,1,nBlockLength)-1);
D=encodeTurbo1(aBitArray);


aY = horzcat(D(1,:), D(2,:), D(3,:));
% BPSK modulate
aTxSignal = modulateBPSK(aY);
% Send through AWGN channel
noise = (1)*(randn(1,length(aTxSignal)))/sqrt(100);

%No noise for debugging
%aRxSignal = aTxSignal + noise;
aRxSignal = aTxSignal;


%Demodulate received signal
aR = aRxSignal;
aRp = demodulateBPSK(aRxSignal);

D = reshape(transpose(aR),length(aRxSignal)/3,3)';


assert(rem(length(aRxSignal),3)==0);
%[aDecodedMessage,aDecodedMessage2, aLLR1, aLLR2] = decodeTurbo(D, size(aBitArray,2), 3, std(noise));
%D(1,:)=0;
%D(2,:)=0;
D(3,:)=0;

L=size(aBitArray,2);
M=3;
[aDecodedMessage,aLLR]=decodeBCJRSoft(D, size(aBitArray,2), 3, std(noise), zeros(1,L+M));


fprintf('\n\n The SNR is %d db\n', 10*log10(var(aTxSignal)/var(noise))); 
fprintf('number of bit flips in the code is %d, percentage is %f\n', length(find(aRp ~= aY)), length(find(aRp ~= aY))/length(aRp));
fprintf('number of bit flips in the message after decoding is %d \n', length(find(aDecodedMessage(1:length(aBitArray)) ~= aBitArray)));
fprintf('\n\naLLR absolute sum = %f\n', sum(abs(aLLR))/length(aLLR));
%fprintf('number of bit flips in the message2 after decoding is %d \n', length(find(aDecodedMessage2(1:length(aBitArray)) ~= aBitArray)));

