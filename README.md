# ModemInfo <img src="https://github.com/Kodo-kakaku/ModemInfo/blob/main/image/logo.png" align='right' width="30%">
**ModemInfo** designed for 
<a href="https://github.com/koshev-msk/modemfeed/tree/master/luci/applications/luci-app-modeminfo">luci-app-modeminfo</a>
backend. Goal of the project is to provide a user-friendly interface for receiving information from 3G/LTE dongle.

## Features
The main task of the server part is to request data about the equipment and the modem signal. The problem with using modem is the lack of any standardization of commands for receiving data from the modem (each vendor uses its own commands). Now it is implemented to receive data from the modem using the serial port via AT commands or using the [QMI](https://en.wikipedia.org/wiki/Qualcomm_MSM_Interface) interface. The response is returned in a [JSON](https://www.json.org/json-en.html) format that can be studied, parsed with a script or sent to the user interface, now all information about the modem is available to you!

## How to use
- ### Qmi mode
To work in this mode, you must specify the name of the mode and the port. This is enough to get data from the modem!
```
For example:
./ModemInfo --qmi -d /dev/cdc-wdm0
{"imei":"xxxxx","manufacturer":"Quectel","model":"EP06-E","firmware":"EP06ELAR04A04M4G","iccid":"-","reg":"registered", ...etc}
```
- ### AT mode
It 's a little more complicated here ... as I have already written, due to compatibility issues with different vendors, there is no convenient scalable solution. This solution does not solve these problems, although it is a more flexible solution. So, to work with your modem, we need to create a JSON file with commands for this modem (there is an example file in the examples), this will allow us to analyze this file and execute these commands, the result of execution will be JSON, which will put the answers to the places of requests, everything is simple!

**Important:** 
- it is not possible to remove or add fields now (it may appear in new versions) 
If you don't want to use a field like **{ "chiptemp": "in+QTEMP"}** just make it empty, like this **{"chiptemp": ""}**. 
- Each field can work with an array. For example, it happens that you need to execute some command before you get the data that you need, then you can use the following format **{"cops": ["In + COPS =3,2","In + COPS?"]}**. 
- And of course, if you want to receive any other data, add them to the "any" field, for example:
**{"any": ["AT+QENG=\"servingcell\"", "AT+QCAINFO", "YOUR COMMAND"]}**.

After the file is ready, you will be able to execute the request!)
```
For example:
./ModemInfo --at -d /dev/ttyUSB2 -f /usr/share/quectel.json
{"device":["Quectel","EP06"],"imei":"xxxxx","imsi":"xxxxx","iccid":"+ICCID: xxxx","firmware":"EP06ELAR04A04M4G","chiptemp":"+QTEMP: 31,31,30", ...etc}
```

## Other Features
- You can request data about hardware or network separately!
```
For example:
./ModemInfo --network --qmi -d /dev/cdc-wdm0
./ModemInfo --hardware --at -d /dev/ttyUSB2 -f /www/quectel.json

HARDWARE -> { "device", "imei", "imsi", "iccid", "firmware", "chiptemp" }
NETWORK  -> { "csq", "cops", "creg", "cereg", "cgreg", "any" }
```
- And of course it's pretty to print!
```
For example:
./ModemInfo --pretty --network --qmi -d /dev/cdc-wdm0
./ModemInfo -p --hardware --at -d /dev/ttyUSB2 -f /www/quectel.json
```
