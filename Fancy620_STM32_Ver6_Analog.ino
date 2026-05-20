/*######################################################################    
#       ___      ___ _______    ______   _______   ___ ________        #
#       \   \  /   /   __   \ /   ____) |    __  \|   |    __  \       #
#        \   /   /|   |  |   |   |____  |   |__)  |   |   |__)  |      #
#         \    /  |   |  |   |    ___  \|    ____/|   |       _/       # 
#         |   |   |   |__|   |   (___)  |   |     |   |   |\   \       #
#         |___|    \_______ / \________/|___|     |___|___| \___\      # 
#                  https://www.qsl.net/yo6pir                          #   
########################################################################               
 *             Transceiver controller FANCY-620 Ver.6_Analog 
 *                    cu indicator S-meter analogic
 *    Data:         01.01.2026                  
 *    Processor:    STM32F103CBT6, 128KB Flash, 20KB RAM, EEprom 24Cxx
 *    VFO-DDS:      Si5351, VFO, BFO, IF-SHIFT, RIT
 *    Display:      ILI9341 SPI
 *     LCD Library:  Adafruit_ILI9341.h - Hardware_SPI
 *    MEMORY:       EEProm 24C02 - 2kB RAM; 89% Flash fill
 *    Cod original inspirat din Snipper3
 *    Functionality included:
 *          - Indicator S-meter Analogic
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
 *          - Mesaje de interfa in partea de jos a ecranului pe ultima linie 
 *          - Tensiune de alimentare monitorizata pe ecran Ualim < 19V
 *          - Masurare de temperatura Celsius cu Thermistor de 10K
 *          - Pe emisie CW se genereaza ton de 700Hz + FREQ_VFO          
 *          - VFO = CLK0, BFO = CLK1
 ****************************************************************************/
#include <Wire.h>                
#include <Adafruit_GFX.h>
#include <Adafruit_ILI9341.h>
#include <SPI.h>
#include "main.h"
/* Create a eprom object configured at address 0
 Sketch assumes that there is an eprom present at this address*/
AT24C02 eprom(AT24C_ADDRESS_0);
/* Create another eprom object configured att address 2
Sketch assumes that there is NO eprom present at this address*/
AT24C02 badEprom(AT24C_ADDRESS_2);
//

uint16_t backcolor = BLACK;

/*----------   Encorder setting  ---------------*/
#define ENC_A     PB12                    // Rotary encoder A
#define ENC_B     PB13                    // Rotary encoder B
Rotary Rot = Rotary(ENC_A,ENC_B);

/*----------   ILI9341 setting  -------------------*/
#define   ILI9341_CS        PB10
#define   ILI9341_DC        PB0
#define   ILI9341_RST       PB1
Adafruit_ILI9341 tft = Adafruit_ILI9341(ILI9341_CS, ILI9341_DC, ILI9341_RST); // Use hardware SPI

//Setari Si5351
Si5351 si5351(0x62);    //adresa 0x60→(OR), sau 0x62→Clone

long ssb_filter_freq;
long bfo_offset;
byte bands_mask = 0x3F;
bool bands_active[6] = {true, true, true, true, true, true};
volatile bool flag_btn = false;
volatile bool flag_up = false;
volatile bool flag_dw = false;

//variabile Fast-encoder
unsigned long last_encoder_time = 0;
int multiplier = 1;

char buf[16];                             // buffer pentru formatul xxx.xxx

byte     Si5351OscPower[] = {0,4,4,4}; // Nivel de iesire Si5351 (1,2,3,4)->(2,4,6,8)mA

/*---------- Variable setting ----------*/
long Flim[MAXBANDS][2] = { {3500000, 3800000}, //Limitele de frecventa ale benzilor
                           {7000000, 7200000}, 
                           {14000000, 14400000}, 
                           {18000000, 18200000}, 
                           {21000000, 21500000},
                           {28000000, 29000000}};
int       cur_vfo = 0;                      //Default 0 = VFO-A; 1 = VFO-B
long      freq[2];                          //cele doua frecvente afisate 
long      romf[4];                          // Buffer de citire Freq din EEprom
long      freqold = 1;
int       freq_ifshift = 0;
int       freq_ifshift_old = 0;
int       freq_rit, freq_rit_old;
String    freq_str = String(freq[cur_vfo]);   //Frequency text
long      vfofreq = 0;
long      freqv=0;
uint64_t  si5351_freq=0;
long      vfofreqb;                 
long      cio = 0;                            //variabila care tine BFO
long      old_cio;                            //variabila temporara BFO
long      romb[5];                            // EEPROM bfo copy buffer
char      f100m,f10m,fmega,f100k,f10k,f1k,f100,f10,f1;
int       fstep = 100;
uint16_t  steprom;                            //Pasul citit din EEprom [int]
uint16_t  fmode;                              //MODE citit din EEprom
uint16_t  scalarom;                           //Flag SCALA citit din EEprom
uint16_t  agcrom;                             //Flag AGC citit din EEprom
bool      init_flag;                          //Flag de pornire PLL
bool      flag_rit=0;
bool      flag_spl=0;
bool      flag_lock;
bool      flag_att = 0;
bool      flag_amp = 0;
bool      flag_agc = 0; 
bool      flag_salveaza = 0;                    // Frequency data Wite Flag(EEPROM)
bool      flag_step = 0;                    //Flag STEP ACT
bool      flag_on_air=0;                      //steagul "ON-AIR"
bool      message = 0;
int       flag_scala;
int       meter_value  = 0;
int       romadd     = 0;                   //adresa mem care tine VFO curent
int       key = 0;
int       adc_v;
uint16_t          band;                   
uint16_t          Status;
uint16_t          Stare;       
uint32_t          xtalFreq;
unsigned long     eep_freq[4];
int               eep_romadd;
int               eep_fstep;
int               eep_fmode;
int               eep_scala;
int               eep_agc;
unsigned long     eep_bfo[6];
int               eep_rombadd;
long              freqb = 0;
bool firstDraw = true;
uint8_t     bitmask=0;    //masca ce reflecta benzile active

