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

% This function is what needs to be ported to Ziria (along with table loading).
function [index, offset, bestDetVal] = detectPRACH(waveform, preambleTable)
    tCp = 198;
    x = waveform(tCp/2 + (1:1536)); % step in half a cyclic prefix
    
    % The fully sampled preamble has 24576 samples = 800us (preamble duration) * 30.72 MHz (LTE sampling rate)
    % We sample at the smallest sampling frequency of 30.72 MHz / 16 = 1.92 MHz
    % Thus, we take 800us (preamble duration) * 1.92 MHz (LTE sampling rate) = 1536 samples = 512 * 3

    xF = fft3x512(x);
    %xF = fft3x512BW(x, 'int32sora');

    xFs = [xF(1118:1536) xF(1:420)]; % fft shift and useful carrier extraction
    postFftPwrScaleBits = 9; %3 bits per amplitude lost in fft, 6 bits in pwr

    bestDetVal = 3e5; %detection threshold, this is for ca 10dB SNR and good timing estimate
    index = -inf; offset = inf;
    for preambleNum = 0:size(preambleTable, 1)-1
        % BOZIDAR: Here we really need an AGC for a complete implementation (instead of /2^10)
        xD = floor((2^14 + xFs .* preambleTable(1+preambleNum,:))/(2^10)); %derotate preamble according to assumed sequence number
        xD2 = xD(2:end) - xD(1:(end-1)); %detection metric - channel estimate with minimum carrier-to carrier changes has smallest metric
        d = floor(2^20 * floor( xD2 * xD2' ) / max(1,floor( xD * xD' )) ); %64bit/32bit

        xDdiff = floor( (2^14 + xD(2:end) .*conj(xD(1:(end-1))))/(2^postFftPwrScaleBits) ); %work out angle change per sample
        angleTime = angle16bitBW( round(mean(xDdiff)) ); %average angle change
        o =  floor( (2^14 + 12288 * angleTime) / (2^15) ); %timing offset

        fprintf('%d %e %e %e %e %e %d %e\n', ...
                preambleNum, xD * xD', xD2 * xD2', mean(xD(2:end) .*conj(xD(1:(end-1)))), mean(xDdiff), angleTime, o, d);
        
        if (d < bestDetVal) && (abs(o) < (0.5*(30.72/1.92)*tCp))
            bestDetVal = d; offset = o; index = preambleNum;
        end
    end
    assert(index >= 0); assert(offset < inf);
end
    
