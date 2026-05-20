#define BMP_WIDTH  240
#define BMP_HEIGHT 120

const int centerX =          120;
const int centerY =          155;
const int needleLength =      130;
// Ajustare pentru Landscape 320x240
const int centerX_LS = 160; // Centrul orizontal (320 / 2)
const int centerY_LS = 210; // Coborâm centrul ceasului spre baza ecranului


void fillCanvasFromRLE(int bx, int by) {
  uint32_t rleIndex = 0;
  int px = 0, py = 0;
  int cw = sharedCanvas.width();  // Folosim lățimea canvas-ului partajat
  int ch = sharedCanvas.height(); // Folosim înălțimea lui

  sharedCanvas.fillScreen(0);

  while (rleIndex < sizeof(rle_bitmap) / sizeof(rle_bitmap[0])) {
    uint16_t count = rle_bitmap[rleIndex++];
    uint16_t color = rle_bitmap[rleIndex++];

    while (count--) {
      // Coordonate ABSOLUTE raportate la BMP_WIDTH (240)
      if (px >= bx && px < bx + cw && py >= by && py < by + ch) {
          sharedCanvas.drawPixel(px - bx, py - by, color);
      }
      px++;
      if (px >= 240) { px = 0; py++; }
      if (py >= by + ch) return; // Optimizare: ieșim imediat ce am umplut canvas-ul
    }
  }
}

void citire_pozitie_ac() {
    int raw = citireADCmediatFast(SMETER);
    const int deadband = 4;

    if (abs(raw - oldValNeedle) <= deadband) {
        raw = oldValNeedle;
    } else {
        oldValNeedle = raw;
    }

    // Mapare unghi țintă
    float targetAngle = map(raw, 0, 4096, -136, -44);
    targetAngle = constrain(targetAngle, -136, -44);

    if (firstDraw) {
        updateNeedleAtomic(targetAngle, targetAngle); // Prima dată desenăm fix la țintă
        oldAngle = targetAngle;
        firstDraw = false;
    } 
    else {
        // --- LOGICA DE SMOOTHING ---
        const float smoothing = 0.25f; // Poți pune 0.05f pentru un ac și mai "greu"
        
        // Calculăm poziția intermediară
        float newAngle = oldAngle + smoothing * (targetAngle - oldAngle);

        // Desenăm doar dacă mișcarea e vizibilă
        if (fabs(newAngle - oldAngle) > 0.85f) { 
            updateNeedleAtomic(newAngle, oldAngle);
            // Actualizăm poziția "veche" cu cea pe care tocmai am desenat-o
            oldAngle = newAngle; 
        }
    }
}

/*---------- Desenare ceas indicator analogic -------------------*/
void drawRLEBitmap16() {
  uint32_t rleIndex = 0;
  uint32_t px = 0, py = 0;

  while (rleIndex < sizeof(rle_bitmap)/sizeof(rle_bitmap[0])) {
    uint16_t count = rle_bitmap[rleIndex++];
    uint16_t color = rle_bitmap[rleIndex++];
    while (count--) {
      tft.drawPixel(px, py, color);
      px++;
      if (px >= BMP_WIDTH) { px=0; py++; }
      if (py >= BMP_HEIGHT) return;
    }
  }
}

/*-------- Redesenare ceas analog doar zona afectată de ac ------------*/
void drawRLEBitmapBox(int xStart, int yStart, int xEnd, int yEnd) {
  uint32_t rleIndex = 0;
  uint32_t px = 0, py = 0;

  while (rleIndex < sizeof(rle_bitmap)/sizeof(rle_bitmap[0])) {
    uint16_t count = rle_bitmap[rleIndex++];
    uint16_t color = rle_bitmap[rleIndex++];
    while (count--) {
      if (px >= xStart && px <= xEnd && py >= yStart && py <= yEnd) {
        tft.drawPixel(px, py, color);
      }
      px++;
      if (px >= BMP_WIDTH) { px=0; py++; }
      if (py > yEnd) return;
    }
  }
}

void updateNeedleAtomic(float newAngle, float oldAngle) {
  const int CW = 80;  
  const int CH = 80;
  const int BMP_H = 120; // Înălțimea totală a datelor RLE
  
  // AJUSTEAZĂ AICI: La ce pixel (0-120) se termină rama ta de jos?
  // Dacă rama se termină mai sus, pune 110 sau 115.
  const int LIMITA_RAMA = 95; 

  // --- PASUL 1: ȘTERGERE ---
  float radOld = oldAngle * DEG_TO_RAD;
  int midX_old = centerX + (needleLength - 40) * cos(radOld);
  int midY_old = centerY + (needleLength - 40) * sin(radOld);
  
  int bxOld = constrain(midX_old - (CW/2), 0, BMP_WIDTH - CW);
  int byOld = constrain(midY_old - (CH/2), 0, BMP_H - CH); 

  fillCanvasFromRLE(bxOld, byOld);

  // Calculăm înălțimea să nu atingă rama
  int hUtilOld = LIMITA_RAMA - byOld; 
  if (hUtilOld > CH) hUtilOld = CH;
  if (hUtilOld < 0) hUtilOld = 0;

  if (hUtilOld > 0) {
    tft.drawRGBBitmap(bxOld, byOld, sharedCanvas.getBuffer(), CW, hUtilOld);
  }

  // --- PASUL 2: DESENARE ---
  float radNew = newAngle * DEG_TO_RAD;
  int midX_new = centerX + (needleLength - 40) * cos(radNew);
  int midY_new = centerY + (needleLength - 40) * sin(radNew);
  
  int bxNew = constrain(midX_new - (CW/2), 0, BMP_WIDTH - CW);
  int byNew = constrain(midY_new - (CH/2), 0, BMP_H - CH); 

  fillCanvasFromRLE(bxNew, byNew);

  int vrfX = (centerX + needleLength * cos(radNew)) - bxNew;
  int vrfY = (centerY + needleLength * sin(radNew)) - byNew;
  int bzaX = (centerX + (needleLength - 55) * cos(radNew)) - bxNew;
  int bzaY = (centerY + (needleLength - 55) * sin(radNew)) - byNew;

  sharedCanvas.drawLine(bzaX - 1, bzaY, vrfX - 1, vrfY, MAGENTA);
  sharedCanvas.drawLine(bzaX, bzaY, vrfX, vrfY, WHITE);
  sharedCanvas.drawLine(bzaX + 1, bzaY, vrfX + 1, vrfY, MAGENTA);

  // Calculăm înălțimea să nu atingă rama
  int hUtilNew = LIMITA_RAMA - byNew; 
  if (hUtilNew > CH) hUtilNew = CH;
  if (hUtilNew < 0) hUtilNew = 0;

  if (hUtilNew > 0) {
    tft.drawRGBBitmap(bxNew, byNew, sharedCanvas.getBuffer(), CW, hUtilNew);
  }
}
