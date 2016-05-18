
// Using this we compress images into 10% of the original size
// cause most of them are black/white and very blocky :D

#include <string.h>

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


