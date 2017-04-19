# The One Memory Access sketch


## Introduction

The *sketch* is a probabilistic data structure and widely used for per-flow measurement in the real network. The key metrics of sketches are their memory usage, high accuracy, and high speed. There are various sketches in the literature, but they cannot achieve both high accuracy and high speed at the same time given a fixed memory size. To address this issue, we propose a new sketch, namely the OM sketch. It achieves much higher accuracy than the state-of-the-art, and achieves close to one memory access and one hash computation for each insertion or query. To verify the effectiveness and efficiency of our framework, we perform a series of experiments to compare the OM sketch with CM, CU, C sketch.


## About the source codes, dataset and parameters setting

We implement the CM, CU, C (short for the Count sketch) and OM sketch with C++. We complete these codes on Ubuntu 14.04 and compiled successfully using g++ 4.8.4.

The file stream.dat is the subset of one of the encrypted IP traces used in experiments. This small dataset contains 1M items totally and 193,894 distinct items. The maximum frequency of those items is 4426. The full dataset can be download on our homepage (http://net.pku.edu.cn/~yangtong/uploads/stream_full.dat).

We set the memory allocated to each sketch 0.1MB. The other parameters setting is the same as mentioned in the paper.


# How to run

Suppose you've already cloned the repository and start from the 'OM_Sketch_Framework' directory.

You just need:

	& make
	& ./main


## Output format

Our program will print the correct rate, the throughput of insertion and query of these sketches, the ARE and AAE of these sketches, and the average number of memory accesses of insertion and query of the OM sketch. Note that to obtain convincing results of the throughput, you are supposed to set the micro "testcycles" in the main.cpp to a larger value (e.g. 100) and to run the program on a Linux server.