## Project 04

Erin Donaher | edonaher@nd.edu  
Maya Kuzak | mkuzak@nd.edu  
Andrew Hennessee | ahennes3@nd.edu  
Patrick Hsiao | phsiao@nd.edu
<br></br>

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

To test our program, we ran with ```input/BiggerFiles.txt``` as the input file, which contains 5 instances of ```data/Test.pcap```. We tested on ```student11.cse.nd.edu```. The results below reflect the processing of nearly 200,000 packets with the number of threads varying from 2 to 8. As a design choice, our program executes with 1 Producer thread and N-1 Consumer threads.

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