# PA-GPSR
Improvement and Performance Evaluation of GPSR-Based Routing Techniques for Vehicular Ad Hoc Networks

This repository provides PA-GPSR for ns3 **v3.23**. Besides that, we have implemented two competitors (MM-GPSR and GPSR) also available in this repository.

# Installation
1. Install all required dependencies required by ns-3. You can find the commands [here](https://www.nsnam.org/wiki/Installation).


# Generating Results

Run the "plot_results.m" script located at "scripts" folder to plot the graphs.

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
@article{silva2019improvement,
  title={Improvement and Performance Evaluation of GPSR-Based Routing Techniques for Vehicular Ad Hoc Networks},
  author={Silva, Andrey and Reza, Niaz and Oliveira, Aurenice},
  journal={IEEE Access},
  year={2019},
  publisher={IEEE}
}
```
