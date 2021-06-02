# arduino_computer_monitor
Monitor a computer by connecting an Arduino to the motherboard.

My goal here was to see what I can monitor without installing any software on the computer, but just using connections to the motherboard.

More specifically, my program monitors:
* CPU Fan requested speed in % of maximum speed
* CPU Fan actual speed in RPM
* HDD usage % (+ displays a filled square that mimicks the HDD LED)
* Temperature (not connected to the motherboard, just uses a temperature sensor)

Overview during development (outside computer):
![General overview](/images/overview.jpg?raw=true)

Final result:
![Final result in computer](/images/overview_final.jpg?raw=true)

Circuit schematic:
![Circuit schematic](/images/schematic.jpg?raw=true)
I used an Arduino Leonardo, but I don't use any Leonardo-specific features so an Arduino Uno could be used instead for example.