// Coordonate de referinta pentru afisare MainFreq1
int xpos = 1;  // stanga
int ypos = 62;  // sus

uint32_t lastEncoderTime = 0;   // ISR-ul encoderului setează acest timp

/*Volts measurement variables*/
double    v1;                               //varriabila care tine tensiunea afisata VOLT

int prev_freq = freq_ifshift;  // Frecvența anterioară
bool start_with_small = false; // Flag pentru alternarea începutului scării
 
int_fast32_t tempo = 0; 
int_fast32_t timepassed;                    // int to hold the arduino miilis since startup
int_fast32_t runseconds10msg = 0;           //cronometru pentru afisare mesaj
int_fast32_t runseconds10volts = -50;       //cronometru afisare Temp/Volt
unsigned long buttonPressStartTime = 0;     // Momentul in care a fost apasata o tasta
bool ket1confirmata = false;                   // Variabila pentru confirmare daca Tasta1 a fost apasata 
bool key2pressed = false;                   // Variabila pentru confirmare daca Tasta2 a fost apasata 
bool key3pressed = false;                   // Variabila pentru confirmare daca Tasta3 a fost apasata 
bool key4pressed = false;                   // Variabila pentru confirmare daca Tasta4 a fost apasata 
bool key5pressed = false;                   // Variabila pentru confirmare daca Tasta5 a fost apasata  

typedef struct {
  const char* label;
  bool*  flag;   // 0 = OFF, 1 = ON
} Button_t;

 Button_t buttons[5] = {
  { "+20dB", &flag_amp },
  { "ATT", &flag_att },
  { "FAST", &flag_agc },
  { "SHIFT", &flag_rit },
  { "SPL", &flag_spl }
};

/******************************************************************************************************
----------------------------------  Initialization  Program  ------------------------------------------
*******************************************************************************************************/ 
void setup() {
// calibrează ADC-ul
  rcc_clk_enable(RCC_ADC1);
  adc_calibrate(ADC1);
  adc_enable(ADC1);

  timepassed = millis();
  afio_cfg_debug_ports(AFIO_DEBUG_NONE);          // ST-LINK(PB3,PB4,PA15,PA12,PA11) Can be used      
  
  Serial1.begin(115200);  // 115200 bps este standard, rapid și stabil
  delay(100);

  Serial1.println("Debug FANCY-620: start"); 
  Wire.begin();
  Wire.setClock(400000);
 
  pinMode( ENC_A,INPUT_PULLUP);                   
  pinMode( ENC_B,INPUT_PULLUP);                   
  /*Activeaza intreruperile pentru pini encoder*/
  attachInterrupt( ENC_A, Rotary_encoder_isr, CHANGE);    // Encorder A
  attachInterrupt( ENC_B, Rotary_encoder_isr, CHANGE);    //          B
  delay(100);

  tft.begin(36000000);
  tft.setRotation(1);
  tft.fillScreen(BLACK);

  pinMode(SW_BAND,INPUT_PULLUP);
  pinMode(SW_MODE,INPUT_PULLUP);
  pinMode(SW_STEP,INPUT_PULLUP);
  pinMode(SW_RIT,INPUT_PULLUP);
  pinMode(SW_TX,INPUT_PULLUP);
  pinMode(ENC_A,INPUT_PULLUP);                   
  pinMode(ENC_B,INPUT_PULLUP);                    
  pinMode(SW_AMP,INPUT_PULLUP); 
  pinMode (BAND_OUT1,OUTPUT);
  pinMode (BAND_OUT2,OUTPUT);
  pinMode (BAND_OUT3,OUTPUT);
  pinMode(MODE_OUT1,OUTPUT);
  pinMode(MODE_OUT2,OUTPUT);
  pinMode(AMP_OUT,OUTPUT);
  pinMode(ATT_OUT,OUTPUT);
  pinMode(AGC_OUT,OUTPUT);

  eepDataInit();      //citeste EEprom si verifica/initializeaza valori Default
  delay(100);
  loadSystemSettings();     //incarca setarile initiale de sistem
  
  si5351.init(SI5351_CRYSTAL_LOAD_8PF, xtalFreq, 0);
  si5351.set_pll(SI5351_PLL_FIXED, SI5351_PLLA);
  si5351.set_pll(SI5351_PLL_FIXED, SI5351_PLLB);
  
  Si5351Strengh();
//---- Intrare in modul de calibrare Sistem Settings -----
  if (get_keys() == 5) { // Tasta SW_STEP apasata
     loopSettings(); // <-- AICI CHEMĂM FUNCȚIA DE SETĂRI
  } 
  init_screen();
} 

//------------------ Fast ADC read ---------------
 uint16_t fastADCRead(uint8_t channel)
{
    ADC1->regs->SQR3 = channel;
    // conversie dummy (discard)
    ADC1->regs->CR2 |= ADC_CR2_ADON;
    while (!(ADC1->regs->SR & ADC_SR_EOC));
    (void)ADC1->regs->DR;
    // conversie valida
    ADC1->regs->CR2 |= ADC_CR2_ADON;
    while (!(ADC1->regs->SR & ADC_SR_EOC));
    return ADC1->regs->DR;
}

/*----------- Citeste intrarea ADC MediatFast ---------------*/
unsigned int citireADCmediatFast(uint8_t channel) {
  const int nrMedii = 8;    //numar de citiri ADCmediat_fast
  uint32_t suma = 0;
  for (int i = 0; i < nrMedii; i++) {
    suma += fastADCRead(channel);
  }
  return (unsigned int)(suma / nrMedii);
}

