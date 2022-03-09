# Reading my Vitocal

In my home we get our heat by a "Viessman Vitocal-222" which comes with an optical interface that can be used to retrieve data from the sensors that are built into it.

Of course there are commercial ready-to-go systems available that you can buy from Viessmann but since I just started fiddling around with IoT devices it seemed to be the perfect goal to build something from scratch ...

... and fortunatly not quite from scratch: there's a ton of resources, documentation, ideas & software from people who did all the heavy lifting before. So if someone is interested in this topic I highly recommend to browse through the resources at [openV](https://github.com/openv/openv) for all bloody details.

If someone looks for a ready to use library to connect an esp32 or esp8266 to Viessmann boilers I recommend the [VitoWiFi](https://github.com/bertmelis/VitoWiFi) project - it is extensible and comes with a ready to use [platform.io module](https://registry.platformio.org/libraries/bertmelis/VitoWiFi) - super easy to setup.


Inspired by all this I decided to roll my own code just to ramp up my C++ again and to better understand how it actually works. So this implementation is much simpler, only cares about the "KW" protocol and is probably also easier to comprehend just in case someone needs to...

