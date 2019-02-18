function [lp, tx, rx, pdr, hop, delay, phyrx, phytx, yield] = doCalc(filename)
%DOCALC Summary of this function goes here
%   Detailed explanation goes here

matrix_table = readtable(filename);

%% newest matlab version code

%matrix = matrix_table.Variables;
%matrix = matrix(:,2:end);
%mean_matrix = mean(matrix);

%yield = mean(matrix(:,3)./(matrix(:,2).*matrix(:,5)));
%lp_error = matrix(:,1)./matrix(:,2);
%lp = 100*mean(lp_error);
%tx = mean_matrix(2);
%rx = mean_matrix(3);
%pdr = mean_matrix(4);
%hop = mean_matrix(5);
%delay = mean_matrix(6);
%phyrx = mean_matrix(7);
%phytx = mean_matrix(8);


%% if your'e getting error on line 9, use this code instead (old matlab version only)
yield = mean(matrix_table.(char(matrix_table.Properties.VariableNames(4))) ... 
    ./ (matrix_table.(char(matrix_table.Properties.VariableNames(3))).* ...
    matrix_table.(char(matrix_table.Properties.VariableNames(6)))));

lp_error = matrix_table.(char(matrix_table.Properties.VariableNames(2))) ...
    ./matrix_table.(char(matrix_table.Properties.VariableNames(3)));
lp =  100*mean(lp_error);
tx    = mean(matrix_table.(char(matrix_table.Properties.VariableNames(3))));
rx    = mean(matrix_table.(char(matrix_table.Properties.VariableNames(4))));
pdr   = mean(matrix_table.(char(matrix_table.Properties.VariableNames(5))));
hop   = mean(matrix_table.(char(matrix_table.Properties.VariableNames(6))));
delay = mean(matrix_table.(char(matrix_table.Properties.VariableNames(7))));
phyrx = mean(matrix_table.(char(matrix_table.Properties.VariableNames(8))));
phytx = mean(matrix_table.(char(matrix_table.Properties.VariableNames(9))));

end

