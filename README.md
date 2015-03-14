ndns-demo
=========

This project aims to
---------------------
  - set up a local NDNS experimental environment including:
    - local NDNS name server daemon configured with testing zones and data
    - local testing NDN identities, i.e., namespace owner or content producer, whose certificates are issued by local NDNS instance
    - Other NDNS configuration: configuration file, trust anchor, db file, etc
  - demonstrate some apps integrating NDNS in its code, including:
    - query-ndns: fetch data from NDNS and validate it
    - cpu-sensor: get local CPU temperature and store it in NDNS


Set up Local Experimental Environment
--------------------------------------------------
- install NDNS and its hack feature which setup local expermental environment automatically.
    - git remote add hack https://github.com/shockjiang/ndns.git , this commands add new remote branch to existing repository; also git clone can create new repository
    - git pull hack
    - git checkout deploy
  - compile and install ndns and hack features: e.g., ./waf clean && ./waf, sudo ./waf install
    - hack version contains application called ndns-demo-data that builds zones, create testing identities and data
    - hack version contains application called ndns-shot that fetch Data by name (without version) and validate it according to specified validation configuration file
  - set up experimental environment automatically
    - sudo ndns-demo-data, and check the warning message very carefully since it may modify you existing data (NDN Identity or NDNS configuration).

Data that Local Experimental Environment contains:
----------------------------------------
  - NDN Identities: /, /ndn, /ndn/edu, /ndn/edu/ucla, /ndn/edu/ucla/alice, /ndn/edu/ucla/bob. Note that alice and bob represent normal individuals, and the others represent zones.
  - local test trust anchor: /usr/local/etc/ndns/anchors/root-local.cert
  - ndns daemon configuration: /usr/local/etc/ndns/ndns.conf, this configures 4 zones, root zone, zone /ndn, zone /ndn/edu, zone /ndn/edu/ucla
  - database file: /usr/local/var/ndns/ndns.db, the db file contains the certificate of all identities and referral of parent zone to children zones, and two test resource records (RRs) for individual identities: /ndn/edu/ucla/NDNS/alice/TXT and /ndn/edu/ucla/NDNS/bob/TXT
  - validator configuration file: /usr/local/etc/ndns/validator.conf, thie configuration specify a directory as trust anchor dir.
  - source code of demo apps to show how to integrating NDNS into applications.


Have Fun!
------------------------------------------
Now all is ready and you can just interact with NDNS after starting nfd and ndns daemon.
- start the ndns service
  - start nfd: sudo nfd-start
  - start ndns-daemo: sudo ndns-daemon
- Remotely interact with ndns
  - dig existing RR: ndns-dig /ndn/edu/ucla/alice -t TXT
  - store new RR: ndns-update /ndn/edu/ucla /alice -t LINK -o /att
  - dig new RR: ndns-dig /ndn/edu/ucla/alice -t LINK
- Zone administrator manipulates data in local database
  - add RR to db: ndns-add-rr /ndn/edu/ucla /alice TXT2 -t resp -c some-random-content
  - add RR stored in local file: ndns-add-rr-from-file -f ndn.edu.ucla.jack.cert
  - read RR from db: ndns-get-rr /ndn/edu/ucla /alice TXT2
  - remove RR from db: ndns-remove-rr /ndn/edu/ucla  /alice TXT2
  - list all zones in db: ndns-list-all-zones
  - list all record in a zone: ndns-list-zone /ndn/edu/ucla
