function [lp, tx, rx, pdr, hop, delay, phyrx, phytx, yield] = getConfidenceInterval(filename, nseed)
%GETCONFIDENCEINTERVAL Summary of this function goes here
%   Detailed explanation goes here

z = 1.96; % 95% confidence interval
%z = 1.65; % 90%
n = nseed; %number of samples

matrix_table = readtable(filename);


%% newest matlab version code

%matrix = matrix_table.Variables;
%matrix = matrix(:,2:end);

%error_matrix = z * std(matrix)/sqrt(n);

%lp_error = matrix(:,1)./matrix(:,2);

%yield_mat = matrix(:,3)./(matrix(:,2).*matrix(:,5));

%lperror_matrix = z * std(lp_error)/sqrt(n);

%yielderror_matrix = z * std(yield_mat)/sqrt(n);

%yield = yielderror_matrix;
%lp = 100*lperror_matrix;
%tx = error_matrix(2);
%rx = error_matrix(3);
%pdr = error_matrix(4);
%hop = error_matrix(5);
%delay = error_matrix(6);
%phyrx = error_matrix(7);
%phytx = error_matrix(8);

%% if your'e getting error on line 9, use this code instead (old matlab version only)
yield = z*std(matrix_table.(char(matrix_table.Properties.VariableNames(4))) ... 
    ./ (matrix_table.(char(matrix_table.Properties.VariableNames(3))).* ...
    matrix_table.(char(matrix_table.Properties.VariableNames(6)))))/sqrt(n);

lp_error = z*std(matrix_table.(char(matrix_table.Properties.VariableNames(2))) ...
    ./matrix_table.(char(matrix_table.Properties.VariableNames(3))))/sqrt(n);
lp =  100*lp_error;

tx = z*std(matrix_table.(char(matrix_table.Properties.VariableNames(3))))/sqrt(n);
rx = z*std(matrix_table.(char(matrix_table.Properties.VariableNames(4))))/sqrt(n);
pdr = z*std(matrix_table.(char(matrix_table.Properties.VariableNames(5))))/sqrt(n);
hop = z*std(matrix_table.(char(matrix_table.Properties.VariableNames(6))))/sqrt(n);
delay = z*std(matrix_table.(char(matrix_table.Properties.VariableNames(7))))/sqrt(n);
phyrx = z*std(matrix_table.(char(matrix_table.Properties.VariableNames(8))))/sqrt(n);
phytx = z*std(matrix_table.(char(matrix_table.Properties.VariableNames(9))))/sqrt(n);
end
