%% script to read the results from ns3
% Author: Andrey Silva
% andreysilva.eng@gmail.com

clear;
clc;
close all;

n_seeds = 30;
simtime = 200;
vec = 30:20:110;
n_points = length(vec);

n_pairs = '5'; %'5' '10' '15' '20'

str0 = strcat('../results/gpsr_results/pairs',n_pairs,'/gpsr');
str1 = strcat('../results/pagpsr_results/pairs',n_pairs,'/pagpsr');
str2 = strcat('../results/mmgpsr_results/pairs',n_pairs,'/mmgpsr');
stri = '_results.txt';
temp_str = int2str(vec(1));

for i=1:n_points
temp_str = int2str(vec(i));

filename = strcat(str0,temp_str,stri);
[gpsr_lp(i), gpsr_tx(i), gpsr_rx(i), gpsr_pdr(i), gpsr_hop(i), gpsr_delay(i),...
   gpsr_phyrx(i), gpsr_phytx(i), gpsr_yield(i)] =  doCalc(filename);

[error_lp(i), error_tx(i), error_rx(i), error_pdr(i), error_hop(i), error_delay(i),...
   error_phyrx(i), error_phytx(i), error_yield(i)] =  getConfidenceInterval(filename,n_seeds); 

filename2 = strcat(str1,temp_str,stri);

[pagpsr_lp(i), pagpsr_tx(i), pagpsr_rx(i), pagpsr_pdr(i), pagpsr_hop(i), pagpsr_delay(i),...
   pagpsr_phyrx(i), pagpsr_phytx(i), pagpsr_yield(i)] =  doCalc(filename2);

[paerror_lp(i), paerror_tx(i), paerror_rx(i), paerror_pdr(i), paerror_hop(i), paerror_delay(i),...
   paerror_phyrx(i), paerror_phytx(i), paerror_yield(i)] =  getConfidenceInterval(filename2,n_seeds); 

filename3 = strcat(str2,temp_str,stri);
[mmgpsr_lp(i), mmgpsr_tx(i), mmgpsr_rx(i), mmgpsr_pdr(i), mmgpsr_hop(i), mmgpsr_delay(i),...
   mmgpsr_phyrx(i) ,mmgpsr_phytx(i), mmgpsr_yield(i)] =  doCalc(filename3);

[mmerror_lp(i), mmerror_tx(i), mmerror_rx(i), mmerror_pdr(i), mmerror_hop(i), mmerror_delay(i),...
   mmerror_phyrx(i), mmerror_phytx(i),mmerror_yield(i)] =  getConfidenceInterval(filename3,n_seeds); 
end


figure;

a = errorbar(vec,pagpsr_lp,paerror_lp,'--k','LineWidth', 1);hold on;
b = errorbar(vec,gpsr_lp,error_lp,'k','LineWidth', 1);
c = errorbar(vec,mmgpsr_lp, mmerror_lp,'-.k','LineWidth', 1);

%a.CapSize = 14; % newest matlab version 
%b.CapSize = 14;
%c.CapSize = 14;
xlabel('Number of vehicles');
ylabel('Packet loss rate (%)');
[~, ~, ~, ~] = legend('PA-GPSR','GPSR', 'MM-GPSR');
hold off;
set(gca,'linewidth',1)

figure;

a = errorbar(vec,pagpsr_delay,paerror_delay,'--k','LineWidth', 1);hold on;
b = errorbar(vec,gpsr_delay,error_delay,'k','LineWidth', 1);
c = errorbar(vec,mmgpsr_delay, mmerror_delay,'-.k','LineWidth', 1);

%a.CapSize = 14;
%b.CapSize = 14;
%c.CapSize = 14;
xlabel('Number of vehicles');
ylabel('Average end-to-end delay (ms)');
[~, ~, ~, ~] = legend('PA-GPSR','GPSR', 'MM-GPSR');
hold off;

set(gca,'linewidth',1)


figure;

a = errorbar(vec,pagpsr_yield,paerror_yield,'--k','LineWidth', 1);hold on;
b = errorbar(vec,gpsr_yield,error_yield,'k','LineWidth', 1);
c = errorbar(vec,mmgpsr_yield, mmerror_yield,'-.k','LineWidth', 1);

%a.CapSize = 14;
%b.CapSize = 14;
%c.CapSize = 14;
xlabel('Number of vehicles');
ylabel('Network yield');
[~, ~, ~, ~] = legend('PA-GPSR','GPSR', 'MM-GPSR');
hold off;

set(gca,'linewidth',1)