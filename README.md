# PA-GPSR
Improvement and Performance Evaluation of GPSR-Based Routing Techniques for Vehicular Ad Hoc Networks

This repository provides PA-GPSR for ns3 v3.23. Besides that, we have implemented two competitors (MM-GPSR and GPSR) also available in this repository.


# Generating Results

Run the "plot_results.m" script located at "scripts" folder to generate the results.

Parameters: 

- n_seeds -> **Number of seeds used in the simulation**
- simtime -> **Simulation time**
- time_vector -> **The vector of the number of vehicles used in simulation. In our case this number was [30 50 70 90 110]**
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
