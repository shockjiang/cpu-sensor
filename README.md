ndns-demo
=========

This project demonstrates some NDNS use case implementation
- query-ndns: fetch RR from NDNS and validate it
- cpu-sensor: get local CPU temperature and store it in NDNS

Dependency
----------
- ndn-cxx
- NFD
- NDNS system (hosted locally or on NDN Testbed)


Install NDNS to set up local testing environment
--------------------------------------------------
- install NDNS ( we suggest to use version here: https://github.com/shockjiang/ndns/tree/deploy, then do the following configuration
  - contain application called ndns-demo-data that build zones locally and add RRs as testing data)
  - contain application called ndns-shot that fetch Data by name (without version) and validate it according to specified validation configuration file


1. create validator configuration based on %s/validator.conf.sample %(dir), here dir is /usr/local/etc/ndns by default
   - e.g.: cp %s/validator.conf.sample %s/validator.conf" %(dir, dir)

2. download root certificate of NDN Testbed to %s/anchors/root.cert %(dir)
   - e.g.: wget http://named-data.net/ndnsec/ndn-testbed-root.ndncert.base64 -O %s/anchors/root.cert" %(dir)

3. create log configuration based %s/log4cxx.properties.sample %(dir)
   - e.g.: cp %s/log4cxx.properties.sample %s/log4cxx.properties" %(dir, dir)

By now, your NDNS tools, including ndns-dig, ndns-shot could work, but we suggest to allow multiple trust anchor for developing/debugging convenience

4. create some zones locally ndns database, and create identities, create a local root cert and revise validator configuration

   - e.g.: sudo ndns-demo-data

Install Demo's validator configuration file
-------------------------------------------

after compile the ndns-demo

    - e.g.: cp ./build/validator-demo.conf %/validator-demo.conf %(dir)
