## Project 06

Erin Donaher | edonaher@nd.edu  
Maya Kuzak | mkuzak@nd.edu  
Andrew Hennessee | ahennes3@nd.edu  
Patrick Hsiao | phsiao@nd.edu
<br></br>

Contributions: 
All worked on code together
<br></br>
Milestone 6

<br></br>
For the milestone, you should use the same files as Project 6 and implement fs_debug.  

Your narration should describe the contributions of each group member.

Grading for Milestone 6 (aka the fifth milestone)
=====================================

Correct Operation - Varying File Systems (60%)

Operate correctly given a proper file block of varying sizes.  You do not need to assess whether or not the ratio of nodes to data is correct, only that the file system itself checks out.  Your output should match the expected output as described in the writeup for Project 6.  
Error Handling - Incorrect File Systems (30%)

Operate correctly without crashing on file blocks that are not set up correctly applying appropriate sanity checks to ensure robust operation.  For example, what if the various counts do not match up correctly (the disk block is 20 blocks but the super block says there are 1000 blocks)?  What if the magic number is incorrect?  What if a direct block or indirect block is out of range?  Note that if the magic number fails, you can stop right there.  
Narration / Documentation (10%)

Submit a proper README.md with group member information and an appropriate narration describing the contributions of the respective group members as well as a discussion of how you approached fs_debug. 

