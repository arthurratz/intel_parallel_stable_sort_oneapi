# Parallel "Stable" Three-Way QuickSort Performance Optimization Using Intel® Parallel Studio™ XE And Intel® oneAPI HPC Toolkit

### Author: Arthur V. Ratz @ Intel® DevMesh

## Overview / Usage
In the era of modern computing, the sorting has a large influence on commercial and scientific data processing. The sorting algorithms are commonly used for solving the variety of problems in many fields of science and engineering, including financial transactions processing, mathematical and economic statistics, linear algebra and functional optimization, data compression and visualization, cryptography, linguistic systems, timeline and task scheduling, log and report analysis, artificial intelligence (AI) and data mining, etc. There’s an entire class of the fastsort algorithms used for sorting data effectively. These algorithms have the number of known implementations such as std::sort or qsort functions implemented as a part of C/C++ standard libraries. However, the most of existing fastsort implementations become less efficient, especially when sorting the various of big data. To benefit in sorting of the typically huge amounts of data having more complex user-defined abstract data types (ADT), we obviously need an approach that allows to increase the overall performance of the fastsort over its known shortcomings and limitations. In this research project, I’ve discussed about the aspects of using the various of the Intel’s HPC libraries and tools to deliver a modern code, performing the “stable” sort of big data in parallel on many-core Intel® CPUs as well as the innovative GPUs and FPGA hardware acceleration targets. This, in turn, allows to perform the sort by 2x-11x times faster, depending on the hardware acceleration platform, on which the specific workloads are executed. Additionally, I will discuss how to evaluate the performance speed-up of the parallel “stable” sort, compared to the performance of the legacy sequential sort performed by using the std::sort and qsort routines. For that purpose, I will use the two different performance metrices, such as either the execution wall-time measurement or more complex Intel® V-Tune™ Profiler to examine the performance and scalability of the modern parallel code being discussed. This project contains the number of code samples, demonstrating the parallel "stable" sort, running it on the variety of the Intel's innovative hardware acceleration targets.

## Methodology / Approach
Here’s a series of articles, describing the basic concept and providing the useful guidelines for delivering a modern code, that implements an efficient parallel “stable” three-way quicksort, using Intel® Parallel Studio™ XE and the revolutionary new Intel® oneAPI HPC Toolkit:

"An Efficient Parallel Three-Way Quicksort Using Intel C++ Compiler And OpenMP 4.5 Library" - https://software.intel.com/en-us/articles/an-efficient-parallel-three-way-quicksort-using-intel-c-compiler-and-openmp-45-library

"How To Implement A Parallel "Stable" Three-Way Quicksort Using Intel C++ Compiler and OpenMP 4.5 library" - https://software.intel.com/en-us/articles/how-to-implement-a-parallel-stable-three-way-quicksort-using-intel-c-compiler-and-openmp-45

"How To Optimize A Parallel Stable Sort Performance Using The Revolutionary Intel® oneAPI HPC Toolkit" - https://software.intel.com/en-us/articles/how-to-optimize-the-parallel-stable-sort-performance-using-intel-oneapi-hpc-toolkit

## How To Build And Run This Code In Intel® DevCloud

### Intel® Xeon® Gold 6128 CPUs:
u43833@s001-n050:~$ cd ./parallel_stable_sort_oneapi && make cpu

u43833@s001-n050:~$ chmod +rwx parallel_stable_sort.cpu && ./parallel_stable_sort.cpu

### Intel® 9Gen Graphics NEO:
u43833@s001-n171:~$ qsub -I -l nodes=2:gpu:ppn=2

u43833@s001-n171:~$ cd ./parallel_stable_sort_oneapi && make gpu

u43833@s001-n171:~$ chmod +rwx parallel_stable_sort.gpu && ./parallel_stable_sort.gpu

### Intel FPG Emulator Program (Experimental):
u43833@s001-n083:~$ qsub -I -l nodes=2:fpga_compile:ppn=2

u43833@s001-n083:~$ cd ./parallel_stable_sort_oneapi && make femu

u43833@s001-n083:~$ qsub -I -l nodes=2:fpga_runtime:ppn=2

u43833@s001-n083:~$ chmod +rwx parallel_stable_sort.femu && ./parallel_stable_sort.femu

### Intel® PAC/FPGA Platform (Experimental):
u43833@s001-n083:~$ qsub -I -l nodes=2:fpga_compile:ppn=2

u43833@s001-n083:~$ cd ./parallel_stable_sort_oneapi && make fpga

u43833@s001-n083:~$ qsub -I -l nodes=2:fpga_runtime:ppn=2

u43833@s001-n083:~$ chmod +rwx parallel_stable_sort.fpga && ./parallel_stable_sort.fpga

## Technologies Used
Intel® Parallel Studio™ XE, Intel® oneAPI HPC Toolkit, Intel® C Compiler 19.0, OpenMP 4.5, Data Parallel C (DPC++), Intel® oneAPI Threading Building Blocks (TBB)
