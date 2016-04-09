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

function [ outVec, tS] = downsampleBW( inVec, fClock, fWanted, downSamplingMode)
%downsampleBW downsamples vector using either linear interpolation or nearest neighbour
%Usage: [ outVec, tS] = downsampleBW( inVec, fClock, fWanted, downSamplingMode)
%fClock - original sample clock frequency
%fWanted - desired sample clock frequency
%downSamplingMode - either "nearest" of nearest neighbour, or "interp" for interpolation (linearly interpolated samples)
%or interpFp125 for fixed point implementation ready interpolation

if (fClock < fWanted)
  error('Downsampling only supported, fClock must be larger than fWanted');
end;

switch(lower(downSamplingMode))
  case 'nearest' %nearest neighbour
    % Simple resampling
    outVec = inVec(round(1:(fClock/fWanted):size(inVec,2)));
    tS = (1/fWanted)*[0:(size(outVec, 2)-1)];
    
  case 'interp'
    blockSize = 1e7;
    if (length(inVec) < blockSize)
      tClock = 1/fClock;
      tS = (1/fWanted)*[1:(floor((fWanted/fClock)*size(inVec, 2))-1)]; %timing for wanted samples
      nLow = floor(fClock.*tS);%sample prior to wanted
      tLowDiff = tS - nLow*tClock;%distance in time between wanted and sample prior to wanted
      outVec = fClock*( inVec(nLow).*(tClock-tLowDiff) + inVec(nLow+1).*tLowDiff );
    else
      tClock = 1/fClock;
      tS = (1/fWanted)*[1:(floor((fWanted/fClock)*size(inVec, 2))-1)]; %timing for wanted samples
      numBlocks = floor(length(tS) / blockSize);
      for count = 1:numBlocks
        tSblock = tS( (1+(count-1)*blockSize) : (count*blockSize) );
        nLow = floor(fClock.*tSblock);%sample prior to wanted
        tLowDiff = tSblock - nLow*tClock;%distance in time between wanted and sample prior to wanted
        outVec((1+(count-1)*blockSize) : (count*blockSize)) = fClock*( inVec(nLow).*(tClock-tLowDiff) + inVec(nLow+1).*tLowDiff );
      end;
      tSblock = tS( (1+numBlocks*blockSize) : end );
      nLow = floor(fClock.*tSblock);%sample prior to wanted
      tLowDiff = tSblock - nLow*tClock;%distance in time between wanted and sample prior to wanted
      outVec((1+numBlocks*blockSize) : length(tS)) = fClock*( inVec(nLow).*(tClock-tLowDiff) + inVec(nLow+1).*tLowDiff );
    end;
    
  case 'interpfp125'
    %fixed point, implementation ready, specifically for 40/30.72 conversion, multiple of 3.125us at a time
    % takes advantage of the fact that clock ratio is 125/96
    if ( (fClock ~= 40e6) || (fWanted / 1.92e6) ~= 2^nextpow2(fWanted / 1.92e6) || (mod(length(inVec), 125) ~= 0) )
      disp(['fClock = ' fClock ', fWanted = ' fWanted ', length(inVec) = ' int2str(length(inVec))])
      error('interpfp125 only supports 40MHz downconversion to 30.72MHz/2^[0..4] on k * 3.125us or k * 125 samples basis e.g. 1 LTE slot (20ksamples @ 40MHz)');
    end;
    
    scale_r16 = (2^15); %constant
    halfScale_r16 = scale_r16 / 2;%constant, 2^14
    
    rClock = 125; %constant
    rRate = 30.72e6 / fWanted; %constant
    rWanted = 96 / rRate; %constant
    
    nInBuffers = floor(length(inVec) / rClock);
    %last sample is spot on, but extend vector for consistent
    %interpolation calculations (to avoid condition checking)
    %note not needed in implementation so long as array index can
    %point past the useful data
    invecI_r16 = [real(inVec) 0];
    invecQ_r16 = [imag(inVec) 0];
    
    rVecF = rClock * (1:rWanted) / rWanted; %temp, not needed for implementation
    
    rVec_r16 = floor(rVecF); %constant vector for given sample rate
    w2nd_r16 = floor( (scale_r16-1) * ( rVecF - rVec_r16) ); %constant vector for given sample rate, note -1 to prevent overflow
    w1st_r16 = (scale_r16-1) - w2nd_r16; %constant vector for given sample rate, note -1 to prevent overflow
    
    outVec = zeros(1,nInBuffers*rWanted);
    tS = zeros(size(outVec));
    
    outVecI_r16 = zeros(1,rWanted); %temporary buffer
    outVecQ_r16 = zeros(1,rWanted); %temporary buffer
    
    %processing one buffer at a time, using lookup tables, note
    %the routine yields itself very well to pointer addressing
    for bufCount = 0:(nInBuffers-1)
      inRange  = bufCount*rClock + rVec_r16;
      outRange  = bufCount*rWanted + [1:rWanted];
      
      tS(outRange) = (1/fWanted)*outRange; % for plots only, not needed for implementation
      
      %linear interpolation, note scale present here ensures
      %15-bit interpolation accuracy, 32-bit accumulator is enough
      %since weights are complementary and add up to 32768
      
      outVecI_r16 = floor(( invecI_r16(inRange).*w1st_r16 + invecI_r16(inRange+1).*w2nd_r16 +halfScale_r16)/scale_r16); %(biased rounding)
      outVecQ_r16 = floor(( invecQ_r16(inRange).*w1st_r16 + invecQ_r16(inRange+1).*w2nd_r16 +halfScale_r16)/scale_r16); %(biased rounding)
      outVec(outRange) = complex(outVecI_r16, outVecQ_r16); %note vector assembly can be done on the fly or afterwards
      
    end;
    
  otherwise error(['Unknown downsampling mode ' globConst.downSamplingMode]);
end;

end

%possible other to show deterioration TODO
% Textbook resampling but might have inaccurate clock
% 40/30.72 ~ 13/10
%M = 10;
%L = 13;
%complexData = resample(complexData, M, L);
