/* WaveTest.c
** -- simple driver for WAVE file functions
** cs225 8/18
*/


#include <stdio.h>
#include <math.h>
#include "Wave.h"


#define TWOPI 6.283185307


/* ring modulator filter data */
struct RingModulatorData {
  double argument;
  double argument_inc;
  float mix;
};

/* initialize ring modulator filter data */
void ringModulatorInit(struct RingModulatorData *rmd, float f, float R, float m) {
  rmd->argument = 0.0;
  rmd->argument_inc = TWOPI * f / R;
  rmd->mix = m;
}

/* ring modulator filter function */
short ringModulator(short v, void *vp) {
  struct RingModulatorData *rmd = (struct RingModulatorData*)vp;
  float x = (float)v,
        y = (1.0f-rmd->mix)*x + rmd->mix*(float)sin(rmd->argument)*x;
  rmd->argument += rmd->argument_inc;
  return (short)y;
}


int main(int argc, char *argv[]) {
  WaveData wave_data = NULL;
  struct RingModulatorData rmd;
  unsigned short bd;

  /* exactly one command line argument is expected */
  if (argc != 2) {
    printf("*** name of a 16-bit WAVE file is expected ***\n");
    return 0;
  }

  /* attempt to read the WAVE file */
  wave_data = waveRead(argv[1]);
  if (wave_data == NULL) {
    printf("*** not a valid 16-bit WAVE file ***\n");
    return -1;
  }

  /* filter channel 0 with ring modulator */
  ringModulatorInit(&rmd,350.0f,(float)waveSamplingRate(wave_data),0.5f);
  waveSetFilterData(wave_data,&rmd);
  waveFilter(wave_data,0,ringModulator);

  /* scale channel 0 by a factor of 1.2 (with clipping) */
  bd = waveBoostData(1.2f);
  waveSetFilterData(wave_data,&bd);
  waveFilter(wave_data,0,waveBoost);

  /* scale channel 0 by a factor of 0.5 */
  waveFilter(wave_data,0,waveCut6DB);

  /* if present, scale channel 1 by a factor of 1.41 (with clipping) */
  if (waveChannelCount(wave_data) == 2)
    waveFilter(wave_data,1,waveBoost3DB);

  // writes separate wav files depending on whether the file is mono or stereo
  if(waveChannelCount(wave_data) == 1)
    waveWrite(wave_data, "Output1.wav");
    
  else
    waveWrite(wave_data, "Output2.wav");
    

  waveDestroy(wave_data);
  return 0;
}

