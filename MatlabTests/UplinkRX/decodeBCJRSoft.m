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

% 
% function D=encodeTurbo(aBitArray)
% 
% Turbo encoding (36.212, Section 5.1.3.2)
% 
% Input: 
% - aBitArray: horizontal array of bits to be encoded. Use -1 to denote <NULL> bits.
% 
% Output:
% - 3 horizontal arrays of encoded bits (with nulls denoted by -1), to be combined using interleaveTurboRateMatching
%
% TODO: 
% - Make Sure the other consititunet decoder is disabled while tail
% terminating the first one and viceversa.
%  Lots of numerical issues are ther, for example the code will not work
%  for SNR=20db, because of numerical issues .. carefully deal with block
%  length etc.
% .. How to account for 2D noise function
function [aDecodedMessage, aLLR, eLLRout]=decodeBCJRSoft(D,nL,nM,nSigma,eLLR,nCC)

S1=zeros(1,nM);
% implementing a Viterbi Decoder first

%Construct Trellis % TODO: Avoid reconstructing Trellis for every decoding

n=size(D,2);

aPreviousStateCount = zeros(2^length(S1),n+2);

for t=1:(n+1)
  for k=1:2^length(S1)
      
      for nArrayBit = 0:1;
          
        S1(0 +1) = rem(k-1,2);
        S1(1 +1) = rem(floor((k-1)/2),2);
        S1(2 +1) = rem( floor((k-1)/4),2 );
      
        nFeedback = mod(S1(2) + S1(3), 2);
        
        Output(1,k,t,nArrayBit +1) = nArrayBit;
        Output(2,k,t,nArrayBit +1) = mod(nArrayBit + nFeedback + S1(1) + S1(3) , 2);
        
        S1 = [mod(nArrayBit + nFeedback, 2), S1(1:end-1)];
        
        nextState = S1(0 +1)*(2^0) + S1(1 +1)*(2^1) + S1(2 +1)*(2^2);
        NextState(k,t,nArrayBit +1) = nextState;
        
        nPreviousStates = aPreviousStateCount(nextState +1,t+1);
        PreviousState(nextState +1, t+1,nPreviousStates +1) = k -1;
        aPreviousStateCount(nextState +1,t+1) = nPreviousStates + 1;      
        assert(nPreviousStates+1 <= 2);
      
      end
  end
end

% Viterbi Decoder
Metric(1:2^length(S1),1:(n+1))=10000;
Metric(1,1)=0;

aAlpha = zeros(2^length(S1), nL + nM);
aBeta = zeros(2^length(S1), nL + nM);

aAlpha(:, 0 +1) = -Inf;
aAlpha(0 +1, 0 +1) = 0;

aBeta(:, nL + nM +1) = -Inf;
aBeta(0 +1, nL + nM +1) = 0;

% compute branch-metrics gamma, current implementation is for BPSK only
aGamma(2^length(S1),2^length(S1),nL + nM) = -Inf; % ToDO: avoid this memory requirement
for t=1:nL+nM
    for k=1:(2^length(S1)) 
        
        for nBit = 0:1
            nextState = NextState(k,t,nBit +1);
            y1 = Output(1,k,t, nBit +1);
            y2 = Output(2,k,t, nBit +1);
            
            % ToDo: The below code for handling tail termination looks
            % intimidating, put this in some function!
            if (t < n-3)
               r1 = D(1,t);
               r2 = D(2,t);
            elseif (t == n-3)
               r1 = D(1,t);
               r2 = D(2,t);               
            elseif ( t == n-2 )
               r1 = D(3,t-1);
               r2 = D(1,t);               
            elseif (t == n-1)
               r1 = D(2,t-1);
               r2 = D(3,t-1);               
            else % t=n
               r1 = D(1,t-1);
               r2 = D(2,t-1);
               assert(t==n);
            end
            
            if (nBit == 0) 
              nAPP = exp(eLLR(t))/(1 + exp(eLLR(t)));
            else
              nAPP = 1/(1 + exp(eLLR(t)));
            end
            nProb = exp( -( abs((r1 - (2*y1-1))^2) + abs((r2 - (2*y2-1))^2) )/(2*nSigma^2) );%/(2*pi*nSigma^2); %2y - 1 because 1 is mapped to 1 and 0 to -1; different mapping from tutorial
            nProb = nProb*nAPP; % need not do all this, exp in previous step, now multiplication and then log in next step, can directly perform addition in log domain
            
            %y1p = Output(1,k,t, mod(nBit+1,2) +1);
            %y2p = Output(2,k,t, mod(nBit+1,2) +1);
            %nProbp = exp( -( (r1 - y1p)^2 + (r2 - y2p)^2 )/(2*nSigma^2) )/(2*pi*nSigma^2);
            
            %nProb = nProb/(nProb+nProbp); % TODO: Very important. The algorithm refused to work without this normalization (on 20db like high SNR). DOuble check if this is valid.
            aGamma(k, nextState +1,t) = log(nProb); % TODO: need not do this, just eliminate exp in the previous line.
        end
        
    end
