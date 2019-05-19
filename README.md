# ImageProcessing
## Environment
<pre>
$ cat /etc/os-release
NAME="Ubuntu"
VERSION="18.04.2 LTS (Bionic Beaver)"
ID=ubuntu
ID_LIKE=debian
PRETTY_NAME="Ubuntu 18.04.2 LTS"
VERSION_ID="18.04"

$ gcc --version
gcc (Ubuntu 7.4.0-1ubuntu1~18.04) 7.4.0
Copyright (C) 2017 Free Software Foundation, Inc.
This is free software; see the source for copying conditions.  There is NO
warranty; not even for MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
</pre>

## Init setting
<pre>
$ git clone https://github.com/kuboshiba/ImageProcessing.git
$ cd ImageProcessing
$ sudo mv libX11.so.6.3.0 /usr/lib/x86_64-linux-gnu/libX11.so.6.3.0
$ sudo mv libtiff.so.5.2.4 /usr/lib/x86_64-linux-gnu/libtiff.so.5.2.4
$ ln -s /usr/lib/x86_64-linux-gnu/libX11.so.6.3.0 ./lib/libX11.so
$ ln -s /usr/lib/x86_64-linux-gnu/libtiff.so.5.2.4 ./lib/libtiff.so
</pre>

## Execution
<pre>
$ make [target]
</pre>
