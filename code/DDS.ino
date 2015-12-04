void hi(){

  // dds_d= micros();
  p_mode=sample_step;

  if (master_mode!=0) 
  {
    mod_in=(analogRead(input_pin))-127;  //omly read audio in when not in normal play mode
  }


//linear interpolation for delay time
  lerp_tick++;

if (tap_rate==0 || tap_rate>NUM_SAMPS-10){
  if(lerp_tick>1){
    byte dly_step=1;
    if(lerp_dly <= middle_pot-4){   
      lerp_dly +=dly_step;
    }
    if(lerp_dly >= middle_pot+4){
      lerp_dly -= dly_step;
    }

    lerp_tick=0;
  }
}

if (tap_rate>0 && tap_rate<NUM_SAMPS-10){
  if(lerp_tick>1){
    byte dly_step=1;
    if(lerp_dly <= tap_rate-4){   
      lerp_dly +=dly_step;
    }
    if(lerp_dly >= tap_rate+4){
      lerp_dly -= dly_step;
    }

    lerp_tick=0;
  }
}



  if (master_mode==0) 
  {
    thru_mode=0;
    mod_mode=0;
    sample_step=0;
    fm_mod=0;
    post_mod=sample_out;

  }


  if (master_mode==1 && mod_mode==1) //am
  {
    ain =mod_in;
    post_mod=(sample_out*ain)>>6;

  }


  if (master_mode==1 && mod_mode==0) // trigger
  {
    post_mod=sample_out;
    trigger_cut=60;
    int amod_in=mod_in;

    if (amod_in>70 && p_mod_in<70 && lockout==0)
    {
      for (int i = 0; i < 16; i++)
      {
        if (left_en[i]==0){
          play_trig[i+bank_sel[i]]=1;
          poly_add(i+bank_sel[i]);
          acc[i]=0;
        }

      }
      lockout=1;

    }
    p_mod_in=amod_in;

  }

  if (lockout==1){
    lockout_t++;
    if (lockout_t>10){
      lockout=0;
      lockout_t=0;
    }
  }

  /*
  if (mod_mode==99) //fm dosen't sound too great
   {
   //fm_mod=(mod_in>>2)-32;
   fm_mod=mod_in-127;
   post_mod=sample_out;
   }
   */

  if (master_mode==2) //rec
  {
    mod_mode=0;
    fm_mod=0;

  }


  if (sample_step==3){ //record
    ain =(mod_in*left_pot)>>1;
    ainf=ain;
    post_mod=(mod_in*left_pot)>>2;

    //sample_out=(ainf/3);

  }
  
  record(rec_i,ainf);


  if (master_mode==3){ //through 
    sample_step=0;
    ain =(mod_in*left_pot)>>2;
    post_mod=((ain)*thru_env)>>8;

    if (thru_mode==0)
    {
      if (left_state==HIGH){
        thru_env+=8;

        if (thru_env>240){
          thru_env=255;
        }
      }

      if (left_state==LOW){
        thru_env-=4;

        if (thru_env<=8){
          thru_env=0;
        }
      }

    }


    if (thru_mode==1)
    {
      if (left_state==LOW){
        thru_env+=8;

        if (thru_env>240){
          thru_env=255;
        }
      }

      if (left_state==HIGH){
        thru_env-=4;

        if (thru_env<=8){
          thru_env=0;
        }
      }

    }
  }


  if (sample_step==0 && master_mode!=3){ //play & idle

    tick++;

    if (tick>3){
      tick=0;
    }
  
    if (midi_pitch_en==0){
    spitch=pitch_pot;
    }
    
    if (midi_pitch_en==1){
    spitch=midi_pitch;
    }
    
    // sample_t=micros();
    sample(0,voice_bank[0],spitch,read_buf[0]);
    sample(1,voice_bank[1],spitch,read_buf[1]);
    sample(2,voice_bank[2],spitch,read_buf[2]);
    sample(3,voice_bank[3],spitch,read_buf[3]);

    comb_temp=0;

    for (int i = 0; i < 16; i++)
    {
      comb_temp+=out[i];
    }

    sample_out=comb_temp/3;

  }

  post_mod_f=hard_limit(post_mod);

  dly_out=vdelay(post_mod_f, lerp_dly ,right_pot);
  
//  testj+=8;
// testj%=4000;
  analogWrite(A14,dly_out+2048);


//  dds_t=micros()-dds_d;
}






/////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////


int vdelay(int input, uint16_t length, uint16_t fb_amt){
  int fb_amt_2=(fb_amt*294)>>8;
  int fb_inv=(fb_amt-255)*-1;


  write_head++;

  if (write_head > (NUM_SAMPS-1)){
    write_head=0;
  }


  read_head = ((write_head) + length);

  if (read_head > (NUM_SAMPS-1)){
    read_head-=(NUM_SAMPS-1);
  }


  filter2=(feedback+filter2)>>1;

  to_dly_temp= (((input*fb_inv)>>8) + ((filter2*fb_amt_2)>>8));

  //fold
  if(to_dly_temp<-4000){   
    int fold_amt=(to_dly_temp+4000)<<1;
    to_dly_temp-=fold_amt;
  } 
  if(to_dly_temp>4000){
    int fold_amt=(to_dly_temp-4000)<<1;
    to_dly_temp-=fold_amt;
  } 


  dly_buffer[write_head] =to_dly_temp;

  int read_head1 =dly_buffer[read_head];

  feedback = (input + read_head1); //

  int out_temp = (feedback);  

  if (fb_amt<8){
    out_temp=input;
  }
  return (out_temp);

}




