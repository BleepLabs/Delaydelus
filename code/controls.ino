
void poll(){


  right_ch = right_db.update();
  right_state = right_db.read();

  left_ch = left_db.update();
  left_state = left_db.read();


  if ( right_ch && right_state == LOW ) {
    right_button=1;
  }

  else {
    right_button=0;
  }

  if ( left_ch && left_state == LOW ) {
    left_button=1;
  }

  else {
    left_button=0;
  }


  poll_tick++;

  if (poll_tick>15){
    poll_tick=0;
  }


  digitalWriteFast(left_select_pin,LOW);
  digitalWriteFast(right_select_pin,HIGH);

  for (int i = 0; i < 8; i++)
  {
    byte rl = digitalRead(pins[i]);
    left_en[i]=rl;
  }


  digitalWriteFast(left_select_pin,HIGH);
  digitalWriteFast(right_select_pin,LOW);

  for (int i = 0; i < 8; i++)
  {
    byte rr= digitalRead(pins[i]);
    right_en[i]=rr;
  }


  for (int i = 0; i < 8; i++)
  {
    if (left_en[i]==1 || right_en[i]==1)
    {
      //play_trig[i]=0;
    }
  }

  if (right_button==1)
  {
    for (int i = 0; i < 8; i++)
    {

      if (right_en[i]==0){

        if (shift_state==LOW)
        {
          play_trig[i+bank_sel[i]]=2;
          poly_add(i+bank_sel[i]);

        }

        else{
          play_trig[i+bank_sel[i]]=1;
          poly_add(i+bank_sel[i]);
          //Serial.println("right");



        }
        acc[i+bank_sel[i]]=0;
      }

    }

  }

  if (left_button==1)
  {
    for (int i = 0; i < 8; i++)
    {

      if (left_en[i]==0){

        if (sample_step==0){

          if (shift_state==LOW)
          {
            play_trig[i+bank_sel[i]]=2;
            poly_add(i+bank_sel[i]);
          }

          else{
            play_trig[i+bank_sel[i]]=1;
            poly_add(i+bank_sel[i]);
            //Serial.println("left");

          }
          acc[i+bank_sel[i]]=0;
        }

        if (sample_step==3){
          //    rec_trig[i]=1;
          acc[i+bank_sel[i]]=0;
        }

        if (sample_step==2){
          //  er_trig[i]=1;
          //  s_loc[i]=0;
        }

      }
    }
  }
}

//////////////////////////////////////////////////////////////////////////////////////////////



void pots(){

  pot_tick++;

  if (pot_tick>8){
    pot_tick=0;
  }

  if (sample_step==3){
    right_pot=0; //turn delay off

    if (pot_tick==6){
      uint16_t sm12 = digitalSmooth(analogRead(left_pot_pin),smootha0);
      left_pot=(digitalSmooth(sm12,smootha0)-255)*-1;
    }    

  }


  if (sample_step<3){

    if (pot_tick==0){
      //uint16_t sm10=digitalSmooth(analogRead(right_pot_pin)>>2, smootha1);
      uint16_t sm10=analogRead(right_pot_pin);
      right_pot=(sm10-255)*-1;
      //right_pot=analogRead(A6);
    }


    if (pot_tick==3){
      uint16_t sm3=digitalSmooth(analogRead(middle_pot_pin), smootha2);
      uint16_t log3=pow(sm3,2);
      mp_smooth=log3>>8;
      middle_pot=map(mp_smooth,0,255,NUM_SAMPS-4,0);
    }

    if (pot_tick==6){
      uint16_t sm12 = digitalSmooth(analogRead(left_pot_pin),smootha0);
      left_pot=(digitalSmooth(sm12,smootha0)-255)*-1;

      if (left_pot>64&&left_pot<128){
        rev=0;
        pitch_pot=map(left_pot,64,128,20,128);
      }

      if (left_pot>=128){
        rev=0;
        pitch_pot=map(left_pot,128,255,128,pitch_max);
      }

      if (left_pot<=64){
        rev=1;
        pitch_pot=map(left_pot,0,64,pitch_max,40);

      }

    }

  }

}

