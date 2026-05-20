// Variabile control meniu (locale acestui tab)
int menu_index = 0;
bool edit_mode = false;

// --- FUNCTIA DE SALVARE ---
void saveSystemSettings() {
  eprom.put(EEP_XTAL, xtalFreq); 
  
  // Calcul BFO bazat pe offset
  //eep_bfo[0] = ssb_filter_freq - bfo_offset; // LSB
 // eep_bfo[1] = ssb_filter_freq + bfo_offset; // USB
 // eep_bfo[2] = ssb_filter_freq;            // CW
  eep_bfo[3] = ssb_filter_freq;
  
  eprom.put(0x90, eep_bfo[0]); // LSB
  eprom.put(0x94, eep_bfo[1]); // USB
  eprom.put(0x98, eep_bfo[2]); // CW
  eprom.put(0x9C, eep_bfo[3]); //SSB Filter Freqv
  
  byte mask = 0;
  for(int i=0; i<6; i++) {
    if(bands_active[i]) mask |= (1 << i);
  }
  eprom.write(EEP_BANDS_MASK, mask);
  bitmask=mask;
}

// --- FUNCTIA DE INCARCARE PARAMETRI DE SISTEM ---
void loadSystemSettings() {
  eprom.get(EEP_XTAL, xtalFreq);                //read Frecv Xtall from memory
  band = eprom.read(EEP_BAND);                  //read Band from memory
  //citeste 3 frecvente stocate dela adresa 0x10 →
  romadd = 0x10+(band*0x10);
  for (int i=0; i<3;i++){
  eprom.get((romadd+4*i),romf[i]);  
  }
  for( int i=0; i<2; i++){                      //citeste FreqA - FreqB
   freq[i]=romf[i];
  }
  scalarom = eprom.read(romadd+8);              //citeste scala
  agcrom = eprom.read(romadd+10);               //citeste AGC
  fmode = eprom.read(romadd+12);                //citeste MODE
  steprom = eprom.read(romadd+14);              //cieste STEP  
  bitmask=eprom.read(EEP_BANDS_MASK);           //citeste masca benzilor active
  
  eep_rombadd = 0x90;                          // EEPROM read(BFO)
  for (int i=0; i<4;i++){
    eprom.get((eep_rombadd+(4*i)),romb[i]);  
    eep_bfo[i] = romb[i];    
  }
  ssb_filter_freq = eep_bfo[3];
 
  if (steprom==1){fstep=1000;}                      
  if (steprom==2){fstep=10;}
  if (steprom==3){fstep=100;}
  flag_scala = scalarom;
  flag_agc = agcrom;  
 
  bfo_offset = abs(ssb_filter_freq - eep_bfo[0]); // Diferenta este offset-ul

  byte mask = eprom.read(EEP_BANDS_MASK);
  for(int i=0; i<6; i++) {
    bands_active[i] = (mask >> i) & 0x01;
  }
}

// --- LOGICA MODIFICARE VALORI ---
void updateValue(int index, int dir) {
  switch(index) {
    case 0: bfo_offset += (dir * 10); // Schimbăm de la 10 la 50 Hz
            break;      
    case 1: ssb_filter_freq += (dir * 100); break;              // Filtru +/- 100Hz
    case 2: xtalFreq += (dir * 10);                             // Calibrare XTAL fina 10Hz
            si5351.init(SI5351_CRYSTAL_LOAD_8PF, xtalFreq, 0);  // Actualizează ceasul intern imediat
            //old_cio = 0;
            break;           
    case 3: case 4: case 5: case 6: case 7: case 8:                    // Cele 5-6 benzi active
            bands_active[index-3] = !bands_active[index-3];
            break;
  }
}

// 1. Funcția de citire a butonului cu detectare de timp
int checkButtonAction() {
  if (get_keys() == 5) { // Tasta SW_STEP apasata
    unsigned long startPress = millis();
    
    // Stăm în buclă cât timp tasta este menținută apăsată
    while (get_keys() == 5) {
      // Dacă am depășit 2 secunde, putem da un feedback vizual rapid (opțional)
      if (millis() - startPress > 2000) {
        // Feedback opțional: am putea schimba ceva pe ecran aici
        int yPos = 90;
        tft.fillRect(0, yPos - 15, 319, 23, backcolor); 
      }
      delay(10); 
    }
    // Calculăm durata totală după ce tasta a fost eliberată
    if (millis() - startPress > 2000) {
      return 2; // Long Press detectat
      
    } else {
      return 1; // Short Press detectat
    }
  }
  return 0; // Nicio acțiune
}