/******************************************************************************************************
-----------------------------------------  Main program  ----------------------------------------------
*******************************************************************************************************/ 
void loop() {
  citire_pozitie_ac();  //actualizeaza acul indicator pe afisaj  
  
  if (digitalRead(SW_TX)==LOW) {                 // TX sw check
    flag_on_air=1;                        //ridica steagul ON-AIR
    rxtx();             //ruleaza functiile in emisie
  }
  else{   
  key = get_keys();     //citeste tastele + encoder_button  
    key_bands();        //verifica tasta-1 BANDS/VFO
    key_mode();         //verifica tasta-2 MODE/AGC
    key_amp_att();      //verifica tasta-3 AMP/ATT
    key_shift();        //verifica tasta-4 SHIFT/SPLITT
    key_enc_button();   //verifica tasta-5 ENCODER/SETTINGS  
      
  if(flag_rit){                     /*daca este activat IF-Shift*/
    if (freq_ifshift != freq_ifshift_old){      /*Anti Flicker IF-SHIFT*/
      PLL_write();
      show_scara_mica(flag_rit);
    }
  }
  
      if(freq[cur_vfo] != freqold){           /*Anti-Flicker freq[cur_vfo]*/
        PLL_write();
            if(flag_scala) 
              show_scara_mare();              //modifica acul pe Scala Mare
            else 
              show_frequency1(freq[cur_vfo]); //Modifica frecventa afisata pe cifrele mari
       flag_salveaza = 1;                     // ridica steagul "Avem ceva de salvat"
        timepassed = millis();    
      }
  
  /*----------- EEprom Auto-Save in 10 sec -------------------- */
  if(flag_salveaza){                     
    if(timepassed+10000 < millis()){
     bandwrite();
      flag_salveaza = 0;
       show_msg("          Save to memory...", 0);
    } 
  }
  /*  Afiseaza mesaj permanent cu un refresh de 2sec in functie de message */
    if( millis() > (runseconds10msg + 1000) && message)
    {
     show_msg("  Fancy620 HF TRANSCEIVER by YO6PIR", 0);
      runseconds10msg = millis();
     message = 0;
    } 
  }
    /*Refresh la 3sec VOLTS and TEMPERATURE measurement*/
    if(millis() > runseconds10volts + 3000)
    {
      show_voltage();
      show_temperature();
      runseconds10volts = millis();
    }
}

void Rotary_encoder_isr() {
  if (!flag_lock) {
    unsigned char result = Rot.process();
    if (result) {
      // --- LOGICA DE ACCELERARE ---
      unsigned long now = millis();
      unsigned long time_diff = now - last_encoder_time;
      last_encoder_time = now;

      if (time_diff < 35) {       // Rotire foarte rapidă
        multiplier = 10; 
      } else if (time_diff < 80) { // Rotire medie
        multiplier = 5;
      } else {                     // Rotire lentă (normală)
        multiplier = 1;
      }
      // ----------------------------

      if (flag_rit) { 
        if (result == DIR_CW) {
          freq_ifshift += (10 * (multiplier > 1 ? 2 : 1)); // Accelerare ușoară la IF-SHIFT
          if (freq_ifshift >= 300) freq_ifshift = 300;
        } else {
          freq_ifshift -= (10 * (multiplier > 1 ? 2 : 1));
          if (freq_ifshift <= -300) freq_ifshift = -300;
        }
      } 
      else { 
        long actual_step = fstep * multiplier; // Calculăm pasul accelerat

        if (result == DIR_CW) {
          flag_up = true;   //confirma rotirea inainte
          freq[cur_vfo] += actual_step;
          if (freq[cur_vfo] >= Flim[band][1])  freq[cur_vfo] = Flim[band][1];
        } 
        else {
          flag_dw = true;   //confirma rotirea inapoi
          freq[cur_vfo] -= actual_step;
          if (freq[cur_vfo] <= Flim[band][0]) freq[cur_vfo] = Flim[band][0];
        }
        freq[cur_vfo] = rounding(freq[cur_vfo]);
      }
    }
  }
}

/*------- Initializare Ecranul Principal -----------*/
void init_screen(){
  si5351.output_enable(SI5351_CLK0, 1);
  si5351.output_enable(SI5351_CLK1, 1);
  si5351.output_enable(SI5351_CLK2, 0); 

  init_flag = 1;
  show_frequency2(cur_vfo);

  if(flag_scala){
   tft.fillRect(47,119,270,44,backcolor);                      //fundal scara mare
   tft.fillRect(7,119,40,21,backcolor);                        //sterge fundal  "VFO" de pe pozitie 
   show_scara_mare();    
  }
  else {
    show_frequency1(freq[cur_vfo]);
    tft.setTextColor(WHITE);
    tft.setFont(&FreeMono9pt7b); 
    tft.setCursor(8,138);  
    tft.print("VFO"); 
    tft.setCursor(283,157);  
    tft.print("MHz"); 
  }    
  show_agc(flag_agc);
  show_mode();
  switchBands();                                   //seteaza iesirile de banda   
  show_band_index();
  switch_amp(flag_amp);
  switch_att(flag_att);
  show_step();
  show_scara_mica(0);
  show_temperature();
  show_voltage();
  show_msg("  Please Wait! Loading program...  ", 1);
  
  drawRLEBitmap16();    // bitmap inițial cadran
  /* Deseneaza rama indicator analog*/ 
  tft.drawRoundRect(1,1,241,113,5,WHITE);
  tft.drawRoundRect(2,2,239,111,5,GRAY);
  /*deseneaza chenare lateral dreapta*/
  tft.drawRoundRect(243,1,77,113,5,WHITE);
  tft.drawRoundRect(244,2,75,111,5,GRAY);
   //*Deseneaza o rama dubla cu colturi rotunjite pentru Freq1 si Freq2 
                    
  tft.drawRoundRect(1,115,319,82,5,WHITE);
  tft.drawRoundRect(2,116,317,80,5,GRAY);
  //deseneaza o linie intre Freq1 si Freq2
  tft.drawLine(10, 165, 310, 165, GRAY);
  

deseneazaGrupTaste(200, buttons, 4);
}

