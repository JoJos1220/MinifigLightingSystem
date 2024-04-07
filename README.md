<a name="readme-top"></a>

# MinifigLightingSystem

<p align="center">
  <img src="https://github.com/JoJos1220/MinifigLightingSystem_private/assets/97045955/dce00213-b5e8-45a6-9424-da8050b2ed47" />
</p>

**Figure 1: Overview of the full assembly with Lightning Animation**

[![Contributors][contributors-shield]][contributors-url]
[![Release Version][realease-shield]][release-url]
[![Release Date][releasedate-shield]][releasedate-url]
[![Last commit][lastcommit-shield]][lastcommit-url]

## About this Project and Repository
Welcome to the source code repository for MinifigLighting, built on ESP8266 for the dynamic control of WS2812b LEDs arranged in a sequential configuration.

For a comprehensive assembly guide and additional project details, please efer to the <a href="#contact">Contact</a> section, where you'll find project links leading to detaild instructions on Instructables.

### Features

**LED Control:** The code allows to connect and control up to 255 WS2812b LEDs in serial, allwoing for a flexicble and scalable setup

**Group Assignment:** Assigning LEDs to differnet groups allows you you to create distinct lighting patterns across your connected LEDs.

**Web Server Integration:** Control your LED arrangement by a Webserver over your lokal WiFi.

<!-- TABLE OF CONTENTS -->
<details>
  <summary>Table of Contents</summary>
  <ol>
    <li><a href="#about-this-project-and-repository">About this Project/Repository</a></li>
    <li><a href="#getting-started">Getting Started</a></li>
    <li><a href="#roadmap">Roadmap</a></li>
    <li><a href="#contributing">Contributing</a></li>
    <li><a href="#license">License</a></li>
    <li><a href="#contact">Contact</a></li>
    <li><a href="#acknowledgments">Acknowledgments</a></li>
    <li><a href="#support">Support</a></li>
  </ol>
</details>

Happy tinkering and illuminating! 🚀🛠️💡

## Getting Started

Before you begin, make sure you have the following components (or similar ones) available.

To simplify the Hardware setup, use a ESP8266 ESP01 WS2812 RGB LED Controller Module from your favorit Online Market. Furthermore you will need a 5V Power Supply to the LED Controller module, several 5mm WS2812b LEDs (which are actually perfect fitting into the holes of the 3D-printing part), Condensator's for avoiding power supply issues between the strand, some wires and connectors, soldering iron and some hotglue for gluing the LEDs in place. If you print my suggested Electronic Case you will also need some heat inserts.

![HardwareParts](https://github.com/JoJos1220/MinifigLightingSystem/assets/97045955/39c5879d-b636-4aba-aa08-4fa3bfb26973)

**Figure 2: Minimum Hardware Setup -> ESP8266-01, RGB LED Controller Module, WS2812b 5mm LED and some connector cables**
_____________________________________________________________________________________________________________
### Preparing 3D-printed parts

The STL File of the shelfs to 3D print can be downloaded on Cults3d.com:

Link to Cults3D: https://cults3d.com/:953606

The STL File of the Electronic Case can be downloaded on Thingiverse:

Link to Thingiverse: https://www.thingiverse.com/thing:6381830

_____________________________________________________________________________________________________________

After prepearing the parts, solder the LEDs together in Serial. Choose the wire length in such way, that the LEDs can be easily put into the 3D Printed parts (In this Project, the Wires are app. ~220mm long). You can add up to 6 LEDs into one shelf piece. It is on you, if you want to add some additional connectors for extend the LEDs or you want to group them.

![HardwareSchematics](https://github.com/JoJos1220/MinifigLightingSystem/assets/97045955/bfcbb0c5-422d-40fb-9bbc-bfb07db1e2a2)

**Figure 3: Hardware Schematics**

Holding the PCB of the RGB LED Controller and the USB PCB Interface in place, i customized a closing which can be found on Thingiverse. Use some hot glue to secure the parts within the closing.

<p align="right">(<a href="#readme-top">back to top</a>)</p>

_____________________________________________________________________________________________________________

# Comissioning
Bevor you flash the Software on the ESP8266, you can (but not must) already pre-define your WiFi credentials within "MiniFigLightning.h"
```
// Define Static Variables for Wifi Access in Station Mode
// Edit them directly here for correct Init
#define STASSID "THIS IS YOUR PERSONAL WiFI SSID"
#define STAPSK  "THIS IS YOUR PERSONAL WIFI PASSWORD" // Be sure it is longer then 8 chars.
```
If you now turn the ESP8266 on, it should connect to defined WiFi AP or, if e.g. credentials are false or WiFi is not defined the ESP8266 start-up in Accespoint mode.
You can now connect to it by e.g. Smarthone or Laptop and selecting the *MiniFigLightning* SSID and the *AdminFigure_23* Passwort. Within the Captive-Portal which you where prompt redirected, you can select your WiFi crendtials again!

If captive Portal is not starting navigate to *http://MiniFigLightningSystem.local*

Make also sure, you have setup your used WS2812b LEDs correctly within the Animation.h section. You may have to modify the *typdef NeoRgbFeature* and *NeoEsp8266Uart1800KbpsMethod* section to your specific LEDs! The typdef of your specific LED can be found wihtin the [NeoPixelBus](https://github.com/Makuna/NeoPixelBus) Libary.
```
// NeoPixelBusLg Constant used LED Definition
typedef NeoRgbFeature ConstNeoGrbFeature;
typedef NeoEsp8266Uart1800KbpsMethod ConstNeoEsp8266Uart1Ws2812xMethod;
```

![Webserver_AP_Overview](https://github.com/JoJos1220/MinifigLightingSystem/assets/97045955/ca8783ef-c55e-4336-8384-b5a1b99a2392)

**Figure 4: ESP8266 WiFi in AP-Mode html request**

If the Setup of WiFi is done, the Parameter Homepage can be accessed by the same url within Station mode. Now, you can setup your personal prefered Animation, Change Brigthness, Assign LEDs to groups or the animation speed! Enjoy!

![Webserver_STA_Overview](https://github.com/JoJos1220/MinifigLightingSystem/assets/97045955/d98d7f13-f0e8-4f01-8fa8-c510aa57189c)

**Figure 5: ESP8266 WiFi in STA-Mode html request**

## Roadmap

With the first release of this project a fully functional prototyp is presented to the open-source community.

Further features are implemented based on the community input and the roadmap will continously maintained!

- adding more Animations

- Implementation of a Visual Audio Display

<p align="right">(<a href="#readme-top">back to top</a>)</p>

## Contributing

Everybody is welcome to contribute the project - regardless of the experience level!

1) fork the repository
2) clone your fork on your PC
3) create a branch for your changes
4) add you changes
5) commit and push
6) create a pull request