/////////////////////////////////////////////////////////////////////////////////////////////






void record(byte pad, int input){
  uint32_t max_length, offset;
  
  if (pad<=7){
   max_length= pad_len-64;
   offset=(pad*pad_len);
  
}
if (pad>7){
   max_length= (pad_len*4)-64; //change 2 to 4 to get >8 seconds record time
   offset=(pad*pad_len*4); //change 2 to 4 to get >8 seconds record time
}

poff=offset;


  if (rec_ready[pad]==1 && rec_trig[pad]==0 && sample_step==3){
    rec_happend=1;
    adc_buf[buf_c]= input;
    buf_c++;

    if (buf_c>=buf_len){
      // Serial.println("wwww");

      //WEb();
      wp=w_loc+offset;
      rec_locf= (w_loc+offset);
      flash.write16(rec_locf,adc_buf,buf_len);
      w_loc+=buf_len;
      s_len[pad]=w_loc;
      buf_c=0;

    }


    if (w_loc>max_length){ 
     // Serial.println("   max");

      ee_store(pad);
      wp=w_loc=buf_c=0;
      rec_ready[pad]=0;
      shift_b_en=1;
      sample_step=0;
      master_mode=0;


    }

  }


  if (rec_happend==1 && rec_trig[pad]==1){
    // Serial.println("   ohhi ");

    ee_store(pad);
    wp=w_loc=buf_c=0;
    rec_ready[pad]=0;
    shift_b_en=1;
    sample_step=0;
    master_mode=0;
    rec_happend=0;

  }

  if (rec_trig[pad]==0 && rec_ready[pad]==1){
    //   rec_ready[pad]=0;
    // Serial.println("   over ");

  }

}

/////////////////////////////////////////////////////////////////////////////////////////////





int16_t sample(byte place, byte pad, uint16_t pitch, int16_t* array){
  uint32_t offset;
  
if (pad<=7){
  //byte rev=1;
  offset=(pad*pad_len);
  
}
if (pad>7){
  //byte rev=1;
   offset=(pad*pad_len*4); //change 2 to 4 to get >8 seconds record time
  
}
  if (pad<100){

    if (play_trig[pad]==0){

      amp[pad]=0;
      s_loc[pad]=0; 
      real_loc[pad]=s_loc[pad]+offset;
      acc[pad]=0;
      dx[pad]=1; 
      out[pad]=0;
    }




    if (play_trig[pad]>0){

      if (rev==0){

        amp[pad]=1;
        acc[pad]+=pitch;  
        dx[pad]=(acc[pad] >> 7); 

        if ((dx[pad]>s_len[pad]-32)){

          if (play_trig[pad]==1)
          {
            s_loc[pad]=0; 
            play_trig[pad]=0;
            poly_remove(pad);
            acc[pad]=0;
            dx[pad]=0;
          }

          if (play_trig[pad]==2)
          {
            s_loc[pad]=0;
            acc[pad]=0;
            dx[pad]=0;
          }

        }

        if (tick==place){
          // s_loc[pad]+=dx[pad];

          s_loc[pad]=dx[pad];
          real_loc[pad]=(s_loc[pad]+offset);
          flash.read16(((s_loc[pad])+offset),array,ab_len);

        }

        byte windex=dx[pad]-s_loc[pad];
        if (windex<=ab_len){
          out[pad]=(array[windex]);

        }

      }





      if (rev==1){

        amp[pad]=1;
        //     acc[pad] += (((pitch)<<1)-128)*-1;
        acc[pad] += (pitch);
        dx[pad]=(acc[pad] >> 7); 

        dxflip[pad]=(s_len[pad]-ab_len-1)-dx[pad];



        if (tick==place){

          if (play_trig[pad]==1)
          {
          if ((dx[pad]>s_len[pad]-ab_len)){
            s_loc[pad]=0;
            play_trig[pad]=0;
            poly_remove(pad);
            acc[pad]=0;
            dx[pad]=0;
          }

          if ((dxflip[pad]<ab_len)){
            s_loc[pad]=0;
            play_trig[pad]=0;
            poly_remove(pad);
            acc[pad]=0;
            dx[pad]=1;
            dxflip[pad]=s_len[pad];
          }
          // s_loc[pad]=dx[pad];
          }

          if (play_trig[pad]==2)
          {
            if ((dxflip[pad]<ab_len)){
              s_loc[pad]=s_len[pad];
              acc[pad]=0;
              dx[pad]=1;
              dxflip[pad]=s_len[pad];
              }

          }


          s_loc[pad]=(dxflip[pad])-ab_len;
          if (s_loc[pad]<0){
            s_loc[pad]=0;
          }

          //  R(((s_loc[pad])+offset),array,ab_len);
          flash.read16(((s_loc[pad])+offset),array,ab_len);

        }
        int windex=dxflip[pad]-s_loc[pad]-1;
        // byte windex=dx[pad]-s_loc[pad];



        if (windex>=0 && windex<ab_len){
          out[pad]=(array[windex]);

        }

      }




    


  }
}
  return out[pad];
}

byte vbt(){

  byte total=voice_bank[0]+voice_bank[1]+voice_bank[2]+voice_bank[3]+voice_bank[4]+voice_bank[5]+voice_bank[6]+voice_bank[7];

  return total;

}




















