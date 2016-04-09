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

addpath ..;
rng(0);


fdata = fopen('test_turbodec.outfile.ground', 'w');
fenc = fopen('test_turbodec.infile', 'w');


% All lens, as per Table 5.1.3-3, 36.212
LEN = [40, 48, 56, 64, 72, 80, 88, 96, 104, 112, 120, 128, 136, 144, 152, 160, 168, 176, 184, 192, 200, 208, 216, 224, 232, 240, 248, 256, 264, 272, 280, 288, 296, 304, 312, 320, 328, 336, 344, 352, 360, 368, 376, 384, 392, 400, 408, 416, 424, 432, 440, 448, 456, 464, 472, 480, 488, 496, 504, 512, 528, 544, 560, 576, 592, 608, 624, 640, 656, 672, 688, 704, 720, 736, 752, 768, 784, 800, 816, 832, 848, 864, 880, 896, 912, 928, 944, 960, 976, 992, 1008, 1024, 1056, 1088, 1120, 1152, 1184, 1216, 1248, 1280, 1312, 1344, 1376, 1408, 1440, 1472, 1504, 1536, 1568, 1600, 1632, 1664, 1696, 1728, 1760, 1792, 1824, 1856, 1888, 1920, 1952, 1984, 2016, 2048, 2112, 2176, 2240, 2304, 2368, 2432, 2496, 2560, 2624, 2688, 2752, 2816, 2880, 2944, 3008, 3072, 3136, 3200, 3264, 3328, 3392, 3456, 3520, 3584, 3648, 3712, 3776, 3840, 3904, 3968, 4032, 4096, 4160, 4224, 4288, 4352, 4416, 4480, 4544, 4608, 4672, 4736, 4800, 4864, 4928, 4992, 5056, 5120, 5184, 5248, 5312, 5376, 5440, 5504, 5568, 5632, 5696, 5760, 5824, 5888, 5952, 6016, 6080, 6144];

LEN = [576 3264 48 4096 5440 6144 40 2432 56 1120];


for nBlockLength = LEN
  aBitArray = vertcat(randi(2,1,nBlockLength)-1);
  D=encodeTurbo1(aBitArray);

  fprintf(fdata, '%d,', aBitArray);

  fprintf(fenc, '%d,', D(1,:));
  fprintf(fenc, '%d,', D(2,:));
  fprintf(fenc, '%d,', D(3,:));
end

fclose(fdata);
fclose(fenc);




% $$$ aY = horzcat(D(1,:), D(2,:), D(3,:));
% $$$ % BPSK modulate
% $$$ aTxSignal = modulateBPSK(aY);
% $$$ % Send through AWGN channel
% $$$ noise = (1)*(randn(1,length(aTxSignal)))/sqrt(100);
% $$$ 
% $$$ %No noise for debugging
% $$$ %aRxSignal = aTxSignal + noise;
% $$$ aRxSignal = aTxSignal;
% $$$ 
% $$$ 
% $$$ %Demodulate received signal
% $$$ aR = aRxSignal;
% $$$ aRp = demodulateBPSK(aRxSignal);
% $$$ 
% $$$ D = reshape(transpose(aR),length(aRxSignal)/3,3)';
% $$$ 
% $$$ 
% $$$ 
% $$$ assert(rem(length(aRxSignal),3)==0);
% $$$ %[aDecodedMessage,aDecodedMessage2, aLLR1, aLLR2] = decodeTurbo(D, size(aBitArray,2), 3, std(noise));
% $$$ %D(1,:)=0;
% $$$ %D(2,:)=0;
% $$$ D(3,:)=0;
% $$$ 
% $$$ L=size(aBitArray,2);
% $$$ M=3;
% $$$ [aDecodedMessage,aLLR]=decodeBCJRSoft(D, size(aBitArray,2), 3, std(noise), zeros(1,L+M));
% $$$ 
% $$$ 
% $$$ fprintf('\n\n The SNR is %d db\n', 10*log10(var(aTxSignal)/var(noise))); 
% $$$ fprintf('number of bit flips in the code is %d, percentage is %f\n', length(find(aRp ~= aY)), length(find(aRp ~= aY))/length(aRp));
% $$$ fprintf('number of bit flips in the message after decoding is %d \n', length(find(aDecodedMessage(1:length(aBitArray)) ~= aBitArray)));
% $$$ fprintf('\n\naLLR absolute sum = %f\n', sum(abs(aLLR))/length(aLLR));
% $$$ %fprintf('number of bit flips in the message2 after decoding is %d \n', length(find(aDecodedMessage2(1:length(aBitArray)) ~= aBitArray)));
% $$$ 
% $$$ 
