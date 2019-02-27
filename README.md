# PA-GPSR
Improvement and Performance Evaluation of GPSR-Based Routing Techniques for Vehicular Ad Hoc Networks

Link: https://ieeexplore.ieee.org/document/8638929

This repository provides PA-GPSR for ns3 **v3.23**. Besides that, we have implemented two competitors (MM-GPSR and an updated code of [GPSR](https://code.google.com/archive/p/ns3-gpsr/)) also available in this repository.

# Installation
1. Install all required dependencies required by ns-3. You can find the commands [here](https://www.nsnam.org/wiki/Installation).
2. Download ns3.

```
 wget http://www.nsnam.org/release/ns-allinone-3.23.tar.bz2
 tar xjf ns-allinone-3.23.tar.bz2
 cd ns-allinone-3.23
 git clone https://github.com/CSVNetLab/PA-GPSR
 cp -a PA-GPSR/src/. ns-3.23/src/
 cp -a PA-GPSR/examples/. ns-3.23/scratch/
 cp -a PA-GPSR/results/ ns-3.23/
 cp -a PA-GPSR/figures/ ns-3.23/
 cp -a PA-GPSR/scripts/ ns-3.23/
 cp PA-GPSR/main.sh ns-3.23/

 cd ns-3.23
 CXXFLAGS="-Wall -std=c++0x" ./waf configure --enable-examples
 ./waf
 sudo ./waf install
```
# Running

On the terminal, execute the command: 

```
sh main.sh
```

You can change the parameters of simulation through this file. The parameters are:

- nCores -> **Number of parallel executions bounded by the number of cores that your computer has.**
- algorithms -> **Algorithm name that you want to simulate. Can be pagpsr, gpsr and mmgpsr.**
- seeds -> **A vector of random seeds (size = 30) to perform the simulation.**
- speed -> **Maximum speed of vehicles.**
- cbrconn -> **Vector containing the numbers of source-destination pairs.**

# Generating Results

Run the "plot_results.m" script located at "scripts" folder to plot the graphs.

Parameters: 

- n_seeds -> **Number of seeds used in the simulation**
- simtime -> **Simulation time**
- time_vector -> **The vector containing the numbers of vehicles used in simulation.**
- n_pairs -> **Number of source-destination pairs used in simulation**

## Packet Loss Rate

![](https://github.com/CSVNetLab/PA-GPSR/blob/master/figures/plr_5.png)
![](https://github.com/CSVNetLab/PA-GPSR/blob/master/figures/plr_10.png)
![](https://github.com/CSVNetLab/PA-GPSR/blob/master/figures/plr_15.png)
![](https://github.com/CSVNetLab/PA-GPSR/blob/master/figures/plr_20.png)

## Delay (ms)

![](https://github.com/CSVNetLab/PA-GPSR/blob/master/figures/delay_5.png)
![](https://github.com/CSVNetLab/PA-GPSR/blob/master/figures/delay_10.png)
![](https://github.com/CSVNetLab/PA-GPSR/blob/master/figures/delay_15.png)
![](https://github.com/CSVNetLab/PA-GPSR/blob/master/figures/delay_20.png)

## Network Yield

![](https://github.com/CSVNetLab/PA-GPSR/blob/master/figures/yield_5.png)
![](https://github.com/CSVNetLab/PA-GPSR/blob/master/figures/yield_10.png)
![](https://github.com/CSVNetLab/PA-GPSR/blob/master/figures/yield_15.png)
![](https://github.com/CSVNetLab/PA-GPSR/blob/master/figures/yield_20.png)


Contact
============
* Andrey Silva, Brazil, andreysilva.eng@gmail.com, atorresd@mtu.edu
* Niaz Reza, USA, kreza@mtu.edu
* Aurenice Oliveira, USA, oliveira@mtu.edu

How to reference PA-GPSR?
============
Please use the following bibtex :

# Citation
``` bibtex

@ARTICLE{silva2019improvement, 
author={A. {Silva} and N. {Reza} and A. {Oliveira}}, 
journal={IEEE Access}, 
title={Improvement and Performance Evaluation of GPSR-Based Routing Techniques for Vehicular Ad Hoc Networks}, 
year={2019}, 
volume={7}, 
number={}, 
pages={21722-21733}, 
keywords={Routing;Ad hoc networks;Routing protocols;Safety;Wireless communication;Delays;GPSR;Routing protocol;VANETs;SUMO;NS-3}, 
doi={10.1109/ACCESS.2019.2898776}, 
ISSN={2169-3536}, 
month={Feb},}
```
