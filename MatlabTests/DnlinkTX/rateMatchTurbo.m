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

function ek = rateMatchTurbo(dk, E, rv)
    vk0 = interleave(dk(:,1), 'turbo', 0);
    vk1 = interleave(dk(:,2), 'turbo', 0);
    vk2 = interleave(dk(:,3), 'turbo', 1);
    vk12 = [vk1'; vk2'];                % interlace vk1, vk2
    wk = [vk0; vk12(:)];
    R = length(vk0) / 32;
    % NOTE: Below is the original Doug's code that did not conform the specs
    % but passed the detection tests. We modified it to conform the specs and
    % still passes the test, so we use that version, but keep the old one for the record
    % OLD: Ncb = numel(dk);                    % TODO: have skipped all the Nsoft stuff
    Ncb = 3*32*R;                    % TODO: have skipped all the Nsoft stuff
    k0 = R * (2 * ceil(Ncb / (8 * R)) * rv + 2);
    wk = circshift(wk, [-k0, 0]);
    ek = wk(~isnan(wk));
    ek = ek(mod(0:E-1, length(ek)) + 1);
    
    %size(dk,1)*size(dk,2), length(dk), E, rv, length(vk0), length(vk1), length(vk2), Ncb, 0, k0
end