void controls(){



  pin_mode_b=in_mode_b;
  pshift_b=shift_b;

  mode_ch = mode_db.update();
  mode_state = mode_db.read();

  shift_ch = shift_db.update();
  shift_state = shift_db.read();

  shift_b=shift_state;

  if (shift_ch && shift_state==LOW)
  {
    if (master_mode==3)
    {
      thru_mode++;
      if (thru_mode>1)
      {
        thru_mode=0;
      }
    }
  }



  if ( mode_ch && mode_state == LOW ) {
    master_mode++;
    if (master_mode>3){
      master_mode=0;
    }
  }
  else {
    right_button=0;

  }

  if (master_mode==1 ) { //sampling sample_step
    sample_step=0;

    if (shift_b==0 && pshift_b==1){
      mod_mode=!mod_mode;
    }
  }

  ///++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++


  if (master_mode==2 && sample_step==0) { //sampling 

    if (left_state==0 )
    {

      bank_rst_c+=millis()-prevs;
      if (bank_rst_c>=1500){

        for (int i = 0; i < 8; i++)
        {
          if (left_en[i]==0 ){
            if (bank_sel[i]==8 && bank_rst_c>0){
              bank_sel[i]=0;
              master_mode=0;
              bank_rst_c=0;
              ee_store(i); 
            }

            if (bank_sel[i]==0 && bank_rst_c>0){
              bank_sel[i]=8;
              master_mode=0;
              bank_rst_c=0;
              ee_store(i); 
            }

          }

        }

      }

    }
    if (left_state==1){
      bank_rst_c=0;
    }






    if (shift_b==0){

      if (pshift_b==1){
        shift_b_c=0;
        shift_b_en=1;
      }

      if (shift_b_c<500){
        // shift_b_c++;
        shift_b_c+=millis()-prevs;
      }

      if (shift_b_c>=500){
        sample_step=1;
      }

    }

    prevs=millis();


    w_loc=0 ;
    buf_c=0;  
    //  playback=1;

    for(int i=8; i<16; i++){
      rec_trig[i]=0;
      rec_ready[i]=0;

    }

  }



  if (master_mode==2 && sample_step==1) {  //warning
    bank_rst_c=0;
    rec_lock=rec_happend=0;


    for(int i=0; i<8; i++){

      play_trig[i+bank_sel[i]]=0;
      //poly_clear();

    }

    if (shift_b==0 && pshift_b==1){
      sample_step=2;
    }


  }

  if (master_mode==2 && sample_step==2) {  ////////////////////////////////////////erase

    for (int i = 0; i < 8; i++)
    {


      if (left_en[i]==0 && rec_lock==0){
        poly_remove(i);

        bank_sel[i]=8;

        i=i+8;

        uint32_t el0 = pad_len*i *2;
        flash.erase_64k(el0);
        flash.erase_64k((el0)+0x010000);
        // flash.erase_64k((el0)+0x020000);
        // flash.erase_64k((el0)+0x030000);
        rec_ready[i]=1;
        er_trig[i]=0; 
        shift_b_c=0;
        sample_step=3;
        rec_i=i;
        rec_lock=1;
      }


    }

  }

  if (master_mode==2 && sample_step==3) {  //record

    rec_trig[rec_i]=left_state;


    if ( left_ch && left_state == HIGH ) {
      master_mode=0;
      sample_step=0;

    }

  }



}




int previnput[8];
int readChange(byte n, int input){
  int diff=32;
  //int input = analogRead(n);
  int output;
  if ((input>(previnput[n]+diff))||(input<(previnput[n]-diff))){

    output= input;
    previnput[n]=input;
    //Serial.println("C");
  }


  else{
    output=  previnput[n];
    ///Serial.println("-");
  }
  return output;
}


