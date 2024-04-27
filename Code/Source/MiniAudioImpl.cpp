#define MINIAUDIO_IMPLEMENTATION

#include "miniaudio.h"
#include "phonon.h" /* Steam Audio */

#define FORMAT ma_format_f32 /* Must be floating point. */
#define CHANNELS 2 /* Must be stereo for this example. */
#define SAMPLE_RATE 48000
