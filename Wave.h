/* Wave.h
** -- functions for manipulating 16-bit WAVE files (HW study)
** cs225 8/18
*/

#ifndef CS225_WAVE_H
#define CS225_WAVE_H


typedef struct _WaveData *WaveData;


// filter functions used in calls to 'waveFilter'
short waveCut6DB(short in, void *data);
short waveBoost3DB(short in, void *data);
unsigned short waveBoostData(float gain);
short waveBoost(short in, void *data);


// API functions
WaveData waveRead(const char *filename);

int waveChannelCount(WaveData w);

int waveSamplingRate(WaveData w);

void waveSetFilterData(WaveData w, void *vp);

void waveFilter(WaveData w, int channel, short (*filter)(short,void*));

void waveWrite(WaveData w, const char *filename);

void waveDestroy(WaveData w);


#endif

