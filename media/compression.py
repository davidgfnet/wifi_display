
# RLE compression:
# Chunk header is one byte, decoded means:
#  0XXX XXXX: The following byte is repeated XXXXXXX times + 1 (from 1 to 128)
#  1XXX XXXX: Just copy the following XXXXXXX+1 bytes (means the pattern is not compressible)

def countb(buf):
	ref = buf[0]
	ret = 0
	while ret < len(buf) and buf[ret] == ref:
		ret += 1

	return ret

def compress(buf):
	outb = []

	i = 0
	accum = []
	while (i < len(buf)):
		bytec = min(countb(buf[i:]), 128)
		encoderle = (bytec > 3)

		if encoderle or len(accum) == 128:
			# Flush noncompressable pattern
			if len(accum) > 0:
				b = len(accum) - 1
				b |= 0x80
				outb.append(b)
				outb += accum
				accum = []

		if encoderle:
			# Emit a runlegth
			outb.append(bytec-1)
			outb.append(buf[i])
			i += bytec
		else:
			accum.append(buf[i])
			i += 1

	# Make sure to flush it all
	outb += accum

	return outb

