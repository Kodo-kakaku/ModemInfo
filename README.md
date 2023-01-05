# ModemInfo <img src="https://github.com/Kodo-kakaku/ModemInfo/blob/main/image/logo.png" align='right' width="30%">
**ModemInfo** designed for 
<a href="https://github.com/koshev-msk/modemfeed/tree/master/luci/applications/luci-app-modeminfo">luci-app-modeminfo</a>
backend. Goal of the project is to provide a user-friendly interface for receiving information from 3G/LTE dongle.

## Features
The main task of the backend is to request data about hardware and modem signal,
the result is returned in JSON format. Now it is implemented to receive data from modem 
using serial port AT commands or using the QMI interface.
