Opus Ogg Packing
================

Reads connection log gendered by Discord and creates an Ogg file. Only uses specified SSRC. 
Specifications: https://wiki.xiph.org/OggOpus.

Contributions
-------------

https://github.com/xiph/libopusenc/blob/master/src/ogg_packer.h
https://github.com/xiph/libopusenc/blob/master/src/ogg_packer.c
https://github.com/xiph/libopusenc/blob/master/src/opus_header.h
https://github.com/xiph/libopusenc/blob/master/src/opus_header.c

Opus header is simplified to remove `opus` dependencies. 

Build
-----

    make

Run
---
Sampling rate, number of channels and audio SSRC are hard-coded.

    ./converter connection_log out.ogg

https://rameshvarun.github.io/binary-inspector/ogg/

Issues
------
- Needs to write a special end stream packet (silence)

