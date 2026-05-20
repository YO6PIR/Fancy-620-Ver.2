#ifndef MAIN_H
#define MAIN_H

#include <si5351.h>
#include <Rotary.h>                 
#include <at24c02.h>

#include <Fonts/TomThumb.h>
#include <Fonts/FreeSerifBold12pt7b.h>
#include <Fonts/FreeMonoBold12pt7b.h>
#include <Fonts/FreeMonoBold9pt7b.h>
#include <Fonts/FreeMono9pt7b.h>
#include <Fonts/FreeSans9pt7b.h>

#include "font24.h"
#include "Smeter_bitmap.h"        // array RLE generat pentru indicator analogic

//Culori ILI9341 si UI
#define NAVY        ILI9341_NAVY
#define BLUE        ILI9341_BLUE
#define CYAN        ILI9341_CYAN
#define GREEN       ILI9341_GREEN
#define MAGENTA     ILI9341_MAGENTA
#define ORANGE      ILI9341_ORANGE
#define YELLOW      ILI9341_YELLOW
#define RED         ILI9341_RED
#define WHITE       ILI9341_WHITE
#define GRAY        0x8410
#define BLACK       ILI9341_BLACK
// 2. Definiții Culori UI
#define LIGHTGRAY    0xC618
#define MAROON    0x7800
#define DARKGRAY     0x4A49

#define BTN_ON_COLORS { ORANGE, YELLOW, MAGENTA, CYAN }

/*----------   CW Tone  -------------------*/ 
#define   CW_TONE     700                // 700Hz
/*----------   I/O Assign  ------------------*/ 
#define   MODE_OUT1    PB15    //Iesire1 in cod BCD pt MODE                            
#define   MODE_OUT2    PA8     //Iesire2 in cod BCD pt MODE                        
#define   BAND_OUT1    PB3     //
#define   BAND_OUT2    PB4     //--Iesiri in cod BCD pt BANDS(CD4028 Decoder) 
#define   BAND_OUT3    PB5     //
#define   SW_BAND      PA0     //Buton de schimbare benzi            
#define   SW_MODE      PC14    //Buton de schimbare MODE[LSB,USB,CW,AM]             
#define   SW_STEP      PB14    //Buton Encoder             
#define   SW_RIT       PC15    //Buton RIT             
#define   SW_TX        PC13    //INPUT PTT sense             
#define   SMETER       PA1     //INPUT S-meter analog 
#define   VOLT         PA3      //INPUT Voltage Battery sense
#define   TEMP         PA2      //INPUT Temperature Thermistor 10k 
#define   SW_AMP       PA4      //Buton AMP/ATT
#define   AMP_OUT      PA10     //OUTPUT AMP signal[ON/OFF]
#define   ATT_OUT      PA9     //OUTPUT ATT signal [ON/OFF]
#define   AGC_OUT      PA15     //OUTPUT AGC signal[ON/OFF]                
#define   XTAL_FREQ    25000000     // Default Crystal frequency 27MHz

#define SHORT_PRESS_THRESHOLD   300                 
#define MAXBANDS                6                   //Nr maxim de benzi 

/*************  Definitii Senzor Temperatura    */
#define BETA    3950        //coeficientul Beta al termistorului
#define R_NOMINAL 10020      //rezistenta in Ohmi la 25C
#define ROOM_TEMP 298.15    //Temperatura la 25K
#define R_BALANCE 9940      //rezistenta divizorului serie de tensiune
#define ADC_MAX   4096.0    //Val Max ADC

#define SCREEN_W 320
#define BTN_H    25
#define BTN_R    3
#define BTN_GAP  5

// Definim canvas-ul global pentru a refolosi memoria
// Declari un singur Canvas mare, global (MAXIM 16 KB pentru siguranță)
// 80 x 80 pixeli = 12,800 bytes
GFXcanvas16 sharedCanvas(80, 80);

#endif
