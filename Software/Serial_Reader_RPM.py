import serial
from datetime import datetime
import csv
import matplotlib
matplotlib.use("tkAgg")
import matplotlib.pyplot as plt
import numpy as np
import json 

arduino_port = "COM3"
baud = 500000



abort = False
ser = serial.Serial(arduino_port, baud)
ser.flushInput()

data = {}

ser_bytes = ser.readline()
decoded_bytes_str = ser_bytes[0:len(ser_bytes)-2].decode("utf-8")
key,values = decoded_bytes_str.split(":")
data["Fs"] = float(values)

ser_bytes = ser.readline()
decoded_bytes_str = ser_bytes[0:len(ser_bytes)-2].decode("utf-8")
key,values = decoded_bytes_str.split(":")
data["Ganancias"] = values

data["X"] = []
data["Y"] = []
data["Z"] = []
data["RPM"] = []



plot_window = 2000
x_var = np.array(np.zeros([plot_window]))
y_var = np.array(np.zeros([plot_window]))
z_var = np.array(np.zeros([plot_window]))
rpm_var = np.array(np.zeros([plot_window]))

plt.ion()
fig, ax = plt.subplots()
linex, = ax.plot(x_var)
liney, = ax.plot(y_var)
linez, = ax.plot(z_var)
linerpm, = ax.plot(rpm_var)


while True:
#for _ in range(3*30):
    try:
        ser_bytes = ser.readline()
        try:
            decoded_bytes_str = ser_bytes[0:len(ser_bytes)-2].decode("utf-8")
            var,values,dim,checksum = decoded_bytes_str.split(":")
            values = list(values.split(","))
            
            # Verif
            if len(values) == int(dim):
                for i in range(len(values)):
                    values[i] = int(values[i])
                if sum(values) == int(checksum):
                    #Todo OK
                    if var == "RPM":
                        values[0] = values[1]
                    data[var] += values
                else:
                    print("Error CHECKSUM: {} vs {}".format(sum(values), int(checksum)))
            else:
                print("Error DIMENSION: {} vs {}".format(len(values), dim))
                    
        except KeyboardInterrupt:
            print("Keyboard Interrupt 1")
            abort = True
            
        except Exception as Error:
            print(str(Error))
            print(decoded_bytes_str)
            continue
        
        if var == "X":
            x_var = np.append(x_var,values)
            x_var = x_var[int(dim):plot_window+int(dim)]
            linex.set_ydata(x_var)
            
        elif var == "Y":
            y_var = np.append(y_var,values)
            y_var = y_var[int(dim):plot_window+int(dim)]
            liney.set_ydata(y_var)
            
        elif var == "Z":
            z_var = np.append(z_var,values)
            z_var = z_var[int(dim):plot_window+int(dim)]
            linez.set_ydata(z_var)
            print(".", end="")
            
        elif var == "RPM":
            rpm_var = np.append(rpm_var,values)
            rpm_var = rpm_var[int(dim):plot_window+int(dim)]
            linerpm.set_ydata(rpm_var)
            print(".", end="")
            if abort:
                print("Finalizando adquisición...")
                break
            
        ax.relim()
        ax.autoscale_view()
        fig.canvas.draw()
        fig.canvas.flush_events()
        
    except KeyboardInterrupt:
        print("Keyboard Interrupt 2")
        abort = True
        
    except Exception as Error:
        print("Keyboard Interrupt")
        print(str(Error))
        break
ser.close()

if abort:
    print()
    s = input("Guardar datos? S/N")
    if s.lower() != "n":
        name = "{}_v1.json".format(datetime.now().strftime("%d_%m_%Y_%H-%M-%S"))
        out_file = open(name, "w") 
        json.dump(data, out_file)
        out_file.close()
        print("Datos guardados como '{}'".format(name))
    else:
        print("Descartando datos...")