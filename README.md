[![Deploy](https://www.herokucdn.com/deploy/button.svg)](https://heroku.com/deploy)

[![Build Status](https://travis-ci.org/Credits-CRDS/Credits.png?branch=master)](https://travis-ci.org/Credits-CRDS/Credits)
[![Stories in Ready](https://badge.waffle.io/Credits-CRDS/Credits.png?label=ready&title=Ready)](https://waffle.io/Credits-CRDS/Credits)

Graph on Pull Request History
====================================

[![Throughput Graph](https://graphs.waffle.io/Credits-CRDS/Credits/throughput.svg)](https://waffle.io/Credits-CRDS/Credits/metrics/throughput)  

# **Credits (CRDS) v1.0.0.0**

![CRDS logo](https://github.com/Credits-CRDS/Credits/blob/master/src/qt/res/icons/light/about.png)

**Copyright (c) 2017 Credits Developers**

What is Credits?
----------------
* Coin Suffix: CRDS
* PoW Mining Algorithm: Argon2d
* PoW Difficulty Algorithm: DELTA
* PoW Period: ~36 years
* PoW Target Spacing: 128 Seconds
* PoW Reward per Block: 1 CRDS
* Maturity: 10 Blocks
* PoW Blocks: ~675 per day
* Masternode Collateral Amount: 500 CRDS
* Masternode Min Confirmation: 10 Blocks
* Masternode Reward: See Below
* Total Coins: 95,000,000 (~36 Years)
* Min TX Fee: 0.0001 CRDS

Credits uses peer-to-peer technology to operate securly with no central authority (decentralisation): managing transactions and issuing currency (CRDS) are carried out collectively by the Credits network. Credits is the name of open source software which enables the use of the currency CRDS.

Credits utilises Masternodes, Privatesend and InstantSend to provide anonymous and near instant transaction confirmations.

Credits implements Gavin Andresens signature cache optimisation from Bitcoin for significantly faster transaction validation.


**Masternode/Privatesend Network Information**
Utilisation of InstantSend for near-instant transactions and PrivateSend for anonymous transactions.

**MainNet Parameters**
P2P Port = 31000
RPC Port = 31050
Masternodes = 31000
Magic Bytes: 0x2f 0x32 0x45 0x51

**TestNet Parameters**
P2P Port = 31400
RPC Port = 31450
Masternodes = 31400
Magic Bytes: 0x1f 0x22 0x05 0x30

**RegTest Parameters**
P2P Port = 31500
RPC Port = 31550
Masternodes = 31500
Magic Bytes = 0x1f 0x22 0x05 0x2f

**Rewards Structure**

 Years|      Blocks        |   PoW  | Masternodes |
------|--------------------|--------|-------------|
 0-1  |       0 -  246544  | 10CRDS |    1CRDS    |
 1-2  |  246545 -  493088  | 10CRDS |    1CRDS    |
 2-3  |  493089 -  739631  |  9CRDS |    2CRDS    |
 3-4  |  739632 -  986175  |  9CRDS |    2CRDS    |
 4-5  |  986176 - 1232719  |  8CRDS |    3CRDS    |
 5-6  | 1232720 - 1479263  |  8CRDS |    3CRDS    |
 6-7  | 1479264 - 1725806  |  7CRDS |    4CRDS    |
 7-8  | 1725807 - 1972350  |  7CRDS |    4CRDS    |
 8-9  | 1972351 - 2218894  |  6CRDS |    5CRDS    |
 9-10 | 2218895 - 2465438  |  6CRDS |    5CRDS    |
10-11 | 2465439 - 2711981  |  5CRDS |    6CRDS    |
11-12 | 2711982 - 2958525  |  5CRDS |    6CRDS    |
12-13 | 2958526 - 3205069  |  4CRDS |    7CRDS    |
13-14 | 3205070 - 3451613  |  3CRDS |    7CRDS    |
14-15 | 3451614 - 3698156  |  3CRDS |    8CRDS    |
15-16 | 3698156 - 3944700  |  2CRDS |    8CRDS    |
16-17 | 3944701 - 4191244  |  2CRDS |    9CRDS    |
17-18 | 4191245 - 4437788  |  1CRDS |    9CRDS    |
18-19 | 4437789 - 4684331  |  1CRDS |   10CRDS    |
19-20 | 4684332 - 4930875  |  1CRDS |   10CRDS    |
20-21 | 4930876 - 5177419  |  1CRDS |   10CRDS    |
21-22 | 5177420 - 5423963  |  1CRDS |   10CRDS    |
22-23 | 5423964 - 5670506  |  1CRDS |   10CRDS    |
23-24 | 5670507 - 5917050  |  1CRDS |   10CRDS    |
24-25 | 5917051 - 6163594  |  1CRDS |   10CRDS    |
25-26 | 6163595 - 6410138  |  1CRDS |   10CRDS    |
26-27 | 6410139 - 6656681  |  1CRDS |   10CRDS    |
27-28 | 6656682 - 6903225  |  1CRDS |   10CRDS    |
28-30 | 6903226 - 7149769  |  1CRDS |   10CRDS    |   
30-31 | 7149770 - 7396313  |  1CRDS |   10CRDS    |
31-32 | 7396314 - 7642856  |  1CRDS |   10CRDS    |
32-33 | 7642857 - 7889400  |  1CRDS |   10CRDS    |
33-34 | 7889401 - 8135944  |  1CRDS |   10CRDS    |
34-35 | 8135945 - 8382488  |  1CRDS |   10CRDS    |
35-36 | 8382489 - 8629031  |  1CRDS |   10CRDS    |

UNIX BUILD NOTES
====================
Some notes on how to build Credits in Unix. 

Note
---------------------
Always use absolute paths to configure and compile Credits and the dependencies,
for example, when specifying the the path of the dependency:

    ../dist/configure --enable-cxx --disable-shared --with-pic --prefix=$BDB_PREFIX

Here BDB_PREFIX must absolute path - it is defined using $(pwd) which ensures
the usage of the absolute path.

To Build
---------------------

```bash
./autogen.sh
./configure
make
make install # optional
```

This will build credits-qt as well if the dependencies are met.

Dependencies
---------------------

These dependencies are required:

 Library     | Purpose          | Description
 ------------|------------------|----------------------
 libssl      | SSL Support      | Secure communications
 libboost    | Boost            | C++ Library
 libevent    | Networking       | OS independent asynchronous networking
 
Optional dependencies:

 Library     | Purpose          | Description
 ------------|------------------|----------------------
 miniupnpc   | UPnP Support     | Firewall-jumping support
 libdb4.8    | Berkeley DB      | Wallet storage (only needed when wallet enabled)
 qt          | GUI              | GUI toolkit (only needed when GUI enabled)
 protobuf    | Payments in GUI  | Data interchange format used for payment protocol (only needed when GUI enabled)
 libqrencode | QR codes in GUI  | Optional for generating QR codes (only needed when GUI enabled)
 libzmq3     | ZMQ notification | Optional, allows generating ZMQ notifications (requires ZMQ version >= 4.x)
 
For the versions used in the release, see [release-process.md](release-process.md) under *Fetch and build inputs*.

System requirements
--------------------

C++ compilers are memory-hungry. It is recommended to have at least 3 GB of
memory available when compiling Credits.

Dependency Build Instructions: Ubuntu & Debian
----------------------------------------------
Build requirements:

    sudo apt-get install build-essential libtool automake autotools-dev autoconf pkg-config libssl-dev libcrypto++-dev libevent-dev git
    
for Ubuntu 12.04 and later or Debian 7 and later libboost-all-dev has to be installed:

    sudo apt-get install libboost-all-dev

 db4.8 packages are available [here](https://launchpad.net/~bitcoin/+archive/bitcoin).
 You can add the repository using the following command:

        sudo add-apt-repository ppa:bitcoin/bitcoin
        sudo apt-get update

 Ubuntu 12.04 and later have packages for libdb5.1-dev and libdb5.1++-dev,
 but using these will break binary wallet compatibility, and is not recommended.

for Debian 7 (Wheezy) and later:
 The oldstable repository contains db4.8 packages.
 Add the following line to /etc/apt/sources.list,
 replacing [mirror] with any official debian mirror.

    deb http://[mirror]/debian/ oldstable main

To enable the change run

    sudo apt-get update

for other Debian & Ubuntu (with ppa):

    sudo apt-get install libdb4.8-dev libdb4.8++-dev

Optional (see --with-miniupnpc and --enable-upnp-default):

    sudo apt-get install libminiupnpc-dev

ZMQ dependencies (provides ZMQ API 4.x):

        sudo apt-get install libzmq3-dev
    
Dependencies for the GUI: Ubuntu & Debian
-----------------------------------------

If you want to build Credits-Qt, make sure that the required packages for Qt development
are installed. Qt 5 is necessary to build the GUI.
If both Qt 4 and Qt 5 are installed, Qt 5 will be used. Pass `--with-gui=qt5` to configure to choose Qt5.
To build without GUI pass `--without-gui`.

For Qt 5 you need the following:

    sudo apt-get install libqt5gui5 libqt5core5a libqt5dbus5 qttools5-dev qttools5-dev-tools libprotobuf-dev protobuf-compiler libcrypto++-dev

libqrencode (optional) can be installed with:

    sudo apt-get install libqrencode-dev
    
Once these are installed, they will be found by configure and a credits-qt executable will be
built by default.

Notes
-----
The release is built with GCC and then "strip creditsd" to strip the debug
symbols, which reduces the executable size by about 90%.

miniupnpc
---------

[miniupnpc](http://miniupnp.free.fr/) may be used for UPnP port mapping.  It can be downloaded from [here](
http://miniupnp.tuxfamily.org/files/).  UPnP support is compiled in and
turned off by default.  See the configure options for upnp behavior desired:

    --without-miniupnpc      No UPnP support miniupnp not required
    --disable-upnp-default   (the default) UPnP support turned off by default at runtime
    --enable-upnp-default    UPnP support turned on by default at runtime

To build:

    tar -xzvf miniupnpc-1.6.tar.gz
    cd miniupnpc-1.6
    make
    sudo su
    make install

Berkeley DB
-----------
It is recommended to use Berkeley DB 4.8. If you have to build it yourself:

```bash
CREDITS_ROOT=$(pwd)

# Pick some path to install BDB to, here we create a directory within the credits directory
BDB_PREFIX="${CREDITS_ROOT}/db4"
mkdir -p $BDB_PREFIX

# Fetch the source and verify that it is not tampered with
wget 'http://download.oracle.com/berkeley-db/db-4.8.30.NC.tar.gz'
echo '12edc0df75bf9abd7f82f821795bcee50f42cb2e5f76a6a281b85732798364ef  db-4.8.30.NC.tar.gz' | sha256sum -c
# -> db-4.8.30.NC.tar.gz: OK
tar -xzvf db-4.8.30.NC.tar.gz

# Build the library and install to our prefix
cd db-4.8.30.NC/build_unix/
#  Note: Do a static build so that it can be embedded into the exectuable, instead of having to find a .so at runtime
../dist/configure --prefix=/usr/local --enable-cxx
make 
sudo make install

# Configure Credits to use our own-built instance of BDB
cd $CREDITS_ROOT
./configure (other args...) LDFLAGS="-L${BDB_PREFIX}/lib/" CPPFLAGS="-I${BDB_PREFIX}/include/"
```

**Note**: You only need Berkeley DB if the wallet is enabled (see the section *Disable-Wallet mode* below).

Boost
-----
If you need to build Boost yourself:

    sudo su
    ./bootstrap.sh
    ./bjam install


Security
--------
To help make your Credits installation more secure by making certain attacks impossible to
exploit even if a vulnerability is found, binaries are hardened by default.
This can be disabled with:

Hardening Flags:

    ./configure --enable-hardening
    ./configure --disable-hardening


Hardening enables the following features:

* Position Independent Executable
    Build position independent code to take advantage of Address Space Layout Randomization
    offered by some kernels. An attacker who is able to cause execution of code at an arbitrary
    memory location is thwarted if he doesn't know where anything useful is located.
    The stack and heap are randomly located by default but this allows the code section to be
    randomly located as well.

    On an Amd64 processor where a library was not compiled with -fPIC, this will cause an error
    such as: "relocation R_X86_64_32 against `......' can not be used when making a shared object;"

    To test that you have built PIE executable, install scanelf, part of paxutils, and use:

        scanelf -e ./creditsd

    The output should contain:
     TYPE
    ET_DYN

* Non-executable Stack
    If the stack is executable then trivial stack based buffer overflow exploits are possible if
    vulnerable buffers are found. By default, credits should be built with a non-executable stack
    but if one of the libraries it uses asks for an executable stack or someone makes a mistake
    and uses a compiler extension which requires an executable stack, it will silently build an
    executable without the non-executable stack protection.

    To verify that the stack is non-executable after compiling use:
    `scanelf -e ./creditsd`

    the output should contain:
    STK/REL/PTL
    RW- R-- RW-

    The STK RW- means that the stack is readable and writeable but not executable.

Disable-wallet mode
--------------------
When the intention is to run only a P2P node without a wallet, credits may be compiled in
disable-wallet mode with:

    ./configure --disable-wallet

In this case there is no dependency on Berkeley DB 4.8.

Mining is also possible in disable-wallet mode, but only using the `getblocktemplate` RPC call.

AVX2 Mining Optimisations
-------------------------
For increased performance when mining, AVX2 optimisations can be enabled. 

Prior to running the build commands:

    CPPFLAGS=-march=native
    
CPU's with AVX2 support:

    Intel
        Haswell processor, Q2 2013
        Haswell E processor, Q3 2014
        Broadwell processor, Q4 2014
        Broadwell E processor, Q3 2016
        Skylake processor, Q3 2015
        Kaby Lake processor, Q3 2016(ULV mobile)/Q1 2017(desktop/mobile)
        Coffee Lake processor, expected in 2017
        Cannonlake processor, expected in 2017
    AMD
        Carrizo processor, Q2 2015
        Ryzen processor, Q1 2017

Example Build Command
--------------------
Qt Wallet and Deamon, CLI version build:

    ./autogen.sh && ./configure --with-gui && make

CLI and Deamon Only Buld:

    ./autogen.sh && ./configure --without-gui && make
