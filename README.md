ndns-demo
=========

This project aims to
---------------------
  - help developpers to set up a local NDNS experimental environment
    - testing identities
    - testing zone and its data
    - testing configuration: configuration file, trust anchor, db file
  - demonstrate some apps that integrating NDNS in its code
    - query-ndns: fetch RR from NDNS and validate it
    - cpu-sensor: get local CPU temperature and store it in NDNS


Set up Local Experimental Environment
--------------------------------------------------
- install NDNS and its hack feature for local expermental enviroment.
  - the hack feature is here: https://github.com/shockjiang/ndns/tree/deploy. users can add a new remote branch and checkout the deploy branch following the instructions here
    - git remote add hack https://github.com/shockjiang/ndns.git for existing local repository or clone to create new repository
    - git pull hack
    - git checkout deploy
  - compile and install ndns and hack features: e.g., ./waf clean && ./waf, sudo ./waf install
    - hack version contains application called ndns-demo-data that build zones locally and add RRs as testing data)
    - hack version contains application called ndns-shot that fetch Data by name (without version) and validate it according to specified validation configuration file
  - set up experimental environment automatically
    - sudo ndns-demo-data, and check the warning message very carefully since it may modify you existing data (NDN Identity or NDNS configuration).

Local Experimental Environment contains:
----------------------------------------
  - NDN Identities: /, /ndn, /ndn/edu, /ndn/edu/ucla, /ndn/edu/ucla/alice, /ndn/edu/ucla/bob. Note that alice and bob are normal individual identity, and the others are zone identity.
  - local test trust anchor: /usr/local/etc/ndns/anchors/root-local.cert
  - ndns daemon configuration: /usr/local/etc/ndns/ndns.conf, this configures 4 zones, root zone, zone /ndn, zone /ndn/edu, zone /ndn/edu/ucla
  - database file: /usr/local/var/ndns/ndns.db, the db file contains the certificate of all identities and referral of parent zone to children zones, and two test RRs for individual identities: /ndn/edu/ucla/NDNS/alice/TXT and /ndn/edu/ucla/NDNS/bob/TXT
  - validator configuration file: /usr/local/etc/ndns/validator.conf, thie configuration specify a directory as trust anchor dir.
  - source code of demo apps to show how to integrating NDNS into applications.
  

Have Fun!
------------------------------------------
- start nfd: sudo nfd-start
- start ndns-daemo: sudo ndns-daemon
- try to interact with ndns
  - dig existing RR: ndns-dig /ndn/edu/ucla/alice -t TXT 
  - store new RR: ndns-update /ndn/edu/ucla /alice -t LINK -o /att
  - dig new RR: ndns-dig /ndn/edu/ucla/alice -t LINK
