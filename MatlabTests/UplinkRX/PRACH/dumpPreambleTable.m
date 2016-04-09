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

function dumpPreambleTable(prachSettings)
    nZc = 839;
    fdPrachTable = zeros(prachSettings.nPreambles, nZc);
    for preambleNum = 1:prachSettings.nPreambles
        [fdPrachSeq, ~, ~] = ulGenPrachSeqBW( ...
            prachSettings.nUlRb, prachSettings.frameNumber, prachSettings.subFrameNumber, ...
            preambleNum-1, prachSettings.prachConfigurationIndex, prachSettings.prachFrequencyOffset, ...
            prachSettings.rachRootSequence, prachSettings.zeroCorrelationZoneConfig, prachSettings.highSpeedFlag, prachSettings.betaPrach);
        fdPrachTable(preambleNum,:) = exp(1i*(2*pi/nZc)* 54 *[-((nZc-1)/2):((nZc-1)/2)]) .* conj(fdPrachSeq); %note complex conjugate needed for derotation
    end
    
    fScaleMax = max(max(abs(fdPrachTable)));
    fdPrachTable = int16(round( (32767/fScaleMax) * fdPrachTable ));
    
    % Pad preamble table up to 840 for Ziria x4 alignment.
    table = [fdPrachTable zeros(prachSettings.nPreambles, 1)].';
    f = fopen(['prach' '.table'], 'wt');
    fprintf(f, '%d,', real(table)); fprintf(f, '\n');
    fprintf(f, '%d,', imag(table));
    fclose(f);
end
