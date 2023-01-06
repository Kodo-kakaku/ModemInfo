# ModemInfo <img src="https://github.com/Kodo-kakaku/ModemInfo/blob/main/image/logo.png" align='right' width="30%">
**ModemInfo** designed for 
<a href="https://github.com/koshev-msk/modemfeed/tree/master/luci/applications/luci-app-modeminfo">luci-app-modeminfo</a>
backend. Goal of the project is to provide a user-friendly interface for receiving information from 3G/LTE dongle.

## Features
The main task of the server part is to request data about the equipment and the modem signal. The problem with using modem is the lack of any standardization of commands for receiving data from the modem (each vendor uses its own commands). Now it is implemented to receive data from the modem using the serial port via AT commands or using the [QMI](https://en.wikipedia.org/wiki/Qualcomm_MSM_Interface) interface. The response is returned in a [JSON](https://www.json.org/json-en.html) format that can be studied, parsed with a script or sent to the user interface, now all information about the modem is available to you!

## How to use
### Qmi mode
To work in this mode, you must specify the name of the mode and the port. This is enough to get data from the modem!
```
For example:
./ModemInfo --qmi -d /dev/cdc-wdm0
{"imei":"0000000000000000","manufacturer":"Quectel","model":"EP06-E","firmware":"EP06ELAR04A04M4G","iccid":"-","reg":"registered","cops":"Your operator","cops_mcc":250,"cops_mnc":2,"cid":"bc8e105","cid_num":197714181,"cell":"05","enbid":"bc8e1","lac":"fffe","lac_num":65534,"mode":"LTE","rssi":-72,"csq_per":100,"rsrq":-11,"rsrp":-104,"sinr":3,"pci":224,"distance":"-","latitude":"-","longitude":"-","arfcn":[1602],"bwdl":5,"scc":["eutran-7"],"bwca":40,"lteca":1}
```