end

% feed forward calculate alphas via recursion
for t=2:nL+nM+1
    
    for k=1:(2^length(S1))        
        previousState1 = PreviousState(k, t, 1);
        previousState2 = PreviousState(k, t, 2);
        nPreviousStates = aPreviousStateCount(nextState +1,t+1);
        assert(nPreviousStates == 2);
         
        z1 = aAlpha(previousState1 +1, t-1) + aGamma(previousState1 +1,k,t-1);
        z2 = aAlpha(previousState2 +1, t-1) + aGamma(previousState2 +1,k,t-1);
        
        if (z1 == -Inf & z2 == -Inf)
           nAlpha = max( z1, z2 ) + log(2);
        else
           nAlpha = max( z1, z2 ) + log(1+exp(-abs(z1-z2))); % ToDo: Scope for computation versus performace tradeoff here by using max-star=max approximation
        end
        %assert(aGamma(previousState +1,k,t-1) ~= -Inf);
        
        aAlpha(k, t) = nAlpha;        
    end
    
end

% feed backward calculate betas via recursion
for t=nL+nM:-1:1
    
    for k=1:(2^length(S1))        
        %for nBit = 0:1
        nextState1 = NextState(k, t, 0 +1);
        nextState2 = NextState(k, t, 1 +1);
          
        z1 = aBeta(nextState1 +1, t+1) + aGamma(k, nextState1 +1, t);
        z2 = aBeta(nextState2 +1, t+1) + aGamma(k, nextState2 +1, t);

        if (z1 == -Inf & z2 == -Inf)
            nBeta = max( z1, z2 ) + log(2);
        else
            nBeta = max( z1, z2 ) + log(1+exp(-abs(z1-z2))); % ToDo: Scope for computation versus performace tradeoff here by using max-star=max approximation
        end
        assert(aGamma(k, nextState1 +1, t) ~= -Inf);
        %end
        aBeta(k, t) = nBeta;        
    end
    
end

aLLR = zeros(1,nL+nM);
% Finally calculate LLRs
for t=1:nL+nM
  %nProbability = zeros(1,2);
  nProbability(1:2)=-Inf;
  for k=1:(2^length(S1))     
      
      for nBit = 0:1       
        nextState = NextState(k, t, nBit +1);
        %nProbability(nBit +1) = nProbability(nBit +1) + exp(aAlpha(k, t) + aBeta(nextState +1, t+1) + aGamma(k, nextState +1, t));
        %nProbability(nBit +1) = max(nProbability(nBit +1), aAlpha(k, t) + aBeta(nextState +1, t+1) + aGamma(k, nextState +1, t)); % TODO: had to do max-star to max approximation here because couldn't think of a way to externd max-star to more than three
        z1 = nProbability(nBit +1);
        z2 = aAlpha(k, t) + aBeta(nextState +1, t+1) + aGamma(k, nextState +1, t);
        
        % Refer to the previous comment in the above commented out code, but still not sure if assumption
        % of associativity of max-star operation is ok.
        if (z1 == -Inf & z2 == -Inf)
           nProbability(nBit +1) = max( z1, z2 ) + log(2);
        else            
           nProbability(nBit +1) = max( z1, z2 ) + log(1+exp(-abs(z1-z2)));
        end
      end  
      
  end
  %aLLR(t) = log(nProbability(0 +1)) - log(nProbability(1 +1));
  aLLR(t) = (nProbability(0 +1)) - (nProbability(1 +1));
end

aDecodedMessage = zeros(1,nL+nM);
nOnes = find(aLLR < 0);
aDecodedMessage(nOnes) = 1;

%Compute Extrinsic LLR
for t=1:nL+nM  
  eLLRout(t) = aLLR(t) - eLLR(t) + 2*(D(1,t))/nSigma^2; % This sign of third term is different from what explained in tutorial because of difference in BPSK mapping.
end
;