<p align="right">(<a href="#readme-top">back to top</a>)</p>

## License

Creative Commons Attribution Share Alike 4.0

[![License: CC BY-NC-ND 4.0](https://img.shields.io/badge/License-CC_BY--NC--ND_4.0-lightgrey.svg)](https://creativecommons.org/licenses/by-nc-nd/4.0/)

See also [LICENSE](LICENSE.md) for more information about license rights and limitations.

<p align="right">(<a href="#readme-top">back to top</a>)</p>

## Contact

Project Link to [Github](https://github.com/JoJo1220/MinifigLightingSystem)

Project Link to [Instructables](https://www.instructables.com/preview/EEYG3BQLDFYRULW/])

Projekt Link to [Cults3D](https://cults3d.com/de/modell-3d/haus/minifigures-compatible-shelf)

<p align="right">(<a href="#readme-top">back to top</a>)</p>

## Acknowledgments
For more detailed information about the Project, please check the Instructables Link given within the Contact section.

You can also check:
 * [WS2812b Tutorial](https://randomnerdtutorials.com/guide-for-ws2812b-addressable-rgb-led-strip-with-arduino/)
 * [Programming Guidline ESP8266-ESP-01](https://elektro.turanis.de/html/prj299/index.html)

## Support

You Like the Project and want to Support me and my work?

Well, I like coffee ;) Maybe we got a deal?

[![ko-fi](https://ko-fi.com/img/githubbutton_sm.svg)](https://ko-fi.com/G2G3OAILE)

[![Donate with PayPal](https://raw.githubusercontent.com/stefan-niedermann/paypal-donate-button/master/paypal-donate-button.png)](https://www.paypal.com/donate/?hosted_button_id=8CTAKMUENCF46)

![MinifigShelfs_connected](https://github.com/JoJos1220/MinifigLightingSystem_private/assets/97045955/6d02b5ce-e36f-4fb9-9e79-e40e2a683608)

**Figure 6: Connected Shelfs**

<p align="right">(<a href="#readme-top">back to top</a>)</p>


<!-- MARKDOWN LINKS & IMAGES -->
[contributors-shield]: https://img.shields.io/github/contributors/JoJo1220/MinifigLightingSystem
[contributors-url]: https://github.com/JoJo1220/MinifigLightingSystem/graphs/contributors
[realease-shield]: https://img.shields.io/github/release/JoJos1220/MinifigLightingSystem.svg?style=plastic
[release-url]: https://github.com/JoJos1220/MinifigLightingSystem/releases/latest
[releasedate-shield]: https://img.shields.io/github/release-date/JoJos1220/MinifigLightingSystem.svg?style=plastic
[releasedate-url]: https://github.com/JoJos1220/MinifigLightingSystem/releases/latest/
[lastcommit-shield]: https://img.shields.io/github/last-commit/JoJos1220/MinifigLightingSystem?style=plastic
[lastcommit-url]: https://github.com/JoJos1220/MinifigLightingSystem/tree
