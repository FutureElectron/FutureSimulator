How to Establish connection
============================

The application can commnunicate with any microcontroller via three defined protocols

- Serial 
- Transport Contorl Protocol
- User Datagram Protocol

.. note:: 
    The application saves all configurations made in a session. When you launch without attempting to configure any parameters,
    the previously set values are used to establish connection when the :guilabel:`connect` button is clicked.


Serial Communication
---------------------
Data can be transmitted in full-duplex mode via serial protocol. The MCU must be configured to transmit serial data via USART or USB. In the absence
of this internal peripherals, the user can use an interface converter such as the FTDI converters or opt for the `TCP <>` or `UDP<>` options.
The first step is to confirm the connection parameters of the serial communication of the MCU. It is necessary to note the :guilabel:`baudrate`, :guilabel:`portnumber`,
:guilabel:`parity` :guilabel:`stopbits` and :guilabel:`databits` in order to configure the application.

To configure the application for serial communication, follow these steps

1. Click on the settings button

.. figure:: ./images/settings-click.png
    :width: 90 %
    :alt: Settings button
    :align: center

    Settings button for configuration

2. Click on the dropdown list to select Serial from the list of options

.. figure:: ./images/setttings-parameters.png
    :width: 90 %
    :alt: Installation step 1
    :align: center

    Serial Communication Selection

3. Select connection parameters.   

4. Click the OK button and connect
  
.. figure:: ./images/settings-click-connect.png
    :width: 90 %
    :alt: connect to serial
    :align: center 

    Click on the connect button

5. When connection is established, the parameters of the connection are displayed in the connection status bar  

.. figure:: ./images/connection-statusbar.png
    :width: 90 %
    :alt: Serial Connected UI change
    :align: center 

    Serial Connected UI change

UDP connection
---------------
UDP connection is established in the same way as the TCP connection. Since UDP is a connection-less protocol, only a UDP socket which is bound to the address
and port provided in the configuration as shown below.

1. Select UDP from the dropdown
   
.. figure:: ./images/settings-setudp.png
    :width: 90 %
    :alt: connect to UDP
    :align: center 

    Select UDP Protocol

2. Enter the ipadress or hostname of your PC.

.. figure:: ./images/settings-udp-parameters.png
    :width: 90 %
    :alt: connect to UDP
    :align: center 

    Provide the UDP connection parameters

.. tip:: 
    You can click on the ::guilabel:`use hostname` button to use only the hostname and not the ipadress. Ensure that when using this option, the hostname resolves 
    to a machine (pc) in the same domain as the pc running the application. You can choose to use :guilabel:`localhost` or :guilabel:`127.0.0.1` instead.


.. warning:: 
    Only one option can be used at a time. If the :guilabel:`use hostname` checkbox is `checked`, then the IP address is greyed out. 
    You can only enter valid IP addresses in the ipadress lineedit. If the value entered is invalid, you cannot save the settings.
    A vaild IP address is contains 4 octects with values between 0-255 respectively.

.. figure:: ./images/settings-udp-localhost.png
    :width: 90 %
    :alt: connect to UDP
    :align: center  

    Valid UDP Connection parameters `: localhost`

3. Click Ok to save the settings

4. If previously connected via any of the available protocol, disconnect and reconnect. 
   
.. figure:: ./images/connection-status-udpconnected.png
    :width: 90 %
    :scale: 100 %
    :alt: connect to UDP
    :align: center    

    Connection status when UDP socket is successfully bound to port and address 

.. warning:: 
    To receive UDP datagrams from your PC, your MCU must implement a UDP client listening to a defined port. This application is 
    set to send to :guilabel:`port 1234`, therefore the UDP socket must be bound to this port. 


TCP connection
---------------

To establish TCP connection, follow these steps

1. Select TCP from the dropdown
   
.. figure:: ./images/settings-setudp.png
    :width: 90 %
    :alt: connect to TDP
    :align: center 

    Select TCP from the dropdown

2. Enter the ipadress or hostname of your PC.

.. figure:: ./images/settings-udp-parameters.png
    :width: 90 %
    :alt: connect to TCP
    :align: center 

3. Enter the IP address of the TCP server you would like to connect to 

.. figure:: ./images/settings-tcp-parameters-google.png
    :width: 90 %
    :alt: use google as hostname
    :align: center 

4. Click OK and Connect button

.. figure:: ./images/connection-status-tcpconnected-google.png
    :width: 90 %
    :alt: connect to UDP
    :align: center 

    Connection status when TCP connection is successfully established

.. warning:: 
    ``google.com`` was only used for illustrative purposes. To successfully connect, you have to implement a TCP server on the MCU.


With a valid connnection established, data commands can be sent using the configured media to the microncontroller.