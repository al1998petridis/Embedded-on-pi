import numpy as np
import matplotlib.pyplot as plt

# Create table with the seconds of calls
mydata = []
readfile = open('BTnearme_call.csv', "r")
for line in readfile:
    h,m,s = line.split(':')
    mydata.append(float(h)*3600+float(m)*60+float(s))
mydata = np.array(mydata)
size = mydata.size

# Create an array with correct times starting from first value as 0 point
myarr = []
myarr.append(mydata[0])
for i in range(size-1):
    myarr.append(myarr[i]+0.1)
myarr = np.array(myarr)

# Create differences array
final = (mydata - myarr)

print("Max error: ", max(final))
print("Min error: ", min(final))
print("Mean error: ", sum(final)/len(final))
n = len(final)
mean_ = sum(final) / n
var_ = sum((item - mean_)**2 for item in final) / (n - 1)

print("Variance: ", var_)
print("Standard deviation: ", var_ ** 0.5)

plt.plot(final)
plt.ylabel("Error in seconds")
plt.xlabel("Calls of BTnearMe function")
plt.show()