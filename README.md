![GitHub release (latest by date)](https://img.shields.io/github/v/release/fulda1/SUSI2)
![GitHub Release Date](https://img.shields.io/github/release-date/fulda1/SUSI2?color=blue&label=last%20release)<br/>
![GitHub commits since latest release (by date)](https://img.shields.io/github/commits-since/fulda1/SUSI2/latest?color=orange)
[![GitHub stars](https://img.shields.io/github/stars/fulda1/SUSI2)](https://github.com/fulda1/SUSI2/stargazers)<br/>
[![arduino-library-badge](https://www.ardu-badge.com/badge/Rcn600.svg)](https://www.ardu-badge.com/Rcn600)
[![License](https://img.shields.io/github/license/fulda1/SUSI2)](#)

# Introduction
This library is heavily inspired by https://github.com/TheFidax/Rcn600/
At minimum trying to keep same interface.
Main change is, that this one is developed for little more actual processor CH32V003. (with theoretical upgrade to other CH32V.. family, and some STM32.. family)

# Index
* [Introduction Rcn600 SUSI](#S-9.4.1-Rcn600-SUSI)
* [Video Example](#Video-Example)
* [Library API](#Library-API)
* [Examples of Use](#Examples-of-Use)

------------

# S-9.4.1 Rcn600 SUSI
This library allows you to use an Arduino board with chip CH32V003F4 (or a microcontroller via Arduino IDE) as a Slave for the SUSI interface.<br/>
Library tested on Nano RISC-V CH32V003 (CH32V003F4U6).</br>

To work, you need 2 resistors **470Ω in series** on the SUSI lines (Clock and Data).<br/>
On procesor CH32V003 *Clock* must be connicted to pin PC5 (SPI_SCK) and *Data* must be connected to pin PC6 (SPI_MOSI). Both pins are 5V tolerant (as requested in specification).<br/>
<img src="https://github.com/fulda1/SUSI2/blob/master/wiring.png">

More information about the specification in the file [RCN-600.pdf](https://github.com/TheFidax/Rcn600/blob/master/RCN-600.pdf).

------------

# Video Example
TBD
[![Video Presentazione](https://img.youtube.com/vi/VzgkDouOvCY/0.jpg)](http://www.youtube.com/watch?v=VzgkDouOvCY)

------------

# Library API
The APIs for the library are in the "[readme.md](https://github.com/fulda1/SUSI2/blob/master/src/readme.md)" file available under the folder '[src](https://github.com/fulda1/SUSI2/tree/master/src)'.</br>

------------

# Examples of Use
Under the folder "[examples](https://github.com/fulda1/SUSI2/tree/master/examples)" are available examples of using the library.</br>

------------
