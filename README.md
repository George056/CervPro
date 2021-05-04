# CervPro

<img src="https://github.com/George056/CervPro/blob/main/Images/CervPro%20icon.png" alt="CervPro Logo" width="240"/>

## Designers
George Cook – Computer Engineer     
Nathan Vielmette – Electrical Engineer (team leader)     
Ethan Grimes – Mechanical Engineer     
Keneth Chelelgo – Mechanical Engineer     

## Description
This product was designed by the above team for Harding University's Engineering Department's senior design capstone project (class of 2020-21). Where we used what we had learned through our time at Harding to design a project of our choosing. This project is to be used by physical therapists in performing the cervical proprioceptive error test (CPE test). It tracks the patients face and calculates velocity and final angle difference.    

This was implemented using a Jetson Nano microprocessor and the PS3 PlayStation Eye camera. To prevent overheating the ICE cooling tower was attached to the Nano. It was powered by a PCB made by Nathan through a barrel power plug.

## Library Dependencies
OpenCV    
dlib

## Compiling
This program uses OpenCV and dlib, which both need to be compiled before compiling this program. When compiling it is easiest to do this by using CMake.