/* Routine for rounding the adjusted value to the fraction of the selected step */
long rounding(long f){
  double fractia = f / fstep;
  double fractia2 = f % fstep;
      if(fractia2 != 0){ 
        return f = fractia * fstep;  
      }
  return f;
}            
/*************************  Citeste tastele si returneaza indexul lor ************************/
int get_keys(void){
  if(digitalRead(SW_BAND) == LOW)       return 1;
  else if(digitalRead(SW_MODE) == LOW)  return 2;
  else if(digitalRead(SW_AMP) == LOW)   return 3;         
  else if(digitalRead(SW_RIT) == LOW)   return 4;
  else if(digitalRead(SW_STEP) == LOW)  return 5;
  return 0;
}
/*********************************** Tasta[1] BANDS/VFO[A-B] ***************************************/
void key_bands(){
 
  if((key==1)&&(!ket1confirmata)){ //Daca Tasta 1 este apasata dar nu a fost confirmata inca...    
        delay(100);                           //Debounce button     
        if(key==1){                           //Tasta este inca apasata?
        buttonPressStartTime = millis();      // Începem sa masuram timpul cat este apasat cu incrementare de 0,1sec
        ket1confirmata = true;                   //confirma ca tasta 1 a fost apasata  
      }
    }
  if((key==0) && ket1confirmata){//daca nu este apasata nicio tasta dar se confirma Tasta1...    
    
       unsigned long pressDuration = millis() - buttonPressStartTime;  // Calculeaza durata apasarii butonului
                
        if(pressDuration < SHORT_PRESS_THRESHOLD) 
        {/* Daca timpul apasarii este mai mic de 1 secunda, consideram ca a fost o apasare scurta*/
//            band++;
  //          if(band > (MAXBANDS-1))band=0;  

            if (bitmask > 0) { // Executăm doar dacă există cel puțin o bandă activă
              do {
                band++;
                if (band >= MAXBANDS) band = 0;
              } while (!(bitmask & (1 << band)));
            }

            if(flag_rit){
              flag_rit=0;
              freq_ifshift=0;
              show_scara_mica(0);                      
            }
            if(flag_scala) {

            tft.fillRect(47,119,270,44,backcolor);                      //fundal scara mare
            tft.fillRect(7,119,40,21,backcolor);                        //sterge fundal  "VFO" de pe pozitie

               show_scara_mare();                              //Afiseaza Scala Mare                 
             }
        }     
        else {/* Daca timpul apasarii este mai mare de 1 secunda, consideram ca a fost o apasare lunga*/          
            cur_vfo ^= 1;                                                
        }
        /* La orice apasare de tasta scurt/lung, executa.....*/
            romadd = 0x010+(band*0x010);            
            for (int i=0; i<3;i++){
              eprom.get((romadd+4*i),romf[i]);  delay(5);
            }            
            freq[cur_vfo]=romf[cur_vfo];
            freq[!cur_vfo]=romf[!cur_vfo];
            agcrom=eprom.read(romadd+10);            
            fmode=eprom.read(romadd+12);
            steprom=eprom.read(romadd+14);  
            flag_agc=agcrom; show_agc(flag_agc);       
            if(steprom==1)fstep=1000;          
            if(steprom==2)fstep=10;
            if(steprom==3)fstep=100;
            
        if(flag_rit)flag_rit = 0;           //Anuleaza RIT la orice schimbare de banda, daca era activat inainte,                  
        show_mode();
        show_step();    
        show_band_index();
        switchBands();                
        show_frequency2(cur_vfo);

        ket1confirmata = false;                  // Resetam starea butonului
        buttonPressStartTime=0;               //reset contor durata
    } 
}
/*********************Tasta-2 MODE[LSB,USB,CW,AM]/AGC[LOW-FAST] ***************************************/
void key_mode(){
 
  if((key==2) && (!key2pressed))              //Tasta 1 este apasata dar nu a fost confirmata inainte...
    {
      delay(100);                             //Debounce button     
      if(key==2){                             //Tasta este inca apasata?
        buttonPressStartTime = millis();      // Începem sa masuram timpul cat este apasat cu incrementare de 0,1sec
        key2pressed = true;                   //confirma ca tasta 2 a fost apasata  
        }
    }
        if(key==0 && key2pressed)             //daca nu este apasata nicio tasta dar se confirma Tasta1...    
    {
       unsigned long pressDuration = millis() - buttonPressStartTime;  // Calculeaza durata apasarii butonului
      
        if(pressDuration < SHORT_PRESS_THRESHOLD) 
        {/* Daca timpul apasarii este mai mic de 1 secunda, consideram ca a fost o apasare scurta*/
         fmode++;
         if(fmode>2)fmode=0;
         show_mode();
         PLL_write();
        }     
        else {/* Daca timpul apasarii este mai mare de 1 secunda, consideram ca a fost o apasare lunga*/
          flag_agc ^= 1;                           //toggle agc mode

        

          show_agc(flag_agc);   
           flag_salveaza = 1;                                // EEP Wite Flag
           timepassed = millis();                
        }
        key2pressed = false;                  // Resetam starea butonului
        buttonPressStartTime=0;               //reset contor durata
    } 
}
/******************************** Tasta-3 AMP[ON-OFF]/ATT[ON-OFF] **************************************/
void key_amp_att(){
 
  if((key==3) && (!key3pressed))              //Tasta 3 este apasata dar nu a fost confirmata inainte...
    {
      delay(100);                             //Debounce button     
      if(key==3){                             //Tasta este inca apasata?
      buttonPressStartTime = millis();      // Începem sa masuram timpul cat este apasat cu incrementare de 0,1sec
      key3pressed = true;                   //confirma ca tasta 3 a fost apasata  
      }
    }
        if(key==0 && key3pressed)             //daca nu este apasata nicio tasta dar se confirma Tasta1...    
    {
       unsigned long pressDuration = millis() - buttonPressStartTime;  // Calculeaza durata apasarii butonului
      
        if(pressDuration < SHORT_PRESS_THRESHOLD) 
        {/* Daca timpul apasarii este mai mic de 1 secunda, consideram ca a fost o apasare scurta*/
          flag_amp^=1;    //toggle Amp
          switch_amp(flag_amp);
        }     
        else {/* Daca timpul apasarii este mai mare de 1 secunda, consideram ca a fost o apasare lunga*/
         flag_att ^= 1;                      //toggle att  
         switch_att(flag_att);     
        }
        key3pressed = false;                  // Resetam starea butonului
        buttonPressStartTime=0;               //reset contor durata
    } 
}
/****************************** Tasta-4 SHIFT[ON-OFF] **********************************/
void key_shift(){
 
  if((key==4) && (!key4pressed))              //Tasta 4 este apasata dar nu a fost confirmata inainte...
    {
      delay(100);                             //Debounce button     
      if(key==4){                             //Tasta 4 este inca apasata?
        buttonPressStartTime = millis();      // Începem sa masuram timpul cat este apasat cu incrementare de 0,1sec
        key4pressed = true;                   //confirma ca tasta 4 a fost apasata  
      }
    }
       if((key==0) && key4pressed)             //daca nu este apasata nicio tasta dar se confirma Tasta1...    
    {
       unsigned long pressDuration = millis() - buttonPressStartTime;  // Calculeaza durata apasarii butonului
      
        if(pressDuration < SHORT_PRESS_THRESHOLD) 
        {/* Daca timpul apasarii este mai mic de 1 secunda, consideram ca a fost o apasare scurta*/
              if(freq_ifshift==0){
                if(!flag_lock)flag_rit ^=1; 
                show_rit(flag_rit);   
              }
              else {
                freq_ifshift=0;PLL_write();
              }
              show_scara_mica(flag_rit);
        }     
        else {/* Daca timpul apasarii este mai mare de 1 secunda, consideram ca a fost o apasare lunga*/             
             flag_scala ^=1;       
             if(flag_scala) {
             tft.fillRect(47,119,270,44,backcolor);                      //fundal scara mare
             tft.fillRect(7,119,40,21,backcolor);                        //sterge fundal  "VFO" de pe pozitie
             show_scara_mare();                              //Afiseaza Scala Mare  
             }
             else{
              tft.fillRect(47,119,270,44,backcolor);                         //fundal scara mare
              show_frequency1(0);                               //Dump-buffer   
              delay(10);
              tft.fillRect(47,119,270,44,backcolor);                      //fundal scara mare
              show_frequency1(freq[cur_vfo]);
              tft.fillRect(7,119,40,21,backcolor);                         //sterge "VFO" de pe pozitie 
              tft.setFont(&FreeMono9pt7b); 
              tft.setTextColor(WHITE);
              tft.setCursor(8,138);  
              tft.print("VFO");
              tft.setCursor(283,157);  
              tft.print("MHz"); 
            }
            flag_salveaza = 1;                                // EEP Wite Flag
            timepassed = millis();      
        }
        /**  Executa daca oricare mod de apasare este activ SHORT/LONG*************/
        key4pressed = false;                        // Resetam stare tasta 4
        buttonPressStartTime=0;                     //reset contor durata
    } 
}
/*********************************** Tasta-5 ENCODER[STEP]/SETTINGS ***********************************/
void key_enc_button(){
   if((key==5) && (!key5pressed))            //Tasta 5 este apasata dar nu a fost confirmata inainte...
    {
      delay(100);                            //Debounce button     
      if(key==5){                            //Tasta 5 este inca apasata?
        buttonPressStartTime = millis();     // Începem sa masuram timpul cat este apasat cu incrementare de 0,1sec
        key5pressed = true;                  //confirma ca tasta 5 a fost apasata           
        }
    }
    if(key==0 && key5pressed)               //la ridicarea degetului de pe tasta se confirma Tasta-5 activata...    
    {
    unsigned long pressDuration = millis() - buttonPressStartTime;  // Calculeaza durata apasarii butonului
      
        if(pressDuration < SHORT_PRESS_THRESHOLD){
        /* Daca timpul apasarii este mai mic de 1 sec -> apasare scurta*/   
          setstep();                             //ajusteaza pasul encoder 
          flag_btn = true;
          flag_salveaza = 1;                                // salveaza noul pas in memorie
          timepassed = millis();      

        }     
        else {// Daca timpul apasarii este mai mare de 1 sec -> apasare lunga
          if(!flag_rit)flag_lock ^=1;     //toggle Flag lOCK daca nu e RIT activat
          show_lock(flag_lock);     
        }
        key5pressed = false;                        // Resetam stare tasta 5
        buttonPressStartTime = 0;                     //reset contor durata
    } 
}
/*---------- PLL write ---------------------------*/
void PLL_write(){
    //stabilim mai intai variabila CIO
    switch(fmode){
      case 0: /*LSB Mode*/
            cio = ssb_filter_freq - bfo_offset; // LSB
            break;
      case 1: /*USB MOde*/
            cio = ssb_filter_freq + bfo_offset; // LSB
            break;
      case 2: /*CW Mode*/
            cio = ssb_filter_freq +700; // LSB
            break; 
    }
  
    Vfo_out(freq[cur_vfo] + cio );      //seteaza DDS
                  
    Bfo_out(cio + (flag_rit ? freq_ifshift : 0));   //seteaza BFO si aduna SHIFT daca este activat

    if (!flag_rit && !flag_scala) {
        freqold = freq[cur_vfo];    //Actualizeaza frecventa afisata
    }
}
/*---------- VFO  out  ---------------*/ 
void Vfo_out(long frequency){
    freqv=frequency;
    si5351_1(); 
}
/*----------  BFO out  ---------------*/        
void Bfo_out(long freqcio){
  if(freqcio != old_cio){
    //si5351aSetFrequency2(freqcio);
    freqv=freqcio;
    si5351_2(); 
    old_cio = freqcio;  
  }
}
/*------------- Show Temperature --------------------*/
void show_temperature(){
  int tmp = get_pa_temp();
    Label_var(tmp, 250, 35,  " COOLER ", "C", 0);
}
/* Citeste Temperatura de la thermistor 10K */
double get_pa_temp(void){
    int adc = analogRead(TEMP);
    double rThermistor = R_BALANCE * ((ADC_MAX / adc) - 1);
    //*Ecuatia Steinhart-Hart*
    double temperatura_K = (BETA * ROOM_TEMP) / 
    (BETA + (ROOM_TEMP * log(rThermistor / R_NOMINAL)));
    return (int) temperatura_K - 273.15;                //transforma din K in Celsius
} 

