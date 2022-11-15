/* waveSolution.h
** -- my C implementation of the waveSolution API
** cs225 8/18
*/

#ifndef CS225_WAVESOLUTION_H
#define CS225_WAVESOLUTION_H


typedef struct _WaveData *WaveData;


short waveSolutionCut6DB(short in, void *data);
short waveSolutionBoost3DB(short in, void *data);
short waveSolutionBoost(short in, void *data);


WaveData waveSolutionRead(const char *fname);

int waveSolutionChannelCount(WaveData w);

int waveSolutionSamplingRate(WaveData w);

void waveSolutionSetFilterData(WaveData w, void *vp);

void waveSolutionFilter(WaveData w, int channel, short (*filter)(short,void*));

void waveSolutionWrite(WaveData w, const char *filename);

void waveSolutionDestroy(WaveData w);


#endif

