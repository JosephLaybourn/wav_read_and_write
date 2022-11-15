/* Wave.template.c
** -- template for homework assignment #1
** cs225
*/

#define MAX_INT16 32767 

#include "Wave.h"
#include "WaveSolution.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

struct _WaveData 
{
  int channel_count;
  int frame_count;
  int sampling_rate;
  void *filter_data;
  short *data16;
};


short waveCut6DB(short in, void *data) 
{
  return in >> 1;
}


short waveBoost3DB(short in, void *data) 
{
  int ix = (int)in;
  
  
  /* compute 1.41*ix */ 
  
  int divide4 = ix >> 2;
  int divide8 = ix >> 3;
  int divide32 = ix >> 5;

  ix += divide4 + divide8 + divide32;

  if (ix > MAX_INT16)
   ix = MAX_INT16; 

  else if (ix < -MAX_INT16)
    ix = -MAX_INT16;
  
  /* clamp to MIN_INT16 as well */ 
  return (short)ix;
}


unsigned short waveBoostData(float gain) 
{
  int i;
  unsigned short bd = 0,
                 mask = 0x01;
  for (i=0; i < 16; ++i) {
    if (gain >= 1) {
      bd |= mask;
      gain -= 1.0f;
    }
    mask <<= 1;
    gain *= 2.0f;
  }
  return bd;
}


short waveBoost(short in, void *data) 
{
  unsigned short n = *((unsigned short *)data);
  int out = 0;
  
  for (int i = 0; i < 16; i++)
  {
    // if the least significant bit on n is 1
    if (n & 0x0001)
    {
      /* creates a temporary copy to right shift, 
       * then add AKA Pseudo multiplication       */
      int temp = in >> i;
      out += temp;

      // Clamps the result so there's no foldback distortion
      if (out > MAX_INT16)
        out = MAX_INT16;

      else if (out < -MAX_INT16)
        out = -MAX_INT16;
    }
    // shifts n to check the next bit
    n >>= 1;
  }

  return (short)out;
}



WaveData waveRead(const char *fname) 
{
  FILE *fp = fopen(fname, "rb");
  WaveData data = (WaveData)calloc(1, sizeof(struct _WaveData));
  int size;
  
  if(!fp)
  {
    fclose(fp);
    return NULL;
  }

  // tests the riff section
  const char riff_test[] = "RIFF";
  char riff_input[5]; /* The 4 characters plus the NULL terminator */
  fread(&riff_input, sizeof(char), 4, fp); 
  
  // gets the riff size to test once we get the data size
  unsigned riff_size;
  fseek(fp, 4, SEEK_SET);
  fread(&riff_size, sizeof(unsigned), 1, fp);
  if(strcmp(riff_test, riff_input))
  {
    fclose(fp);
    printf("stop1");
    return NULL;
  }
  
  // tests the WAVE and fmt chars at the same time
  const char wave_fmt_test[] = "WAVEfmt ";
  char wave_fmt_input[12];
  fseek(fp, 8, SEEK_SET);
  fread(&wave_fmt_input, sizeof(char), 8, fp);
  if(strcmp(wave_fmt_test, wave_fmt_input))
  {
    fclose(fp);
    printf("%s\n", wave_fmt_input);
    return NULL;
  }
  
  // checks the fmt size (should be 16)
  unsigned fmt_size;
  fseek(fp, 16, SEEK_SET);
  fread(&fmt_size, sizeof(unsigned), 1, fp);
  if(fmt_size != 16)
  {
    fclose(fp);
    printf("stop3");
    return NULL;
  }
  
  // checks the audio format (should be 1)
  unsigned short audio_format;
  fseek(fp, 20, SEEK_SET);
  fread(&audio_format, sizeof(unsigned short), 1, fp);
  if(audio_format != 1)
  {
    fclose(fp);
    printf("stop4");
    return NULL;
  }
  
  // copies the channel count info to the wave data struct
  fseek(fp, 22, SEEK_SET);
  fread(&(data->channel_count), sizeof(unsigned short), 1, fp);

  // copies the sampling rate info
  fseek(fp, 24, SEEK_SET);
  fread(&(data->sampling_rate), sizeof(int), 1, fp);
  
  // checks the bits per sample (should be 16)
  unsigned short bits_per_sample;
  fseek(fp, 34, SEEK_SET);
  fread(&bits_per_sample, sizeof(unsigned short), 1, fp);
  if(bits_per_sample != 16)
  {
    fclose(fp);
    printf("stop5");
    return NULL;
  }
  
  // checks the data label
  const char data_label_test[] = "data";
  char data_label_input[5];
  fseek(fp, 36, SEEK_SET);
  fread(&data_label_input, sizeof(char), 4, fp);
  if(strcmp(data_label_test, data_label_input))
  {
    fclose(fp);
    return NULL;
  }

  // finds the amount of data contained in the samples
  fseek(fp, 40, SEEK_SET);
  fread(&size, sizeof(int), 1, fp);
  
  /* decides to record the frame count for mono or stereo audio
   * frame count for mono is just the amount of samples
   * frame count for stereo is the amount of left/right pairs of samples */
  data->frame_count = size / (data->channel_count * sizeof(short));
  
  /* dynamically allocates the amount of data needed to hold all the samples
   * then copies all the sample data into our wave data struct               */
  short *waveform = (short *)malloc(size);
  fseek(fp, 44, 40);
  fread(waveform, sizeof(short), data->frame_count * data->channel_count, fp);
  data->data16 = waveform;

  // debug code
  //printf("Channel Count: %d Sampling Rate: %d\n", data->channel_count, data->sampling_rate);
  //printf("Wave Input Data Amount: %d ", size);
  
  fclose(fp);
  return data;
}