int knee_map(int input){ 
  int inhi=255;
  int inlow=0;
  int outhi=NUM_SAMPS-1;
  int outlow=2;
  int inknee=70;
  int outchg=64;
  int out;

  if (input<inknee){
    out = input;

  }


  if (input>=inknee){
    out=map(input,inknee,inhi,inknee,outhi);

  } 

  return out;

}


int hard_limit(int input){
  int temp_out;

  temp_out=input;


  if (input>2046){
    temp_out=2046;
    // digitalWrite(5,HIGH);
    //  Serial.println(" H");
  }

  if (input<-2046){
    temp_out=-2046;
    //digitalWrite(5,HIGH);
    // Serial.println(" L");

  }

  return temp_out;

}

int fold(int input){
  int outtie;

  if(input<=-4000){   
    int fold_amt=(input+4000)<<1;
    outtie=input-fold_amt;
  } 
  if(input>=4000){
    int fold_amt=(input-4000)<<1;
    outtie=input-fold_amt;
  } 

  return outtie;
}

int digitalSmooth(int rawIn, int *sensSmoothArray){     // "int *sensSmoothArray" passes an array to the function - the asterisk indicates the array name is a pointer
  int j, k, temp, top, bottom;
  long total;
  static int i;
  // static int raw[filterSamples];
  static int sorted[filterSamples];
  boolean done;

  i = (i + 1) % filterSamples;    // increment counter and roll over if necc. -  % (modulo operator) rolls over variable
  sensSmoothArray[i] = rawIn;                 // input new data into the oldest slot

  // Serial.print("raw = ");

  for (j=0; j<filterSamples; j++){     // transfer data array into anther array for sorting and averaging
    sorted[j] = sensSmoothArray[j];
  }

  done = 0;                // flag to know when we're done sorting              
  while(done != 1){        // simple swap sort, sorts numbers from lowest to highest
    done = 1;
    for (j = 0; j < (filterSamples - 1); j++){
      if (sorted[j] > sorted[j + 1]){     // numbers are out of order - swap
        temp = sorted[j + 1];
        sorted [j+1] =  sorted[j] ;
        sorted [j] = temp;
        done = 0;
      }
    }
  }

  /*
  for (j = 0; j < (filterSamples); j++){    // print the array to debug
   Serial.print(sorted[j]); 
   Serial.print("   "); 
   }
   Serial.println();
   */

  // throw out top and bottom 20% of samples - limit to throw out at least one from top and bottom
  bottom = max(((filterSamples * 20)  / 100), 1); 
  top = min((((filterSamples * 80) / 100) + 1  ), (filterSamples - 1));   // the + 1 is to make up for asymmetry caused by integer rounding
  k = 0;
  total = 0;
  for ( j = bottom; j< top; j++){
    total += sorted[j];  // total remaining indices
    k++; 
    // Serial.print(sorted[j]); 
    // Serial.print("   "); 
  }

  //  Serial.println();
  //  Serial.print("average = ");
  //  Serial.println(total/k);
  return total / k;    // divide by number of samples
}



void poly_add(int pad){
  byte added=0;;

  for (int i = 0; i < 4; i++)
  {
    //Serial.print("poly_add");Serial.println(i);
    if (voice_bank[i]==pad && added==0){
      //Serial.print(i);Serial.print(" already ");Serial.println(pad);
      added=1;
    }
}

  for (int i = 0; i < 4; i++)
  {
    if (voice_bank[i]==100 && added==0){ 
      //Serial.print(i);Serial.print(" add ");Serial.println(pad);
      voice_bank[i]=pad;
      added=1;
    }
  }


}

void poly_remove(byte pad){
  byte done;

  // not working
  for (int i = 0; i < 4; i++)
  {
    //Serial.print("poly_remove");Serial.println(i);

    if (voice_bank[i]==pad){
      //Serial.print(i);Serial.print(" rem ");Serial.println(pad);
      voice_bank[i]=100;
    }
  }


}

void poly_clear(){
  for (int i = 0; i < 4; i++)
  {
    voice_bank[i]=0;
  }
}