/**** Rutina de afisare Voltaj Baterie ********************************/
void show_voltage(){
    float v1 = (float) analogRead(VOLT)* 3.3 / 4092 * 6;  
    Label_var(v1, 250, 8,  " BATT ", "V", 1);
    
}

/**** Rutina de afisare a unei variabile pe o eticheta *****************/
void Label_var(float variable, int xpos, int ypos, const char* label, const char* unit_meas, bool zecimala) {
  const int boxW = 65;
  const int boxH = 21;
  const int labelY = 4;

  sharedCanvas.fillScreen(backcolor);

  sharedCanvas.drawRect(0, labelY, boxW, boxH, GRAY);

  sharedCanvas.setFont(); 
  sharedCanvas.setTextColor(GRAY, backcolor); 
  sharedCanvas.setCursor(5, 0); 
  sharedCanvas.print(label);

  sharedCanvas.setFont(&FreeMono9pt7b);
  sharedCanvas.setTextColor(CYAN);
  sharedCanvas.setCursor(zecimala ? 2 : 10, labelY + boxH - 6);
  
  if (variable < 10.0) sharedCanvas.print(' ');
  sharedCanvas.print(variable, zecimala ? 1 : 0);

  sharedCanvas.setTextColor(WHITE);
  if (strcmp(unit_meas, "C") == 0) {
    sharedCanvas.drawCircle(47, labelY + 6, 2, WHITE); 
    sharedCanvas.setCursor(52, labelY + boxH - 6);
  } else {
    sharedCanvas.setCursor(48, labelY + boxH - 6);
  }
  sharedCanvas.print(unit_meas);

 // 5. Trimitere la ecran (DOAR zona utilă 66x26 dintr-un buffer de 80)
// Folosim versiunea extinsă a funcției:
// drawRGBBitmap(x, y, bitmap_ptr, w_util, h_util) 
// Dar pentru că buffer-ul e mai lat, trebuie să folosim o metodă de "decupare"
for(int line = 0; line < 26; line++) {
    // Luăm fiecare linie de 66 pixeli din buffer-ul de 80
    tft.drawRGBBitmap(xpos, ypos - labelY + line, sharedCanvas.getBuffer() + (line * sharedCanvas.width()), 66, 1);
}
}

