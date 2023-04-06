## Milestone 04
Erin Donaher | edonaher@nd.edu  
Maya Kuzak | mkuzak@nd.edu  
Andrew Hennessee | ahennes3@nd.edu  
Patrick Hsiao | phsiao@nd.edu

The purpose of this milestone is to convert threaded code to use condition variables instead of
continually attempting to access a protected part of memory.

Similar to Milestone 3, you are being provided a working skeleton for the code. Your task is to
add the following functionality:
- Robustly check and protect the various input values to make sure they are an appropriate
value
- Measure the total wall clock
- Modify the code to use condition variable. There are two versions for the push and pop
containing a cv annotation (condition variable). Modify those functions and uncomment
the #define


Results with 10 producers and 3000 iterations:
1:Total runtime: 43.232473s
  Drumroll please .... 49 occurrences of 'the'
  
2:Total runtime: 41.776771s
  Drumroll please .... 45 occurrences of 'the'

5:Total runtime: 42.901581s
  Drumroll please .... 42 occurrences of 'the'

8:Total runtime: 41.931736s
  Drumroll please .... 39 occurrences of 'the'

10:Total runtime: 42.918027s
  Drumroll please .... 36 occurrences of 'the'
  

The total runtime of the program varies slightly between runs, but not by much. The runtime ranges from about 41 to 43 seconds, which means that adding more consumers does not appear to have a significant impact on the runtime. The number of occurrences of the word 'the' decreases as the number of consumers increases. This makes sense because adding more consumers means that the messages are being consumed faster, which gives the producers less time to generate messages. As a result, there are fewer messages overall, and therefore fewer occurrences of any particular word. However, it's worth noting that the difference in the number of occurrences of 'the' between runs is relatively small. The range of 'the' occurrences is from 36 to 49, which is only a difference of 13 occurrences. This suggests that the number of consumers may not have a large impact on the overall content of the messages.
