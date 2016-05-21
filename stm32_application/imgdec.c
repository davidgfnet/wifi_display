
// Using this we compress images into 10% of the original size
// cause most of them are black/white and very blocky :D

#include <string.h>
#include "imgdec.h"

// Given an input buffer that encodes an RLE image, place the 
// decode output at out. We know that out is exactly 200*600 bytes
void image_decode(const unsigned char * in, unsigned char * out) {
	int outsize = 200*600;
	while (outsize > 0) {
		unsigned char h = *in++;
		int len = (h & 0x7F) + 1;

		if (h & 0x80) {
			memcpy(out, in, len);
			in += len;
		}
		else {
			memset(out, *in, len);
			in++;
		}

		out += len;
		outsize -= len;
	}
}

void init_decoder(img_decoder * dec, const unsigned char * buffer) {
	dec->ibuf = buffer;
	dec->outptr = 0;
	dec->fsm = ConsumeNext;
}


// Given an input buffer that encodes an RLE image, place the 
// decode output at out. We know that out is exactly 200*600 bytes
unsigned char decode_sample(img_decoder * dec) {
	if (dec->outptr >= 200*600)
		return 0;

	switch (dec->fsm) {
	case ConsumeNext: {
		unsigned char h = *dec->ibuf++;
		dec->counter = (h & 0x7F) + 1;
		if (h & 0x80) {
			// Copy next len bytes to output
			dec->fsm = ProduceCopy;
		}
		else {
			dec->fsm = ProduceFill;
			dec->last = *dec->ibuf++;
		}
		// This doesn't produce a thing but guarantees that
		// next call will produe one byte
		return decode_sample(dec);
	}; break;
	case ProduceCopy:
		if (--dec->counter == 0)
			dec->fsm = ConsumeNext;
		dec->outptr++;
		return *dec->ibuf++;
	case ProduceFill:
		if (--dec->counter == 0)
			dec->fsm = ConsumeNext;
		dec->outptr++;
		return dec->last;
	};
}



