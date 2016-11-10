
from compression import compress

im = [
	# Image goes here :)
]

tftable = [3,2,1,0]

res = []
for i in range(0, len(im), 4):
	n = tftable[im[i+0]]<<6 | tftable[im[i+1]]<<4 | tftable[im[i+2]]<<2 | tftable[im[i+3]]

	res.append(n)

res = compress(res)
nbytes = len(res)

res = [ "%d," % n for n in res ]

res = [ " ".join(res[i:i+16]) for i in range(0, len(res), 16) ]

print 'const unsigned char varname[%d] __attribute__((section(".text"), used)) = {' % nbytes
print "\n".join(res)
print "};"