// 2. Bucla principală din setari.ino modificată să folosească noul sistem
void loopSettings() {
  tft.setFont();
  tft.fillScreen(BLACK);
  tft.setTextColor(YELLOW,BLACK);
  tft.setTextSize(2);
  tft.setCursor(60, 10);
  tft.print("SISTEM SETTINGS"); 
  tft.drawLine(0, 32, 320, 32, GRAY);

  for(int i=0; i<=9; i++) drawMenuRow(i); 
 
  bool exit_flag = false;
  int old_menu_index = 0;

  while (get_keys() == 5) {
      delay(10); // O mică pauză pentru stabilitate
  }
/*
  while(!exit_flag) {
    // A. Gestionare Encoder
    int dir = readEncoder();
    if (dir != 0) {
      if (!edit_mode) {
        old_menu_index = menu_index;
        menu_index = constrain(menu_index + dir, 0, 9);
        drawMenuRow(old_menu_index); 
        drawMenuRow(menu_index);
      } else {
        updateValue(menu_index, dir);
        drawMenuRow(menu_index);
      }
    }

    // B. Gestionare Buton (Scurt / Lung)
     int btnAction = checkButtonAction();

    if (btnAction == 2) { // LONG PRESS (2 secunde+)
      if (menu_index == 2) { // Suntem pe rândul Si5351 XTAL
        if (xtalFreq < 26000000) xtalFreq = 27000000;
        else xtalFreq = 25000000;
        
        si5351.init(SI5351_CRYSTAL_LOAD_8PF, xtalFreq, 0); 
        drawMenuRow(menu_index); // Update vizual imediat
      }
    } 
    else if (btnAction == 1) { // SHORT PRESS
      if (menu_index == 9) { // EXIT & SAVE
       // saveSystemSettings();
        exit_flag = true;
      } else {
        edit_mode = !edit_mode;
        drawMenuRow(menu_index);
      }
    }
   
     bfo_offset = constrain(bfo_offset, 0, 5000); 
     eep_bfo[0] = ssb_filter_freq - bfo_offset; // LSB
     eep_bfo[1] = ssb_filter_freq + bfo_offset; // USB
     // trimite_frecventa_la_si5351() in functie de modul activ [fmode→LSB/USB]
     
     if(edit_mode) {
    if(menu_index == 0 || menu_index == 1) {
        // --- REGLAJ BFO ---
        freqv = eep_bfo[fmode]; // Pregătim valoarea pentru BFO
        si5351_2();             // Trimitem la CLK1
        // Aici nu apelăm si5351_1(), deci VFO rămâne neatins (dacă nu a fost init)
    }
    else if(menu_index == 2) {
        // --- REGLAJ XTAL (Aici ambele trebuie "trezite" după init) ---
        
        // 1. Pregătim și trimitem VFO primul
        freqv = freq[cur_vfo] + eep_bfo[fmode]; 
        si5351_1(); // Trimite VFO pe CLK0
        
        // 2. Pregătim și trimitem BFO imediat după
        freqv = eep_bfo[fmode];
        si5351_2(); // Trimite BFO pe CLK1
    }
  }
       
  }
  */
  while(!exit_flag) {
  // 1. GESTIONARE ENCODER
  int dir = readEncoder();
  if (dir != 0) {
    if (!edit_mode) {
      old_menu_index = menu_index;
      menu_index = constrain(menu_index + dir, 0, 9);
      drawMenuRow(old_menu_index); 
      drawMenuRow(menu_index);
    } else {
      updateValue(menu_index, dir); // Aici se schimbă bfo_offset, xtalFreq, etc.
      
      // --- ACTUALIZARE HARDWARE INSTANTĂ (Doar dacă suntem în EDIT) ---
      refreshHardware(); 
      
      drawMenuRow(menu_index);
    }
  }

  // 2. GESTIONARE BUTON
  int btnAction = checkButtonAction();

  if (btnAction == 2) { // LONG PRESS
    if (menu_index == 2) { // Reset XTAL la valori fixe
      xtalFreq = (xtalFreq < 26000000) ? 27000000 : 25000000;
      si5351.init(SI5351_CRYSTAL_LOAD_8PF, xtalFreq, 0); 
      
      refreshHardware(); // Forțăm trezirea tuturor ieșirilor după init()
      drawMenuRow(menu_index);
    }
  } 
  else if (btnAction == 1) { // SHORT PRESS
    if (menu_index == 9) { 
      exit_flag = true; 
    } else {
      edit_mode = !edit_mode;
      drawMenuRow(menu_index);
    }
  }
}

  saveSystemSettings();
  tft.fillScreen(BLACK);
    tft.setTextSize(1);
    if(!flag_scala){
       tft.fillRect(47,119,270,44,backcolor);                         //fundal scara mare
       show_frequency1(0);                                            //Dump-buffer pt Freq1 
       delay(10);
    }
 return;
}
//Functia de calculare miscare encoder
int readEncoder() {
  if (flag_up) {
    flag_up = false; // "Consumăm" mișcarea
    return 1;        // Spunem meniului să meargă înainte
  }
  if (flag_dw) {
    flag_dw = false; // "Consumăm" mișcarea
    return -1;       // Spunem meniului să meargă înapoi
  }
  return 0;          // Repaus
}
//Functia de refresh hardware daca se schimba ceva in meniul Settings
void refreshHardware() {
  // Calculăm valorile noi de BFO
  bfo_offset = constrain(bfo_offset, 0, 5000); 
  eep_bfo[0] = ssb_filter_freq - bfo_offset; // LSB
  eep_bfo[1] = ssb_filter_freq + bfo_offset; // USB
  eep_bfo[2] = ssb_filter_freq;

  if (menu_index == 0 || menu_index == 1) {
    // REGLAJ BFO: Trimitem doar BFO (CLK1)
    freqv = eep_bfo[fmode];
    si5351_2(); 
  } 
  else if (menu_index == 2) {
    // REGLAJ XTAL: Trimitem ambele (CLK0 + CLK1) pentru a compensa init()
    freqv = 10000000UL; //freq[cur_vfo] + eep_bfo[fmode]; 
    si5351_1(); 
    
    freqv = eep_bfo[fmode];
    si5351_2();
  }
}
//Functia de desenare a meniurilor pe ecran
void drawMenuRow(int i) {
  const char* labels[] = {
    "BFO Offset", "SSB Filter", "Si5351 XTAL", 
    "Band 80m", "Band 40m", "Band 20m", "Band 17m", "Band 15m", "Band 10m", 
    "SAVE & EXIT"
  };

  int yPos = 40 + (i * 20);
  uint16_t colorText, colorBg;

  if (i == menu_index) {
    colorText = CYAN;
    colorBg = edit_mode ? MAROON : DARKGRAY;
  } else {
    colorText = LIGHTGRAY;
    colorBg = BLACK;
  }

  // Desenăm fundalul rândului
  tft.fillRect(0, yPos - 5, 319, 23, colorBg);
  // 3. Setăm fontul la (2)
  tft.setTextSize(2);
  tft.setTextColor(colorText, colorBg);

  // Etichetă
  tft.setCursor((i >= 3 && i <= 8) ? 30 : 10, yPos);
  tft.print(labels[i]);

  // 5. Poziționare Valori (Ajustăm X la 160 pentru a lăsa loc label-urilor lungi)
  tft.setCursor(160, yPos);
  switch(i) {
    case 0: tft.print(bfo_offset); tft.print(" Hz    "); break;
    case 1: tft.print(ssb_filter_freq / 1000.0, 3); tft.print(" MHz "); break;
    case 2: tft.print(xtalFreq); tft.print("    "); break;
    case 3: case 4: case 5: case 6: case 7:case 8:
      tft.setTextColor(bands_active[i-3] ? GREEN : RED, colorBg);
      tft.print(bands_active[i-3] ? "[ ON ] " : "[ OFF ]");
      break;
  }
}
