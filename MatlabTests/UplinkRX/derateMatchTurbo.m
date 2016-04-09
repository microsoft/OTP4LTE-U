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

function dk = derateMatchTurbo(ek, D, rv)
    % TODO: filler bits in vk2?
    % TODO: puncturing, repetition averaging
    k = (1:D)';
    wk0 = interleave(k, 'turbo', 0);    % get correct padding positions
    wk1 = interleave(k, 'turbo', 0);    % get correct padding positions
    wk2 = interleave(k, 'turbo', 1);    % get correct padding positions
    wk12 = [wk1'; wk2'];                % interlace wk1, wk2
    wk = [wk0; wk12(:)];
    C = 32;
    R = ceil(D / C);
    Ncb = 3 * R * C;                    % TODO: have skipped all the Nsoft stuff
    k0 = R * (2 * ceil(Ncb / (8 * R)) * rv + 2);
    pad = sum(isnan(wk(1:k0)));
    assert(length(ek) >= 3*D);          % assume repetition
    ek = circshift(ek(1:3*D), [k0-pad 0]);  % extract one complete cycle
    wk(~isnan(wk)) = ek;                % replace non-padding with real data
    vk = reshape(wk, [], 3);
    vk0 = vk(:,1); vk12 = vk(:,2:3);
    vk12 = reshape(vk12, 2, [])';
    vk1 = vk12(:,1); vk2 = vk12(:,2);
    dk0 = deinterleave(vk0, 'turbo', 0);
    dk1 = deinterleave(vk1, 'turbo', 0);
    dk2 = deinterleave(vk2, 'turbo', 1);
    dk = [dk0 dk1 dk2];
end