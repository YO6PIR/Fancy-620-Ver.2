#define EEP_BAND          0x00         // EEProm BAND Adress
#define EEP_BANDS_MASK    0x01         // Nou: Mască activare benzi (1 byte)
#define EEP_XTAL          0x08         // EEProm Xtall Adress
#define EEP_INITIAL       0xFF         // INIT end Adress pt 24C02 eprom

void eepDataInit() {
  uint16_t Data;
  Data = eprom.read(EEP_INITIAL); 

  if (Data != 73) { // Dacă memoria e goală
    
    // --- DEFINIREA FUNCȚIEI LAMBDA ---
    // Folosim [&] pentru a avea acces la variabilele locale (eep_romadd, etc.) și obiectul eprom
    auto band2write = [&]() {
      for (int i = 0; i < 2; i++) {
        eprom.put((eep_romadd + 4 * i), eep_freq[i]);
        delay(5); // Timp pentru scriere hardware
      }
      eprom.write(eep_romadd + 8, eep_scala);   delay(5);
      eprom.write(eep_romadd + 10, eep_agc);    delay(5);
      eprom.write(eep_romadd + 12, eep_fmode);  delay(5);
      eprom.write(eep_romadd + 14, eep_fstep);  delay(5);
    };

    // --- ÎNCEPUT INIȚIALIZARE DATE ---
    eprom.write(EEP_BAND, 2); 
    delay(5);

    xtalFreq = XTAL_FREQ; 
    eprom.put(EEP_XTAL, xtalFreq); 
    delay(5);
    
    // BAND:0
    eep_romadd = 0x10;
    eep_freq[0] = 3710000; eep_freq[1] = 3580000;   
    eep_scala = 0; eep_agc = 1; eep_fmode = 0; eep_fstep = 1;
    band2write(); // Apelăm Lambda
  
    // BAND:1
    eep_romadd = 0x20;
    eep_freq[0] = 7100000; eep_freq[1] = 7050000;   
    eep_scala = 0; eep_agc = 1; eep_fmode = 0; eep_fstep = 1;
    band2write();

    // BAND:2
    eep_romadd = 0x30;
    eep_freq[0] = 14200000; eep_freq[1] = 14074000; 
    eep_scala = 0; eep_agc = 0; eep_fmode = 1; eep_fstep = 1;
    band2write();

    // BAND:3 ... și așa mai departe pentru toate benzile ...
    eep_romadd=0x40;                   // BAND:3 ROMadd:0x040
    eep_freq[0]=18100000; eep_freq[1]=18050000;    
    eep_scala=0; eep_agc=1; eep_fmode=1; eep_fstep=1;
    band2write();

    eep_romadd=0x50;                   // BAND:4 ROMadd:0x050
    eep_freq[0]=21200000; eep_freq[1]=21074000;   
    eep_scala=0; eep_agc=0; eep_fmode=1; eep_fstep=1;
    band2write();
  
    eep_romadd=0x60;                   // BAND:5 ROMadd:0x060
    eep_freq[0]=28400000; eep_freq[1]=28074000;    
    eep_scala=0; eep_agc=0; eep_fmode=1; eep_fstep=1;
    band2write();

    // ... Restul setărilor (Filter, BFO, etc.) ...
    bands_mask = 0x3F;
    eprom.write(EEP_BANDS_MASK, bands_mask); delay(5);
    
    eep_rombadd = 0x90;
    eep_bfo[0] = 7998500;       //LSB
    eep_bfo[1] = 8001500;       //USB
    eep_bfo[2] = 8000000;       //CW
    eep_bfo[3] = 8000100;       //ssb_filter_freq

    for (int i = 0; i < 4; i++) {
      eprom.put((eep_rombadd + 4 * i), eep_bfo[i]);
      delay(5);
    }

    eprom.write(EEP_INITIAL, 73); // Finalizare inițializare
    delay(5);
  }
}

/*---------- Band data write to eeprom ----------*/
void bandwrite(){
  romadd=0x010+(band*0x010);
  eprom.put(romadd,freq[0]); 
  eprom.put(romadd+4, freq[1]); 
  eprom.write(EEP_BAND,band);
  eprom.write(romadd+8,flag_scala);
  eprom.write(romadd+10,flag_agc);
  eprom.write(romadd+12,fmode);
  if (fstep==1000){steprom=1;}     
  if (fstep==10){steprom=2;}
  if (fstep==100){steprom=3;}
  eprom.write(romadd+14,steprom);
}
