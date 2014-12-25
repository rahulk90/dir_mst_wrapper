dir_mst_wrapper
===============

Wrapper around a C++ implementation for finding the directed MST (Edmonds-ChuLiu Algorithm) 

Details:
This code wraps around the maximum/minimum spanning tree implementation provided in:
http://edmonds-alg.sourceforge.net/

The wrapper is based off the testcase provided and 
uses the complete graph with infinite weights and 
sets the relevant weights of the desired graph to be fininte.

Setup:
1) Please download the code from the website and place this directory in the folder.
2) Point the LFLAGS and INCLUDES to the system boost installation you want to use
3) (Optional) Set the -DDEBUG flag in DEBUG to see output

Usage:
./mstwrapper <input_file> <output_file> <min|max>

Input File Format:
N
src1,dest1,weight1
src2,dest2,weight2
..
srcE,destE,weightE

N is the number of variables in the graph and there are E edges.

Output File Format:
idx1
idx2
..
idxK

The indices correspond to the index (begining at 0) of the edges specified in the input
file. In the above example, if idx1==0, then src1->dest1 was in the maximum spanning tree.
The tree is rooted at the source corresponding to idx1

Python Interface:
directedMST.py provides the interface to access the directed MST functionality through python.

Testcases:
tc1.txt,tc2.txt are two small test cases where the optimal 
spanning tree for the minimum and maximum case is obvious