int waveChannelCount(WaveData w) 
{
  return w->channel_count;
}


int waveSamplingRate(WaveData w) 
{
  return w->sampling_rate;
}


void waveSetFilterData(WaveData w, void *vp) 
{
  w->filter_data = vp;
}


void waveFilter(WaveData w, int channel, short (*filter)(short,void*)) 
{
  short *tempData = w->data16;
  
  // cycles through all the samples, applying the filter to each sample
  for(int i = 0; i < w->frame_count; i++)
  {
    // selects which channel to apply the filter on
    if(w->channel_count == 2)
    {
      if(channel == 1)
      {
        tempData++;
        *tempData = filter(*tempData, w->filter_data);
      }
      else
      {
        *tempData = filter(*tempData, w->filter_data);
        tempData++;
      }
    }
    // runs if wave file is mono
    else
    {
      *tempData = filter(*tempData, w->filter_data);
    }
    tempData++;
  }
}


void waveWrite(WaveData w, const char *fname) 
{
  FILE *fp = fopen(fname, "wb");
  
  if(!fp)
  {
    return;
  }
  
  // creates a wave header sample
  const char riff_label[] = "RIFF";
  const unsigned riff_size = 36 + ((w->frame_count * w->channel_count) * sizeof(short));
  const char file_tag[] = "WAVE";
  const char fmt_label[] = "fmt ";
  const unsigned fmt_size = 16;
  const unsigned short audio_format = 1;
  const unsigned short channel_count = w->channel_count;
  const unsigned sample_rate = w->sampling_rate;
  const unsigned bytes_per_second = w->sampling_rate * (sizeof(short) * channel_count); 
  const unsigned short bytes_per_sample = sizeof(short) * channel_count;
  const unsigned short bits_per_sample = sizeof(short) * 8;
  const char data_label[] = "data";
  const unsigned data_size = (w->frame_count * w->channel_count) * sizeof(short);
  
  // writes all the wave header data to a new wave file
  fwrite(&riff_label, 1, sizeof(riff_label), fp);
  fwrite(&riff_size, 1, sizeof(unsigned), fp);
  fwrite(&file_tag, 1, sizeof(file_tag), fp);
  fwrite(&fmt_label, 1, sizeof(fmt_label), fp);
  fwrite(&fmt_size, 1, sizeof(unsigned), fp);
  fwrite(&audio_format, 1, sizeof(unsigned short), fp);
  fwrite(&channel_count, 1, sizeof(unsigned short), fp);
  fwrite(&sample_rate, 1, sizeof(unsigned), fp);
  fwrite(&bytes_per_second, 1, sizeof(unsigned), fp);
  fwrite(&bytes_per_sample, 1, sizeof(unsigned short), fp);
  fwrite(&bits_per_sample, 1, sizeof(unsigned short), fp);
  fwrite(&data_label, 1, sizeof(data_label), fp);
  fwrite(&data_size, 1, sizeof(unsigned), fp);
  
  // debug code
  //printf("Audio output data amount: %d ", data_size);
  
  // writes all the samples to the wave file
  fwrite(w->data16, sizeof(short), w->frame_count * w->channel_count, fp);
  
  fclose(fp);
}


void waveDestroy(WaveData w) 
{
  free(w->data16);
  free(w);
}

