steg-server
===========

Description
-----------
This is the server component of the HTTP Steganography project. 

This Apache module is meant to work together with the client component of the HTTP Steganography project (not yet implemented) to allow hidden client-server communication within the already established web channel, thus removing the need to work around firewall policies and hopefully evading intrusion detection systems. 

**The project is in a very early stage, and most functionality is not yet implemented.**

Quick Start
-----------
Make sure the 'httpd-devel' package for CentOS/RHEL or the equivalent package for your distro is installed. This package contains the header files and the 'apxs' tool necessary to compile this project.

To compile, just 'cd' into the cloned directory and run:
```
$ apxs -c -i -Iinclude mod_steg.c utils.c packet_builder.c config.c
```

Edit your httpd configuration file, e.g. /etc/httpd/conf/httpd.conf, and add a LoadModule directive:
```
LoadModule steg_module     modules/mod_steg.so
```

Finally, configure the Steg module. These are the module-specific directives:
* **stegInputFile**: Full path to the file where recevied information will be written.
* **stegOutputFile**: Full path to the file where response information will be read from.
* **stegKnockCode**: Code that will be used to recognize a steganogram.
* **stegMethod**: Steganography method that will be used. For the first prototype, only Header will be available.

Keep in mind that this module is implemented as two filters, so you need to use Apache's standard **setInputFilter** and **setOutputFilter** directives.


Example configuration:
```
<Directory "/var/www/html">
  setInputFilter StegInput
  stegEnabled on
  stegInputFile /var/steg/input
  stegOutputFile /var/steg/output
  stegKnockCode 098asdf234
  stegMethod Header Accept-Encoding
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
For the first prototype, only the simplest Steganography method is supported, thus a simple curl call will suffice:
```
$ curl -H "Accept-Encoding: gzip, deflate, <hidden message>"
```
For now, the format of the message is:
```
knockcode|length|payload
```

Where **knockcode** is just what you configured in Apache, **length** is a 3 byte field that indicates the length of the payload, and **payload** is any string. Example:
```
knock005hello
```









