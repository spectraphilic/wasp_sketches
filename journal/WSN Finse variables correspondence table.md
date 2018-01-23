# WSN Finse variables correspondence table

Created January 23, 2018



| Sensor tag     | Variable                                 | Unit      | Sensor Reference                         |
| -------------- | ---------------------------------------- | --------- | ---------------------------------------- |
| bat            | battery voltage level                    | %         | if values are close to 100% then it is a lithium battery, if value close to zero then it is a lead acid battery |
| ctd_cond       | Water conductivity                       | dS/m      | http://manuals.decagon.com/Manuals/13869_CTD_Web.pdf |
| ctd_depth      | Water depth                              | cm        | http://manuals.decagon.com/Manuals/13869_CTD_Web.pdf |
| ctd_temp       | Water temperature                        | deg C     | http://manuals.decagon.com/Manuals/13869_CTD_Web.pdf |
| ds1820         | air temperature                          | deg C     | https://datasheets.maximintegrated.com/en/ds/DS18B20.pdf |
| ds2_dir        | wind direction                           | deg North | http://library.metergroup.com/Manuals/14586_DS2_Web.pdf<br /><br />http://manuals.decagon.com/Integration%20Guides/DS-2%20Integrators%20Guide.pdf |
| ds2_gust       | wind gust speed                          | m/s       | http://library.metergroup.com/Manuals/14586_DS2_Web.pdf |
| ds2_meridional | Average U wind speed                     | m/s       | http://library.metergroup.com/Manuals/14586_DS2_Web.pdf |
| ds2_speed      | wind speed                               | m/s       | http://library.metergroup.com/Manuals/14586_DS2_Web.pdf |
| ds2_temp       | sonic temperature                        | deg C     | http://library.metergroup.com/Manuals/14586_DS2_Web.pdf |
| ds2_zonal      | Average V wind speed                     | m/s       | http://library.metergroup.com/Manuals/14586_DS2_Web.pdf |
| mb_median      | sonic ranger median distance (5 samples) | cm        | https://www.maxbotix.com/documents/HRXL-MaxSonar-WR_Datasheet.pdf |
| mb_sd          | sonic ranger  standard deviation (5 samples) | cm        | https://www.maxbotix.com/documents/HRXL-MaxSonar-WR_Datasheet.pdf |
| rssi           | xbee signal receiving strength. Signal emitted by the original xbee  (18dBm ??) | dBm       | http://www.libelium.com/downloads/documentation/waspmote-digimesh-networking_guide.pdf |

