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

function plotFrame(data)
figure(1);
clf(1);
hold on;
plot(real(data));
lm = max(abs(real(data)));
plot_grid(1, length(data), lm);
end



function plot_grid(s1, len, lm)
  x = s1;
  c = 0;
  while x < len
    y = 0;
    for i = 0:12
      if mod(i, 7) == 0
        y = y + 2048+160;
      else
        y = y + 2048+144;
      end
      %plot([x+y x+y], [-lm lm], 'g-');
    end
        
    if mod(c, 20) == 0
      plot([x x], [-lm lm], 'r-', 'LineWidth', 2);
% $$$       plot([0 0] + x + (2048+160 + (2048+144)*3), [-lm lm], 'g-');
% $$$       plot([0 0] + x + (2048+160*2 + (2048+144)*9), [-lm lm], 'g-');
    elseif mod(c, 10) == 0
      plot([x x], [-lm lm], 'r-');
    else
      if mod(c, 2) == 0
        plot([x x], [-lm lm], 'r:');
      end
    end
    x = x + (2048+160 + (2048+144)*6);
    c = c + 1;
  end

end

