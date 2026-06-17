#pragma once

#include "audio_decoder.h"

#ifdef __cplusplus
extern "C" {
#endif

audio_decoder_t *mp3_decoder_stub_open(const char *path);

#ifdef __cplusplus
}
#endif
