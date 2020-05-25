#!/bin/bash
#  for-loopcmd.sh: for-loop with [list]
#+ generated by command substitution.
eval clear

nCores=7

algorithms="pagpsr"
seeds="1 50 98 150 250 890 2000 2035 7899 11040 11390 12000 12035 17899 21390 22000 22035 27899 31040 32035 871 33 90 1098 1176 543 9761 1394 8888 671"
sufix="\" &"
simprefix="./waf --run \"pagpsr-main"

speed="15"
cbrconn="5 10 15 20"
k=0


#removing old gpsr results
eval rm results/gpsr_results/pairs5/*.txt
eval rm results/gpsr_results/pairs10/*.txt
eval rm results/gpsr_results/pairs15/*.txt
eval rm results/gpsr_results/pairs20/*.txt

#removing old pagpsr results
eval rm results/pagpsr_results/pairs5/*.txt
eval rm results/pagpsr_results/pairs10/*.txt
eval rm results/pagpsr_results/pairs15/*.txt
eval rm results/pagpsr_results/pairs20/*.txt

#removing old mmgpsr results
eval rm results/mmgpsr_results/pairs5/*.txt
eval rm results/mmgpsr_results/pairs10/*.txt
eval rm results/mmgpsr_results/pairs15/*.txt
eval rm results/mmgpsr_results/pairs20/*.txt

start=30
end=30

for ((i=start; i>=end; i=i-20))
do


var=1
n=0
nSim=0
r=0
r2=0
     for cb in `echo $cbrconn`  
     do
	     for alg in `echo $algorithms`  
	     do
	      	      let n=n+1
		      for number in `echo $seeds`  
		      do
			let nSim=nSim+1
			if [ $var -gt $nCores ]
			then
			var=1
			sleep 5
			wait
			fi
			if [ $n -eq 1 ]
			let r=i+1
			let r2=i
			then
			    eval $simprefix "--conn=$cb --speed=15 --seed="$number "--size="$i "--newfile=false" "--algorithm="$alg$sufix
			fi
			if [ $? -eq 0 ]
			then
			    echo OK
			else
			    echo FAIL
			fi
			let var=var+1
			let n=0
			sleep 1
		      done

              done
      done
let k=k+1

done

wait
echo Simulation End 
