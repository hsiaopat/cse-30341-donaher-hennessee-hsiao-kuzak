# Project 06

Erin Donaher | edonaher@nd.edu  
Maya Kuzak | mkuzak@nd.edu  
Andrew Hennessee | ahennes3@nd.edu  
Patrick Hsiao | phsiao@nd.edu

## Milestone 06

Note that our fs.c contains both magic numbers. Magic number 0xf0f03410 corresponds to image.5, image.20, and image.200. Magic number 0x30341003 corresponds to image.10, image.25, and image.100. While we tested with all images, the define for 0x30341003 is currently commented out.

To test, run:
```
>> make
>> ./simplefs images/image.XX XX
 simplefs> debug
```