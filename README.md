# Embedded on pi

## Final Project for Empbedded systems 2021

### Compiling
First of, you must clone current repository with ``` $ git clone https://github.com/al1998petridis/Embedded-on-pi.git ``` 

Go to clone folder and change line 17 from file Makefile

For local execution: ``` $ CC = gcc ```
    
For Raspberry pi 3 execution: ``` $ CC = arm-linux-gnueabihf-gcc ```
    
Open up Bash into folder ``` $ cd ../Final/ProjectFiles ```

Run ``` $ make clean ```

Run ``` $ make ```

### Executing

For local execution: ``` $ make exec ```
   
For Raspberry pi execution: ``` $ scp covidExe root@10.0.0.1:/root ```
    
Connect to Rasberry pi with ``` $ ssh 10.0.0.1 -lroot ``` and your password

``` $ cd /root``` and run ``` $ ./covidExe ```
