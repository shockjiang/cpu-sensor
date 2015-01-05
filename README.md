ndns-demo
=========

This project demonstrates some NDNS use case implementation

dependency:
- ndn-cxx
- NFD
- NDNS ( we suggest to use version here: https://github.com/shockjiang/ndns/tree/deploy, which contains a ndns-demo that build zones locally and add RRs as testing data)

* Note that, NDNS need manully configuration, with following ways:

1) create validator configuration based on %s/validator.conf.sample: " %(dir)
  e.g.: cp %s/validator.conf.sample %s/validator.conf" %(dir, dir)

2) download root certificate of NDN Testbed to %s/anchors/root.cert" %(dir)
  e.g.: wget http://named-data.net/ndnsec/ndn-testbed-root.ndncert.base64 -O %s/anchors/root.cert" %(dir)

3) (optional): create log configuration based %s/log4cxx.properties.sample" %(dir)
  e.g.: cp %s/log4cxx.properties.sample %s/log4cxx.properties" %(dir, dir)

We also suggest to add create some zones locally as testing data:

4) (optional): creat zones locally and add RRs (need to configure validator.conf and ndns.conf)

    * Step 1: add the zones to your ndns.conf (see the following example)
    * Step 2: change to trust anchor to anchors directory (see the following example) in order not to affect normal NDNS.
    * Step 3: run command: sudo ndns-demo


ndns.conf example:
----------------
  zones
{
  dbFile /usr/local/var/ndns/ndns.db

  zone {
    name /
  }

  zone {
    name /ndn
  }

  zone {
    name /ndn/edu
  }

  zone {
    name /ndn/edu/ucla
  }
}

hints
{
  hint /ucla
  hint /att
}

validator.conf example
---------------------
rule
{
  id "NDNS Validator"
  for data
  checker
  {
    type customized
    sig-type rsa-sha256
    key-locator
    {
      type name
      hyper-relation
      {
        k-regex ^(<>*)<KEY>(<>*)<><ID-CERT>$
        k-expand \\1\\2
        h-relation is-prefix-of ; data is only allowed to be signed by the zone key
        p-regex ^(<>*)[<KEY><NDNS>](<>*)<><>$
        p-expand \\1\\2
      }
    }
  }
}

trust-anchor
{
  ; type file
  ; file-name anchors/root.cert
  type dir
  dir anchors
  refresh 1h
}
