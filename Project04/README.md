## Project 04

Erin Donaher | edonaher@nd.edu  
Maya Kuzak | mkuzak@nd.edu  
Andrew Hennessee | ahennes3@nd.edu  
Patrick Hsiao | phsiao@nd.edu
<br></br>

The purpose of this project is to experiment with condition variables as well as performance
tuning using pthreads with a shared object. In addition, you will need to be judicious with
system resources so as not to overly consume all of the resources on the student machines.
The goals of this project are:

• To use condition variables in a producer / consumer arrangement

• To improve overall performance / efficiency

• To make judicious use of system resources

• To measure and experiment to maximize performance

Try it out yourself!

To run with a single data file:
```
>> make
>> ./redextract ./data/yourfile.pcap [-threads N]
```

To run with an input file:
```
>> make
>> ./redextract ./input/yourfile.txt [-threads N]
```

To test our program, we ran with ```input/BiggerFiles.txt``` as the input file, which contains 5 instances of ```data/Test.pcap```. We tested on ```student11.cse.nd.edu```. The results below reflect the processing time (in seconds) of nearly 200,000 packets per run with the number of threads varying from 2 to 8. As a design choice, our program executes with 1 Producer thread and N-1 Consumer threads.

| Threads | Run 1 | Run 2 | Run 3 |
| :---: | :---: | :---: | :---: |
| 2 | 7.742 | 7.586 | 7.553 |
| 3 | 7.358 | 7.538 | 7.195 |
| 4 | 7.849 | 7.287 | 7.871 |
| 5 | 7.756 | 7.164 | 7.531 |
| 6 | 7.356 | 7.361 | 7.564 |
| 7 | 7.093 | 6.957 | 7.085 |
| 8 | 7.160 | 7.315 | 7.481 |

Based on our results, we set the default number of threads to 7 (1 Producer and 6 Consumers).

Below are the results for the processing of individual tcpdump files:

| Test.pcap |
| :---: |
  Total Packets Parsed:    39968
  Total Bytes   Parsed:    30099026
  Total Packets Duplicate: 5
  Total Bytes   Duplicate: 6002
  Total Duplicate Percent:   0.02% | 

| testFile.pcap |
| :---: | 
  Total Packets Parsed:    430
  Total Bytes   Parsed:    441321
  Total Packets Duplicate: 0
  Total Bytes   Duplicate: 0
  Total Duplicate Percent:   0.00% |

| testFile2.pcap |
| :---: | 
  Total Packets Parsed:    10808
  Total Bytes   Parsed:    10395714
  Total Packets Duplicate: 1
  Total Bytes   Duplicate: 1432
  Total Duplicate Percent:   0.01% | 
  
| testSmall.pcap |
| :---: | 
  Total Packets Parsed:    114
  Total Bytes   Parsed:    65760
  Total Packets Duplicate: 7
  Total Bytes   Duplicate: 8627
  Total Duplicate Percent:  13.12%|

