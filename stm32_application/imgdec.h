
#ifndef IMGDEC__HH__
#define IMGDEC__HH__

typedef enum { ConsumeNext, ProduceCopy, ProduceFill } FSM_states;

typedef struct {
	const unsigned char * ibuf; // Input buffer
	unsigned outptr, counter;
	FSM_states fsm;
	unsigned char last;
} img_decoder;


void init_decoder(img_decoder * dec, const unsigned char * buffer);
unsigned char decode_sample(img_decoder * dec);

void image_decode(const unsigned char * in, unsigned char * out);

#endif

