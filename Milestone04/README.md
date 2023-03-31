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