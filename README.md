# Arduino Seabird SBE3 SBE4 

The project contains C/C++ code for Ardiuno Uno and a shield (Eagle project) for SBE-3 and SBE-4 

# SBE 3
Intended primarily for use on moored and fixed-site temperature measuring systems, the SBE 3S can also be used as a component in custom systems or for high-accuracy industrial and environmental temperature monitoring applications. The low noise characteristics of the SBE 3S allow the use of hybrid frequency measuring techniques to obtain rapid sampling with very high resolution; 40 μ°C resolution can be readily obtained at a 6 Hz sampling rate.

CALIBRATION EQUATION

The calibration yields four coefficients (g, h, i, j) that are used in the following equation (Bennett, 1972):

T [°C] = [1 / (g + hln(fo/f) + iln²(fo/f) + jln³(fo/f) )] - 273.15

where T is temperature [°C], ln is natural log function, and f is SBE 3S output frequency [Hz]. Note that fo, an arbitrary scaling term used for purposes of computational efficiency, was historically chosen as the lowest sensor frequency generated during calibration. For calibration results expressed in terms of ITS-90 temperatures, fo is set to 1000. Calibration fit residuals are typically less than 0.0001 °C.

Example Calibration Data (sensor serial number 2213, 30 May 1996):
g = 4.28793855e-03     h = 6.25807786e-04     i = 2.19368239e-05     j = 1.84262924e-06     f0 = 1000.000



# SBE 4
The SBE 4 conductivity sensor is a modular, self-contained instrument that measures conductivity from 0 to 7 S/m, covering the full range of lake and oceanic applications. The sensor has electrically isolated power circuits and optically coupled outputs to eliminate any possibility of noise and corrosion caused by ground loops. Interfacing is also simplified by the square-wave variable frequency output signal (nominally 2.5 to 7.5 kHz, corresponding to 0 to 7 S/m). The sensor offers improved temperature compensation, smaller fit residuals, and faster turn-on stabilization times.

Because of the SBE 4’s low noise characteristics, hybrid frequency measuring techniques (used in Sea-Bird’s CTD instruments) provide rapid sampling with very high temporal and spatial resolution.

CALIBRATION EQUATION

A least-squares fitting technique (including a zero conductivity point in air) yields calibration coefficients for use in the following equation:

Conductivity [S/m] = ( g + hf 2 + if 3+ jf 4 ) / 10 (1 + dt + ep)

where f is instrument frequency [kHz], t is temperature [°C], p is pressure [decibars], and d is thermal coefficient of expansion (3.25 x 10 -06) and e is bulk compressibility (-9.57 x 10 -08) of the borosilicate cell. The resulting coefficients g, h, i, and j are listed on the calibration certificate. Residuals are typically less than 0.0002 S/m.

Example Calibration Data (sensor serial number 2020, 30 May 1997)

Practical Salinity Scale 1978: C(35,15,0) = 4.2914 [S/m]

g = -1.05697877e+01    h = 1.42707291e+00    i = -4.32023820e-03    j = 4.53455585e-04


