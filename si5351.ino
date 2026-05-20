void Si5351Strengh(){
  switch (Si5351OscPower[1]){
          case 1:
              si5351.drive_strength(SI5351_CLK0, SI5351_DRIVE_2MA);
             break;
          case 2:
              si5351.drive_strength(SI5351_CLK0, SI5351_DRIVE_4MA);
             break;
          case 3:
              si5351.drive_strength(SI5351_CLK0, SI5351_DRIVE_6MA);
             break;
          case 4:
              si5351.drive_strength(SI5351_CLK0, SI5351_DRIVE_8MA);
             break;
          default:
              si5351.drive_strength(SI5351_CLK0, SI5351_DRIVE_2MA);
              Si5351OscPower[1] = 1;
             break;
  }
  switch (Si5351OscPower[2]){
          case 1:
              si5351.drive_strength(SI5351_CLK1, SI5351_DRIVE_2MA);
             break;
          case 2:
              si5351.drive_strength(SI5351_CLK1, SI5351_DRIVE_4MA);
             break;
          case 3:
              si5351.drive_strength(SI5351_CLK1, SI5351_DRIVE_6MA);
             break;
          case 4:
              si5351.drive_strength(SI5351_CLK1, SI5351_DRIVE_8MA);
             break;
          default:
              si5351.drive_strength(SI5351_CLK1, SI5351_DRIVE_2MA);
              Si5351OscPower[2] = 1;
             break;
  }
  switch (Si5351OscPower[3]) {
          case 1:
              si5351.drive_strength(SI5351_CLK2, SI5351_DRIVE_2MA);
             break;
          case 2:
              si5351.drive_strength(SI5351_CLK2, SI5351_DRIVE_4MA);
             break;
          case 3:
              si5351.drive_strength(SI5351_CLK2, SI5351_DRIVE_6MA);
             break;
          case 4:
              si5351.drive_strength(SI5351_CLK2, SI5351_DRIVE_8MA);
             break;
          default:
              si5351.drive_strength(SI5351_CLK2, SI5351_DRIVE_2MA);
              Si5351OscPower[3] = 1;
             break;  
        }          
}
/********************************************************************/
void si5351_1(){
  si5351_freq = freqv * 100ULL;
  si5351.set_freq(si5351_freq, SI5351_CLK0);
  delayMicroseconds(10); // lasă semnalul să se stabilizeze
}
/********************************************************************/
void si5351_2(){
  si5351_freq = freqv * 100ULL;
  si5351.set_freq(si5351_freq, SI5351_CLK1);
  delayMicroseconds(10); // lasă semnalul să se stabilizeze
}
/********************************************************************/
void si5351_3(){
  si5351_freq = freqv * 100ULL;
  si5351.set_freq(si5351_freq, SI5351_CLK2);
  delayMicroseconds(10); // lasă semnalul să se stabilizeze
}
/*--- Rutina de resetare Si5351 ---------------*/
void ResetSi5351() {
    // Hard reset logic
    si5351.init(SI5351_CRYSTAL_LOAD_8PF, 0, 0);  
    // Activăm ieșirile (logic on) → dar 0 Hz
    si5351.output_enable(SI5351_CLK0, 0);
    si5351.output_enable(SI5351_CLK1, 0);
    si5351.output_enable(SI5351_CLK2, 0);
}