/****** Rutina de desenare a unei etichete cu Label *******************/
void Label_text(int xpos, int ypos, const char* label, const char* text, int color) {
  const int boxW = 65;
  const int boxH = 21;
  const int labelY = 4;

  sharedCanvas.fillScreen(backcolor);

  sharedCanvas.drawRect(0, labelY, boxW, boxH, GRAY);

  sharedCanvas.setFont();
  sharedCanvas.setTextColor(GRAY, backcolor); 
  sharedCanvas.setCursor(5, 0);
  sharedCanvas.print(label);

  sharedCanvas.setFont(&FreeMono9pt7b);
  sharedCanvas.setTextColor(color);
  sharedCanvas.setCursor(4, labelY + boxH - 6);
  sharedCanvas.print(text);

  // 5. Trimitere la ecran (DOAR zona utilă 66x26 dintr-un buffer de 80)
// Folosim versiunea extinsă a funcției:
// drawRGBBitmap(x, y, bitmap_ptr, w_util, h_util) 
// Dar pentru că buffer-ul e mai lat, trebuie să folosim o metodă de "decupare"
for(int line = 0; line < 26; line++) {
    // Luăm fiecare linie de 66 pixeli din buffer-ul de 80
    tft.drawRGBBitmap(xpos, ypos - labelY + line, sharedCanvas.getBuffer() + (line * sharedCanvas.width()), 66, 1);
}
}

/*---------- Update pozitie ac SMOOTH-OPTIMIZAT -----------------*/
int oldValNeedle = 0;
float oldAngle = -90;

