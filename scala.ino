/************ Deseneaza scala mare pe mijlocul ecranului******************************/
void show_scara_mare(){
  int xpos=47;    //coordonata x
  int ypos=124;  // coordonata y
  tft.setFont(&FreeSans9pt7b);
  tft.setTextColor(GREEN);
  if(cur_vfo) tft.setTextColor(YELLOW);
     
  int x_start = xpos+3; //coordonata x de unde incepe prima diviziune a scalei
  int baseline_y = ypos+36; // coordonata y unde se aseaza baza scalei
  int spacing; 
  int thickness = 3;
  int line_count;                                   
  int number;
  int divizor;
    
    switch (band){    /*Divizor=(Fmax-Fmin)/((line_count-1)*spacing);*/
       case 0://Banda 80m   
          number=35;spacing=16; line_count=16; divizor=1250;break;  // 3500-3800
       case 1://Banda 40m   
          number=700;spacing=12; line_count=21; divizor=833;break; // 7000-7200
       case 2://Banda 20m   
          number=140; spacing=12; line_count=21;divizor=1666;break; // 14000-14400
       case 3://Banda 17m   
          number=1800; spacing=12; line_count=21;divizor=833;break; //  18000-18200
       case 4://Banda 15m   
          number=210; spacing=10; line_count=26;divizor=2000;break;  //  21000-21500
       case 5://Banda 10m   
          number=2800; spacing=12; line_count=21;divizor=4166;break;  //  28000-29000
    }
   // tft.setTextColor(BLACK); 
    tft.fillRect(2+xpos+(freqold- Flim[band][0])/divizor, ypos, thickness+2, 36,BLACK);      //sterge acul_ind old

    if(!init_flag)freqold=freq[cur_vfo];           //actualizeaza frecventa noua
    if(init_flag)init_flag=0;
    
  //  tft.setTextColor(WHITE); 
    bool current_is_small;

    tft.fillRect(2+xpos+(freq[cur_vfo]-Flim[band][0])/divizor, ypos, thickness+2, 36,RED);  //deseneaza acul rosu
    current_is_small = false; //start_with_small;              // Setăm sa începem cu linie mare
  
  for (int i = 0; i < line_count; i++){
    if((i==0)||(i%5==0))  current_is_small = false;                 // Alternăm linia următoare
    
    int len = current_is_small ? 8 : 18;                  // Linia mică = 8px, linia mare = 12px
    int x_position = x_start + (i * spacing);
    int y_position = baseline_y - len;

    //tft.setTextColor(WHITE);
    tft.fillRect(x_position, y_position, thickness, len,WHITE);

    if (!current_is_small){                               // Doar liniile mari primesc cifre
      int text_x = x_position-20;                           //= (number < 0) ? x_position -  : x_position-5;
      tft.setCursor(text_x, y_position - 7);
            
      if(band==1 || band==3 || band==5){
        tft.print(number/100);tft.print(".");
        if((band==1 && i==5)||(band==3 && i==5))tft.print(0);
        tft.print(number%100);
        if(band==1 && i==0)tft.print(0);
        if(band==5)number+=25; else number+=5; 
      }
      else{
        tft.print(number/10);tft.print(".");tft.print(number%10);        
        if(band==0)tft.print(0);
        number +=1;  
      }    
    }
    current_is_small=true;    
 }
}
     
/*----------Rutina generata cu ajutorul AI-----------------------*/
void show_scara_mica(int act){
  if (flag_lock)return;
  int xpos=195;
  int ypos=160;  

  tft.setFont(); 
  if(!act)tft.fillRoundRect(xpos,ypos+8,120,25,3,backcolor);
  int x_start = xpos+10; //209;
  int baseline_y = ypos+30;
  int spacing; 
  int thickness = 2;
  int line_count;                                   
  int number;
  int divizor;
  number=-2; spacing=10; line_count=11;
 
  tft.setTextColor(BLACK); 
  tft.fillRect(xpos+58+freq_ifshift_old/5, ypos+16, thickness+2, 15 , backcolor);  //sterge acul vechi
  
  freq_ifshift_old = freq_ifshift;  

  tft.setTextColor(RED); 
  bool current_is_small;
  if(act){
  tft.fillRect(xpos+58+freq_ifshift/5, ypos+16, thickness+2, 15, RED);    //deseneaza acul rosu
  }
  current_is_small = !start_with_small;              // Setăm să începem cu linie mică
  for (int i = 0; i < line_count; i++) {
    int len = current_is_small ? 8 : 12;                  // Linia mică = 8px, linia mare = 12px
    int x_position = x_start + (i * spacing);
    int y_position = baseline_y - len;

    tft.setTextColor(GRAY);
    if (act){
    tft.setTextColor(WHITE);
    tft.drawRect(x_position, y_position, thickness, len,WHITE);
    tft.setTextColor(CYAN);
     if (!current_is_small) {                              // Doar liniile mari primesc cifre
      int text_x = (number < 0) ? x_position - 8 : x_position-5;
      if (number==0)text_x=x_position-2;
      tft.setCursor(text_x, y_position - 10);
      tft.print(number);if(number!=0)tft.print("0");
      number++;
     }
    }
    current_is_small = !current_is_small;                 // Alternăm linia următoare
  }
}
