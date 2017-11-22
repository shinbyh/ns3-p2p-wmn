# Spontaneous Interaction Networking Architecture: Part 1 (Routing)

This ns-3 scratch project aims to run an application-aware multi-constrained routing protocol in Fog Computing/Edge Computing networks.

## Assumptions
* I assume that all routing nodes are connected with each other via point-to-point links.
* Multiple constraints include: bandwidth, delay, jitter, packet loss rate.
* The routing protocol is based on source routing approaches.

## Prerequisites
* ns-3.26
* Python 3.X
* Python libraries: subprocess, regular expression (re).

## Usage
* Copy all source code files (.h, .cc) into [NS3-HOME]/scratch/p2p-bhshin. For this, you may want to create a new directory under [NS3-HOME]/scratch/.
* Copy all configuration text files (sina.config, .txt) and script files (.sh, .py) into [NS-3-HOME]/.
* Build ns-3 using ./waf.
```
$ ./waf
```
* Change sina.config as you want.
* Create or change 'Xapps_Xpkts.txt' file for making application traffic flows with multiple constraints.
* Run the simulation. You can change the argument for selecting a flow configuration file and a routing scheme.
```
$ ./waf --run "p2p-bhshin --apps=Xapps_Xpkts --scheme=0"
```
* If you want to save the screen output (e.g. output.txt), run as follows:
```
$ ./waf --run "p2p-bhshin --apps=Xapps_Xpkts --scheme=0" &> output.txt
```