/*------------ On Air -----------------------------*/
void rxtx(){
  if(fmode == 2){                              // mode=CW?
  Vfo_out(vfofreq + CW_TONE);               // Vfofreq+700Hz
  show_msg("                CW TONE OUT 700Hz",1); 
  }
  flag_lock=1;
  tft.fillRect(195,168,120,24,backcolor);    
  tft.setFont(&FreeMonoBold12pt7b);
  tft.setTextColor(YELLOW); 
  tft.setCursor(221,186);             
  tft.print("ON-AIR");
  tft.setTextColor(RED); 
  tft.setCursor(220,185);             
  tft.print("ON-AIR");

  while(digitalRead(SW_TX) == LOW){
     citire_pozitie_ac();  //actualizeaza acul indicator pe afisaj  
         /*Refresh la 3sec VOLTS and TEMPERATURE measurement*/
    if(millis() > runseconds10volts + 1000)
    {
      show_voltage();
      show_temperature();
      runseconds10volts = millis();
    }
  }
 flag_lock =0;
 tft.fillRect(195,168,120,24,backcolor);    

  if(flag_rit){
     show_scara_mica(flag_rit);      
  }
}
/*------------- Mode AMP +20dB ---------------*/
void switch_amp(bool state){
  digitalWrite(AMP_OUT, state);
  *buttons[0].flag = state;                  // actualizează flag
  deseneazaButon(200, buttons, 0, 4);               // redesenează doar butonul 0    
}
/*------------- Mode ATT --------------------*/
void switch_att(bool state){
  digitalWrite(ATT_OUT, state); 
  *buttons[1].flag = state;                  // actualizează flag
  deseneazaButon(200, buttons, 1, 4);               // redesenează doar butonul 0    
}
/*------------- show_mode (LSB-USB-CW-AM) ------------*/
void show_mode(){
static uint8_t state = 0;
static unsigned long t0 = millis();

const char *str;

switch (fmode) {
  case 0: str = " LSB"; break;
  case 1: str = " USB"; break;
  case 2: str = " CW "; break;
}
  Label_text(250,88," MODE ", str, YELLOW);
}

/************* RIT-MODE ***********************/
void show_rit(bool state){
      *buttons[4].flag = state;         // actualizează flag
      deseneazaButon(200, buttons, 3, 4); // redesenează doar butonul 4
}
 
/************* AGC-MODE ***********************/
void show_agc(bool flag_agc){
      *buttons[2].flag = flag_agc;         // actualizează flag
          deseneazaButon(200, buttons, 2, 4); // redesenează doar butonul 2
}
/*   Rutina de setare a pasului encoder*/
void setstep(){  
  if(fstep==1000){ 
    fstep = 10; 
  }
  else {
    fstep *= 10; 
  }
  show_step(); 
}
/*-------------Display Step on LCD ---------------------*/
void show_step(){ 
  const char *step_str="";  
  if (fstep==1000)                        step_str="x1000";
  if (fstep==10)                          step_str=" x10 ";
  if (fstep==100)                         step_str=" x100";
  
  Label_text(250, 62, " STEP",step_str, CYAN);  
}

/*----------- Main frequency screen -------------------*/
void show_frequency1(long f){
  int x_pos=55;
  int y_pos=122;
  tft.setTextColor(GREEN);
    
  freq_str = String(f);
  tft.setFont(&FreeSansBold24pt7b);     
  int mojisuu = freq_str.length();

  if(f10 !=(freq_str.charAt(mojisuu-2))){
    tft.fillRect(x_pos + 198, y_pos+2, 28, 36,BLACK); 
    tft.setCursor(x_pos + 198, y_pos + 36);  
    tft.print(freq_str.charAt(mojisuu-2)); 
    f10 = (freq_str.charAt(mojisuu-2));
  }  
  if(f100 != (freq_str.charAt(mojisuu-3))){
   // tft.setTextColor(BLACK);
    tft.fillRect(x_pos + 170, y_pos+2, 28, 36,BLACK);
    tft.setCursor(x_pos + 170, y_pos + 36);
    tft.print(freq_str.charAt(mojisuu-3)); 
    f100 = (freq_str.charAt(mojisuu-3));
  }
  if(f>=1000){
    tft.setCursor(x_pos + 153, y_pos + 36);
    tft.print(".");
  }
  if(f1k !=(freq_str.charAt(mojisuu-4))){
    tft.fillRect(x_pos + 127, y_pos+2, 28, 36,BLACK);
    tft.setCursor(x_pos + 127, y_pos + 36);
    tft.print(freq_str.charAt(mojisuu-4));       
    f1k  = (freq_str.charAt(mojisuu-4));
  }
  if(f10k !=(freq_str.charAt(mojisuu-5))){
    //tft.setTextColor(BLACK);
    tft.fillRect(x_pos + 99, y_pos+2, 28, 36,BLACK);
    tft.setCursor(x_pos + 99, y_pos + 36);
    tft.print(freq_str.charAt(mojisuu-5));   
    f10k = (freq_str.charAt(mojisuu-5));
  }
  if(f100k !=(freq_str.charAt(mojisuu-6))){
    //tft.setTextColor(BLACK);
    tft.fillRect(x_pos + 71, y_pos+2, 28, 36,BLACK);
    tft.setCursor(x_pos + 71, y_pos + 36);
    tft.print(freq_str.charAt(mojisuu-6));   
    f100k = (freq_str.charAt(mojisuu-6));
  }
  if(f>=1000000){
    tft.setCursor(x_pos + 56, y_pos + 36);
    tft.print(".");
  }
  if(f<10000000){
//  umple golul lasat de Zeci de MHz
    tft.fillRect(x_pos, y_pos+2, 28, 36, BLACK);
  }
  if(fmega !=(freq_str.charAt(mojisuu-7))){
   // tft.setTextColor(BLACK);
    tft.fillRect(x_pos + 28, y_pos+2, 28, 36,BLACK);
    tft.setCursor(x_pos + 28, y_pos + 36);
    tft.print(freq_str.charAt(mojisuu-7));   
    fmega  = (freq_str.charAt(mojisuu-7));
  }
  if(f10m !=(freq_str.charAt(mojisuu-8))){
   // tft.setTextColor(BLACK);
    tft.fillRect(x_pos, y_pos+2, 28, 36,BLACK);
    tft.setCursor(x_pos, y_pos + 36);
    tft.print(freq_str.charAt(mojisuu-8));
    f10m = (freq_str.charAt(mojisuu-8));
  }  
}

