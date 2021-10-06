# Embedded on pi

## Final Project for Empbedded systems 2021
### Compilation
First of, you must clone current repository with ``` $ git clone https://github.com/al1998petridis/Embedded-on-pi/ ``` 

Go to clone folder and change line 17 from file Makefile

For local execution: ``` $ CC = gcc ```
    
For raspberry pi execution: ``` $ CC = arm-linux-gnueabihf-gcc ```
    
Open up Bash into folder ``` $ cd ../Final/ProjectFiles ```

Run ``` $ make clean ```

Run ``` $ make ```

For local execution: ``` $ make exec ```
   
For raspberry execution: ``` $ scp covidExe root@10.0.0.1:/root ```
    
and into rasberry with ``` $ ssh 10.0.0.1 -lroot ``` and your password go to /root and run ``` $ ./covidExe ```
