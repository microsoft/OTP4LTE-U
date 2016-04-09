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

% This function detects an asynchronous PRACH, without assuming frame synchronization
% It can detect any shift of a given root sequence
% The root sequence is given as preambleTable(1+preambleNum,:)
function [offset, bestDetVal] = detectUE(waveform, preambleTable, preambleNum)
    tCp = 198;
    x = waveform(tCp/2 + (1:1536)); % step in half a cyclic prefix
    
    % The fully sampled preamble has 24576 samples = 800us (preamble duration) * 30.72 MHz (LTE sampling rate)
    % We sample at the smallest sampling frequency of 30.72 MHz / 16 = 1.92 MHz
    % Thus, we take 800us (preamble duration) * 1.92 MHz (LTE sampling rate) = 1536 samples = 512 * 3

    xF = fft3x512(x);
    %xF = fft3x512BW(x, 'int32sora');

    nZc = 839;
    xFs = [xF(1118:1536) xF(1:420)]; % fft shift and useful carrier extraction
    postFftPwrScaleBits = 9; %3 bits per amplitude lost in fft, 6 bits in pwr

    bestDetVal = 3e5; %detection threshold, this is for ca 10dB SNR and good timing estimate
    index = -inf; offset = inf;

    % BOZIDAR: Here we really need an AGC for a complete implementation (instead of /2^10)
    xD = floor((2^14 + xFs .* preambleTable(1+preambleNum,:))/(2^10)); %derotate preamble according to assumed sequence number
    xD2 = xD(2:end) - xD(1:(end-1)); %detection metric - channel estimate with minimum carrier-to carrier changes has smallest metric
    d = floor(2^20 * floor( xD2 * xD2' ) / max(1,floor( xD * xD' )) ); %64bit/32bit

    xDdiff = floor( (2^14 + xD(2:end) .*conj(xD(1:(end-1))))/(2^postFftPwrScaleBits) ); %work out angle change per sample
    angleTime = angle16bitBW( round(mean(xDdiff)) ); %average angle change
    o =  floor( (2^14 + 12288 * angleTime) / (2^15) ); %timing offset

    
    % Find the best angle and then correlate
    adj = (angle( round(mean(xDdiff)) ) ) / (2*pi/nZc);  % just for printout
    adjn = (angle( round(mean(xDdiff)) ) );
    xFsP = xFs .* exp(1i* (-adjn) *[1:nZc]); 
    
    xDP = floor((2^14 + xFsP .* preambleTable(1+preambleNum,:))/(2^10)); %derotate preamble according to assumed sequence number
    xDP2 = xDP(2:end) - xDP(1:(end-1)); %detection metric - channel estimate with minimum carrier-to carrier changes has smallest metric
    dP = floor(2^20 * floor( xDP2 * xDP2' ) / max(1,floor( xDP * xDP' )) ); %64bit/32bit
    xDdiffP = floor( (2^14 + xDP(2:end) .*conj(xDP(1:(end-1))))/(2^postFftPwrScaleBits) ); %work out angle change per sample
    

    %x(1:10)
    %xF(1:10)
    %fprintf('%d ',real(xF(1:10))); fprintf('%d ',imag(xF(1:10))); fprintf('\n');
    %xD(1:10)
    %xD2(1:10)
    %xDdiff(1:10)

% $$$     round(mean(xDdiff))
% $$$     angle16bitBW( round(mean(xDdiff)) )
% $$$         
% $$$     xFs(1:10)*1e3
% $$$     xFsP(1:10)*1e3
% $$$     xDP(1:10)
% $$$     xDP2(1:10)
% $$$     dP
    
    fprintf('%d %e %e %e %e %e %d %d %e %e\n', ...
            preambleNum, xD * xD', xD2 * xD2', mean(xD(2:end) .*conj(xD(1:(end-1)))), mean(xDdiff), angleTime, round(adj), o, d, dP);
    
    if (dP < bestDetVal)
      bestDetVal = dP; offset = round(adj);
    end
end
    
