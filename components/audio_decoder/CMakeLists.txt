idf_component_register(
    SRCS
        "audio_decoder.c"
        "wav_decoder.c"
        "mp3_decoder_stub.c"
    INCLUDE_DIRS "include"
    REQUIRES fatfs
)
