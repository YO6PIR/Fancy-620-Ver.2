/*########################################################################    
#       ___    ___  _______    ______    _______   ___ _______           #
#       \   \ /  / /   __   \ /   ___)  |    __  \|   |    __  \         #
#        \   /  / |   |  |   |   |____  |   |__)  |   |   |__)  |        #
#         \    /  |   |  |   |    ___  \|    ____/|   |       _/         # 
#         |   |   |   |__|   |   (___)  |   |     |   |   |\   \         #
#         |___|    \_______ / \________/|___|     |___|___| \___\        # 
#                                                                        #   
##########################################################################    
           
 *             Transceiver controller FANCY-620 Ver.6 31.12.2025
			Cu indicator analogic S-meter
		
 *                  https://www.qsl.net/yo6pir/fancy620
 *    Processor:    STM32F103CBT6, 64KB Flash, 20KB RAM, EEprom 24C02
 *    VFO-DDS:      Si5351, VFO, BFO, IF-SHIFT, RIT
 *    Display:      ILI9341 SPI
 *    LCD Library:  Ucglib.h -HWSPI
 *    MEMORY:       EEProm 24C02 - 2kB RAM; 89% Flash fill
 *    Based on 'JAN2KD 2016.10.19 Multi Band DDS VFO Ver3.1 and JA2GQP-2020
 *    Functionality included:
 *          - 6 benzi de frecventa comutabile...............[Key1] scurt
 *          - Comutare VFO A<->B ...........................[Key1] lung
 *          - Ajustare BFO pe fiecare domeniu ........[Key2] apasata la pornire
 *          - Moduri de lucru LSB, USB si CW ...............[Key2] scurt
 *          - AGC ON/OFF....................................[Key2] lung
 *          - ATT ON/OFF................................... [Key3] lung
 *          - AMPLI +20dB ON/OFF........................... [Key3] scurt
 *          - RIT ON/OFF....................................[Key4]scurt 
 *          - Scala liniara pe centru ecranului ON/OFF..... [Key4]lung 
 *          - LOCK KNOB ....................................[ENC_BUT] lung   
 *          - STEP x10, x100, x1K...........................[ENC_BUT] scurt
 *          - Ajustare XTALL oscilator ............[ENC_BUT] apasat la pornire
 *          - iesire de comutare in cod BCD pe CD4028 decoder
 *          - reglaj RIT din encoder cu scala mica liniara pe colt ecran 
 *          - La schimbare pas, rotunjeste VFO cu valoarea STEP
 *          - Salveaza automat dupa 2s de la schimbare frecventa, mode sau banda
 *          - Bargraf analogic 
 *          - Mesaje de interfa in partea de jos a ecranului pe ultima linie 
 *          - Tensiune de alimentare monitorizata pe ecran Ualim < 19V
 *          - Masurare de temperatura Celsius cu Thermistor de 10K
 *          - Pe emisie CW se genereaza ton de 700Hz + FREQ_VFO          
 *          - VFO = CLK0, BFO = CLK1
 ****************************************************************************/
* 		Transceiver controller FANCY-620 Ver6 06.06.2025 
* 	https://www.qsl.net/yo6pir/fancy620 
* 	Processor: STM32F103CBT6, 64KB Flash, 20KB RAM, EEprom 24C02 
* 	VFO-DDS: Si5351, VFO, BFO, IF-SHIFT, RIT 
* 	Display: ILI9341 SPI 
* 	LCD Library: Ucglib.h -HWSPI 
* 	MEMORY: EEProm 24C02 - 2kB RAM; 89% Flash fill
* 	Based on 'JAN2KD 2016.10.19 Multi Band DDS VFO Ver3.1 and JA2GQP-2020
* Functionality included:
* - 6 switchable frequency bands.........................[Key1] short
* - VFO A<->B switching .................................[Key1] long
* - BFO adjustment on each range ........................[Key2] pressed at power-on
* - LSB, USB and CW working modes .......................[Key2] short
* - AGC ON/OFF...........................................[Key2] long
* - ATT ON/OFF.......................................... [Key3] long
* - AMPLI +20dB ON/OFF.................................. [Key3] short
* - RIT ON/OFF...........................................[Key4]short
* - Linear scale on the center of the screen ON/OFF..... [Key4]long
* - LOCK KNOB ...........................................[ENC_BUT] long
* - STEP x10, x100, x1K..................................[ENC_BUT] short
* - XTALL oscillator adjustment .........................[ENC_BUT] pressed at startup
* - BCD code switching output on CD4028 decoder
* - RIT adjustment from encoder with small linear scale on screen corner
* - When changing step, rounds VFO with STEP value
* - Automatically saves after 2s after changing frequency, mode or band
* - Analog bargraph
* - Interface messages at the bottom of the screen on the last line
* - Power supply voltage monitored on the screen Ualim < 19V
* - Celsius temperature measurement with 10K Thermistor
* - On CW transmission, 700Hz tone + FREQ_VFO is generated
* - VFO = CLK0, BFO = CLK1
*****************************************************************************/