/************** Arata VFO si schimba *******************/
void show_frequency2(int vfo){
    int xpos = 15, ypos = 158;
    const char *vfostr[] = {"A", "B"};
    
    tft.fillRect(xpos-2,ypos-16,20,20,backcolor);
    tft.fillRect(xpos-2,ypos+14,20,20,backcolor); 
    tft.setFont(&FreeMonoBold12pt7b); 
    
    tft.setTextColor(GREEN);
    if(vfo)tft.setTextColor(YELLOW);

    tft.setCursor(xpos,ypos);        
    tft.print(vfostr[vfo]); 
    
    tft.setTextColor(YELLOW);
    if(vfo)tft.setTextColor(GREEN);

    tft.setCursor(xpos,ypos+30);     
    tft.print(vfostr[!vfo]);
    
    tft.fillRect(xpos+33,ypos+14,145, 20,backcolor);
    tft.setTextColor(YELLOW);
    tft.setCursor(xpos+35, ypos+30);
/*-----------rutina de formatare String Frecventa--------------*/
/*** Rutina de formatare String generata de AI *********/    
    freq_str = String(freq[!cur_vfo]); 
    String formatted = "";                                    // Inițializare rezultat
    int length = freq_str.length();                           // Lungimea șirului
    
    for (int i = 0; i < length; i++) {
    formatted += freq_str.charAt(i);                          // Adaugă caracterul curent
      if ((length - i - 1) % 3 == 0 && i != length - 1) {
        formatted += ".";                                     // Adaugă punct dacă e nevoie
      }
    }
    tft.print(formatted);
}

/*Seteaza iesirile BCD pentru schimbarea benzilor*/
void switchBands(){
  digitalWrite(BAND_OUT1,LOW);
  digitalWrite(BAND_OUT2,LOW);
  digitalWrite(BAND_OUT3,LOW);
  if (band==0){}
  if (band==1){
   digitalWrite( BAND_OUT1,HIGH); 
  }
   if (band==2){
   digitalWrite(BAND_OUT2,HIGH); 
  }
  if (band==3){
   digitalWrite(BAND_OUT1,HIGH);
   digitalWrite(BAND_OUT2,HIGH); 
  }
  if (band==4){
   digitalWrite(BAND_OUT3,HIGH);
  }
  if (band==5){
   digitalWrite(BAND_OUT1,HIGH);
   digitalWrite(BAND_OUT3,HIGH); 
  }
  if (band==6){
   digitalWrite(BAND_OUT2,HIGH);
   digitalWrite(BAND_OUT3,HIGH);
  }
  if (band==7){
   digitalWrite(BAND_OUT1,HIGH);
   digitalWrite(BAND_OUT2,HIGH);
   digitalWrite(BAND_OUT3,HIGH);     
  }
}

/*---------- Show Band on LCD----------*/
void show_band_index(){
  int xpos=205;
  int ypos=105;
  tft.fillRect(xpos,ypos-16,35,20,backcolor);
  tft.setFont(&FreeMono9pt7b);
 // tft.setTextColor(WHITE);
  tft.setCursor(xpos,ypos);               //   tft.print("Banda ");
  tft.setTextColor(ORANGE);
  switch(band+1){ 
    case 1:
        tft.print("80m");
        break;
    case 2:
        tft.print("40m");
        break;
    case 3:
        tft.print("20m");
        break;
    case 4:
        tft.print("17m");
        break;
    case 5:
        tft.print("15m");
        break;
    case 6:
        tft.print("10m");
        break;                    
    default:
        break;    
  } 
}
/************ Arata mesajul SPLIT ********************/
void show_lock(bool ind){
  if(flag_rit)return;
  int xpos=230;
  int ypos=188;
  tft.setFont(&FreeMonoBold12pt7b);
  tft.setCursor(xpos,ypos);
  tft.fillRect(xpos,ypos-16,57,20,backcolor);
  if(ind){
    tft.setTextColor(ORANGE); tft.print("LOCK");
  }
}
/********* Mesaj afisat pe row 9*****************/
void show_msg(const char *msg, int bcg){  
  tft.setFont();
  tft.fillRect(0, 228, 320, 12, (bcg == 1) ? NAVY : backcolor);
  tft.setTextColor((bcg==1)? WHITE : GRAY);   
  tft.setCursor(0,230); 
  tft.print(msg);  
  runseconds10msg = millis(); 
  //message=(flag_xtalladj || flag_bfoadj)?0:1;
  message=1;
}

// --- Funcție pentru un singur buton ---
void deseneazaButon(uint16_t y, Button_t *btn, uint8_t index, uint8_t count) {
   tft.setFont(&FreeMonoBold12pt7b);
    uint16_t totalGap = (count + 1) * BTN_GAP;
    uint16_t btnW = (SCREEN_W - totalGap) / count;
    uint16_t x = BTN_GAP + index * (btnW + BTN_GAP);

    // border
    uint16_t borderColor = (*btn[index].flag) ? WHITE : GRAY;

    // curățare interior
    tft.fillRoundRect(x+1, y+1, btnW-2, BTN_H-2, BTN_R-1, BLACK);

    // contur
    tft.drawRoundRect(x, y, btnW, BTN_H, BTN_R, borderColor);

    // text bounds pentru centrare
    int16_t x1, y1;
    uint16_t w, h;
    tft.getTextBounds(btn[index].label, 0, 0, &x1, &y1, &w, &h);

    uint16_t tx = x + (btnW - w)/2;
    uint16_t ty = y + (BTN_H + h)/2;

    static const uint16_t onColors[5] = BTN_ON_COLORS;
    uint16_t color = GRAY;
    if (*btn[index].flag && index < 5)
        color = onColors[index];

    // afișare text
    tft.setCursor(tx, ty);
    tft.setTextColor(color, BLACK); // background negru -> șterge vechiul text
    tft.print(btn[index].label);
}

// --- Funcție pentru toate butoanele ---
void deseneazaGrupTaste(uint16_t y, Button_t *btn, uint8_t count) {
    tft.setFont(&FreeMonoBold12pt7b);

    for (uint8_t i = 0; i < count; i++) {
        deseneazaButon(y, btn, i, count);  // folosește funcția pentru un singur buton
    }
}
