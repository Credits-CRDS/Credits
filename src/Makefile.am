DIST_SUBDIRS = secp256k1 univalue

AM_LDFLAGS = $(PTHREAD_CFLAGS) $(LIBTOOL_LDFLAGS) $(HARDENED_LDFLAGS)
AM_CXXFLAGS = $(HARDENED_CXXFLAGS)
AM_CPPFLAGS = $(HARDENED_CPPFLAGS)
EXTRA_LIBRARIES =

CREDITS_CONFIG_INCLUDES=-I$(builddir)/config
CREDITS_INCLUDES=-I$(builddir) -I$(builddir)/obj $(BOOST_CPPFLAGS) $(LEVELDB_CPPFLAGS) $(CRYPTO_CFLAGS) $(SSL_CFLAGS)

CREDITS_INCLUDES += -I$(srcdir)/secp256k1/include
CREDITS_INCLUDES += -I$(srcdir)/univalue/include

LIBCREDITS_SERVER=libcredits_server.a
LIBCREDITS_WALLET=libcredits_wallet.a
LIBCREDITS_COMMON=libcredits_common.a
LIBCREDITS_CLI=libcredits_cli.a
LIBCREDITS_UTIL=libcredits_util.a
LIBCREDITS_CRYPTO=crypto/libcredits_crypto.a
LIBCREDITSQT=qt/libcreditsqt.a
LIBSECP256K1=secp256k1/libsecp256k1.la
LIBUNIVALUE=univalue/libunivalue.la

$(LIBSECP256K1): $(wildcard secp256k1/src/*) $(wildcard secp256k1/include/*)
	$(AM_V_at)$(MAKE) $(AM_MAKEFLAGS) -C $(@D) $(@F)

$(LIBUNIVALUE): $(wildcard univalue/lib/*) $(wildcard univalue/include/*)
	$(AM_V_at)$(MAKE) $(AM_MAKEFLAGS) -C $(@D) $(@F)

# Make is not made aware of per-object dependencies to avoid limiting building parallelization
# But to build the less dependent modules first, we manually select their order here:
EXTRA_LIBRARIES += \
  crypto/libcredits_crypto.a \
  libcredits_util.a \
  libcredits_common.a \
  libcredits_server.a \
  libcredits_cli.a
if ENABLE_WALLET
CREDITS_INCLUDES += $(BDB_CPPFLAGS)
EXTRA_LIBRARIES += libcredits_wallet.a
endif
if ENABLE_ZMQ
EXTRA_LIBRARIES += libcredits_zmq.a
endif

if BUILD_CREDITS_LIBS
lib_LTLIBRARIES = libcreditsconsensus.la
LIBCREDITS_CONSENSUS=libcreditsconsensus.la
else
LIBCREDITS_CONSENSUS=
endif

bin_PROGRAMS =
TESTS =
BENCHMARKS =

if BUILD_CREDITSD
  bin_PROGRAMS += creditsd
endif

if BUILD_CREDITS_UTILS
  bin_PROGRAMS += credits-cli credits-tx
endif

.PHONY: FORCE check-symbols check-security
# credits #
CREDITS_CORE_H = \
  activemasternode.h \
  addressindex.h \
  spentindex.h \
  addrman.h \
  alert.h \
  amount.h \
  arith_uint256.h \
  base58.h \
  bip39_english.h \
  bip39.h \
  bloom.h \
  cachemap.h \
  cachemultimap.h \
  chain.h \
  chainparams.h \
  chainparamsbase.h \
  chainparamsseeds.h \
  checkpoints.h \
  checkqueue.h \
  clientversion.h \
  coincontrol.h \
  coins.h \
  compat.h \
  compat/byteswap.h \
  compat/endian.h \
  compat/sanity.h \
  compressor.h \
  consensus/consensus.h \
  consensus/merkle.h \
  consensus/params.h \
  consensus/validation.h \
  core_io.h \
  core_memusage.h \
  dbwrapper.h \
  masternode.h \
  masternode-payments.h \
  masternode-sync.h \
  masternodeman.h \
  masternodeconfig.h \
  flat-database.h \
  governance.h \
  governance-classes.h \
  governance-exceptions.h \
  governance-object.h \
  governance-vote.h \
  governance-votedb.h \
  hash.h \
  hdchain.h \
  httprpc.h \
  httpserver.h \
  indirectmap.h \
  init.h \
  instantsend.h \
  key.h \
  keepass.h \
  keystore.h \
  limitedmap.h \
  main.h \
  memusage.h \
  merkleblock.h \
  messagesigner.h \
  miner.h \
  net.h \
  netbase.h \
  netfulfilledman.h \
  noui.h \
  policy/fees.h \
  policy/policy.h \
  policy/rbf.h \
  pow.h \
  prevector.h \
  primitives/block.h \
  primitives/transaction.h \
  privatesend.h \
  psnotificationinterface.h \
  privatesend-relay.h \
  protocol.h \
  pubkey.h \
  random.h \
  reverselock.h \
  rpcclient.h \
  rpcprotocol.h \
  rpcserver.h \
  scheduler.h \
  script/interpreter.h \
  script/script.h \
  script/script_error.h \
  script/sigcache.h \
  script/sign.h \
  script/standard.h \
  serialize.h \
  spork.h \
  streams.h \
  support/allocators/secure.h \
  support/allocators/zeroafterfree.h \
  support/cleanse.h \
  support/lockedpool.h \
  sync.h \
  threadsafety.h \
  timedata.h \
  tinyformat.h \
  torcontrol.h \
  txdb.h \
  txmempool.h \
  ui_interface.h \
  uint256.h \
  undo.h \
  util.h \
  utilmoneystr.h \
  utilstrencodings.h \
  utiltime.h \
  validationinterface.h \
  version.h \
  versionbits.h \
  wallet/crypter.h \
  wallet/db.h \
  wallet/wallet.h \
  wallet/wallet_ismine.h \
  wallet/walletdb.h \
  zmq/zmqabstractnotifier.h \
  zmq/zmqconfig.h\
  zmq/zmqnotificationinterface.h \
  zmq/zmqpublishnotifier.h

obj/build.h: FORCE
	@$(MKDIR_P) $(builddir)/obj
	@$(top_srcdir)/share/genbuild.sh $(abs_top_builddir)/src/obj/build.h \
	  $(abs_top_srcdir)
libcredits_util_a-clientversion.$(OBJEXT): obj/build.h

# server: shared between creditsd and credits-qt
libcredits_server_a_CPPFLAGS = $(AM_CPPFLAGS) $(CREDITS_INCLUDES) $(MINIUPNPC_CPPFLAGS) $(EVENT_CFLAGS) $(EVENT_PTHREADS_CFLAGS)
libcredits_server_a_CXXFLAGS = $(AM_CXXFLAGS) $(PIE_FLAGS)
libcredits_server_a_SOURCES = \
  addrman.cpp \
  alert.cpp \
  bloom.cpp \
  chain.cpp \
  checkpoints.cpp \
  dbwrapper.cpp \
  governance.cpp \
  governance-classes.cpp \
  governance-object.cpp \
  governance-vote.cpp \
  governance-votedb.cpp \
  httprpc.cpp \
  httpserver.cpp \
  init.cpp \
  main.cpp \
  merkleblock.cpp \
  messagesigner.cpp \
  miner.cpp \
  net.cpp \
  netfulfilledman.cpp \
  noui.cpp \
  policy/fees.cpp \
  policy/policy.cpp \
  pow.cpp \
  rest.cpp \
  rpcblockchain.cpp \
  rpcmasternode.cpp \
  rpcgovernance.cpp \
  rpcmining.cpp \
  rpcmisc.cpp \
  rpcnet.cpp \
  rpcrawtransaction.cpp \
  rpcserver.cpp \
  script/sigcache.cpp \
  sendalert.cpp \
  timedata.cpp \
  torcontrol.cpp \
  txdb.cpp \
  txmempool.cpp \
  ui_interface.cpp \
  validationinterface.cpp \
  versionbits.cpp \
  $(CREDITS_CORE_H)

if ENABLE_ZMQ
LIBCREDITS_ZMQ=libcredits_zmq.a

libcredits_zmq_a_CPPFLAGS = $(CREDITS_INCLUDES) $(ZMQ_CFLAGS)
libcredits_zmq_a_CXXFLAGS = $(AM_CXXFLAGS) $(PIE_FLAGS)
libcredits_zmq_a_SOURCES = \
  zmq/zmqabstractnotifier.cpp \
  zmq/zmqnotificationinterface.cpp \
  zmq/zmqpublishnotifier.cpp
endif


# wallet: shared between creditsd and credits-qt, but only linked
# when wallet enabled
libcredits_wallet_a_CPPFLAGS = $(AM_CPPFLAGS) $(CREDITS_INCLUDES)
libcredits_wallet_a_CXXFLAGS = $(AM_CXXFLAGS) $(PIE_FLAGS)
libcredits_wallet_a_SOURCES = \
  activemasternode.cpp \
  masternode.cpp \
  masternode-payments.cpp \
  masternode-sync.cpp \
  masternodeconfig.cpp \
  masternodeman.cpp \
  instantsend.cpp \
  keepass.cpp \
  policy/rbf.cpp \
  privatesend.cpp \
  privatesend-relay.cpp \
  psnotificationinterface.cpp \
  wallet/crypter.cpp \
  wallet/db.cpp \
  wallet/rpcdump.cpp \
  wallet/rpcwallet.cpp \
  wallet/wallet.cpp \
  wallet/wallet_ismine.cpp \
  wallet/walletdb.cpp \
  $(CREDITS_CORE_H)

# crypto primitives library
crypto_libcredits_crypto_a_CPPFLAGS = $(AM_CPPFLAGS) $(CREDITS_CONFIG_INCLUDES) $(PIC_FLAGS)
crypto_libcredits_crypto_a_CXXFLAGS = $(AM_CXXFLAGS) $(PIE_FLAGS) $(PIC_FLAGS)
crypto_libcredits_crypto_a_SOURCES = \
  crypto/common.h \
  crypto/argon2d/argon2.h \
  crypto/argon2d/core.h \
  crypto/argon2d/encoding.h \
  crypto/argon2d/thread.h \
  crypto/argon2d/argon2.c \
  crypto/argon2d/core.c \
  crypto/argon2d/encoding.c \
  crypto/argon2d/opt.c \
  crypto/argon2d/thread.c \
  crypto/blake2/blake2b.c \
  crypto/blake2/blake2.h \
  crypto/blake2/blake2-impl.h \
  crypto/blake2/blamka-round-opt.h \
  crypto/blake2/blamka-round-ref.h \
  crypto/hmac_sha256.cpp \
  crypto/hmac_sha256.h \
  crypto/hmac_sha512.cpp \
  crypto/hmac_sha512.h \
  crypto/ripemd160.cpp \
  crypto/ripemd160.h \
  crypto/sha1.cpp \
  crypto/sha1.h \
  crypto/sha256.cpp \
  crypto/sha256.h \
  crypto/sha512.h \
  crypto/sha512.cpp \
  crypto/sha512.h

# common: shared between creditsd, and credits-qt and non-server tools
libcredits_common_a_CPPFLAGS = $(AM_CPPFLAGS) $(CREDITS_INCLUDES)
libcredits_common_a_CXXFLAGS = $(AM_CXXFLAGS) $(PIE_FLAGS)
libcredits_common_a_SOURCES = \
  amount.cpp \
  arith_uint256.cpp \
  base58.cpp \
  bip39.cpp \
  chainparams.cpp \
  coins.cpp \
  compressor.cpp \
  consensus/merkle.cpp \
  core_read.cpp \
  core_write.cpp \
  hash.cpp \
  hdchain.cpp \
  key.cpp \
  keystore.cpp \
  netbase.cpp \
  primitives/block.cpp \
  primitives/transaction.cpp \
  protocol.cpp \
  pubkey.cpp \
  scheduler.cpp \
  script/interpreter.cpp \
  script/script.cpp \
  script/script_error.cpp \
  script/sign.cpp \
  script/standard.cpp \
  spork.cpp \
  $(CREDITS_CORE_H)

# util: shared between all executables.
# This library *must* be included to make sure that the glibc
# backward-compatibility objects and their sanity checks are linked.
libcredits_util_a_CPPFLAGS = $(AM_CPPFLAGS) $(CREDITS_INCLUDES)
libcredits_util_a_CXXFLAGS = $(AM_CXXFLAGS) $(PIE_FLAGS)
libcredits_util_a_SOURCES = \
  chainparamsbase.cpp \
  clientversion.cpp \
  compat/glibc_sanity.cpp \
  compat/glibcxx_sanity.cpp \
  compat/strnlen.cpp \
  random.cpp \
  rpcprotocol.cpp \
  support/cleanse.cpp \
  support/lockedpool.cpp \
  sync.cpp \
  uint256.cpp \
  util.cpp \
  utilmoneystr.cpp \
  utilstrencodings.cpp \
  utiltime.cpp \
  $(CREDITS_CORE_H)

if GLIBC_BACK_COMPAT
libcredits_util_a_SOURCES += compat/glibc_compat.cpp
endif

# cli: shared between credits-cli and credits-qt
libcredits_cli_a_CPPFLAGS = $(AM_CPPFLAGS) $(CREDITS_INCLUDES)
libcredits_cli_a_CXXFLAGS = $(AM_CXXFLAGS) $(PIE_FLAGS)
libcredits_cli_a_SOURCES = \
  rpcclient.cpp \
  $(CREDITS_CORE_H)

nodist_libcredits_util_a_SOURCES = $(srcdir)/obj/build.h
#

# creditsd binary #
creditsd_SOURCES = creditsd.cpp
creditsd_CPPFLAGS = $(AM_CPPFLAGS) $(CREDITS_INCLUDES)
creditsd_CXXFLAGS = $(AM_CXXFLAGS) $(PIE_FLAGS)
creditsd_LDFLAGS = $(RELDFLAGS) $(AM_LDFLAGS) $(LIBTOOL_APP_LDFLAGS)

if TARGET_WINDOWS
creditsd_SOURCES += creditsd-res.rc
endif

creditsd_LDADD = \
  $(LIBCREDITS_SERVER) \
  $(LIBCREDITS_COMMON) \
  $(LIBUNIVALUE) \
  $(LIBCREDITS_UTIL) \
  $(LIBCREDITS_CRYPTO) \
  $(LIBLEVELDB) \
  $(LIBMEMENV) \
  $(LIBSECP256K1)

if ENABLE_ZMQ
creditsd_LDADD += $(LIBCREDITS_ZMQ) $(ZMQ_LIBS)
endif

if ENABLE_WALLET
creditsd_LDADD += libcredits_wallet.a
endif

creditsd_LDADD += $(BOOST_LIBS) $(BDB_LIBS) $(SSL_LIBS) $(CRYPTO_LIBS) $(MINIUPNPC_LIBS) $(EVENT_PTHREADS_LIBS) $(EVENT_LIBS)

# credits-cli binary #
credits_cli_SOURCES = credits-cli.cpp
credits_cli_CPPFLAGS = $(AM_CPPFLAGS) $(CREDITS_INCLUDES) $(EVENT_CFLAGS)
credits_cli_CXXFLAGS = $(AM_CXXFLAGS) $(PIE_FLAGS)
credits_cli_LDFLAGS = $(RELDFLAGS) $(AM_LDFLAGS) $(LIBTOOL_APP_LDFLAGS)

if TARGET_WINDOWS
credits_cli_SOURCES += credits-cli-res.rc
endif

credits_cli_LDADD = \
  $(LIBCREDITS_CLI) \
  $(LIBUNIVALUE) \
  $(LIBCREDITS_UTIL)
credits_cli_LDADD += $(BOOST_LIBS) $(SSL_LIBS) $(CRYPTO_LIBS) $(EVENT_LIBS)
#

# credits-tx binary #
credits_tx_SOURCES = credits-tx.cpp
credits_tx_CPPFLAGS = $(AM_CPPFLAGS) $(CREDITS_INCLUDES)
credits_tx_CXXFLAGS = $(AM_CXXFLAGS) $(PIE_FLAGS)
credits_tx_LDFLAGS = $(RELDFLAGS) $(AM_LDFLAGS) $(LIBTOOL_APP_LDFLAGS)

if TARGET_WINDOWS
credits_tx_SOURCES += credits-tx-res.rc
endif

credits_tx_LDADD = \
  $(LIBUNIVALUE) \
  $(LIBCREDITS_COMMON) \
  $(LIBCREDITS_UTIL) \
  $(LIBCREDITS_CRYPTO) \
  $(LIBSECP256K1)

credits_tx_LDADD += $(BOOST_LIBS) $(CRYPTO_LIBS)
#

# creditsconsensus library #
if BUILD_CREDITS_LIBS
include_HEADERS = script/creditsconsensus.h
libcreditsconsensus_la_SOURCES = \
  crypto/hmac_sha512.cpp \
  crypto/ripemd160.cpp \
  crypto/sha1.cpp \
  crypto/sha256.cpp \
  crypto/sha512.cpp \
  hash.cpp \
  primitives/transaction.cpp \
  pubkey.cpp \
  script/creditsconsensus.cpp \
  script/interpreter.cpp \
  script/script.cpp \
  uint256.cpp \
  utilstrencodings.cpp

if GLIBC_BACK_COMPAT
  libcreditsconsensus_la_SOURCES += compat/glibc_compat.cpp
endif

libcreditsconsensus_la_LDFLAGS = $(AM_LDFLAGS) -no-undefined $(RELDFLAGS)
libcreditsconsensus_la_LIBADD = $(LIBSECP256K1)
libcreditsconsensus_la_CPPFLAGS = $(AM_CPPFLAGS) -I$(builddir)/obj -I$(srcdir)/secp256k1/include -DBUILD_CREDITS_INTERNAL
libcreditsconsensus_la_CXXFLAGS = $(AM_CXXFLAGS) $(PIE_FLAGS)

endif
#

CLEANFILES = leveldb/libleveldb.a leveldb/libmemenv.a
CLEANFILES += *.gcda *.gcno
CLEANFILES += compat/*.gcda compat/*.gcno
CLEANFILES += consensus/*.gcda consensus/*.gcno
CLEANFILES += crypto/*.gcda crypto/*.gcno
CLEANFILES += policy/*.gcda policy/*.gcno
CLEANFILES += primitives/*.gcda primitives/*.gcno
CLEANFILES += script/*.gcda script/*.gcno
CLEANFILES += support/*.gcda support/*.gcno
CLEANFILES += univalue/*.gcda univalue/*.gcno
CLEANFILES += wallet/*.gcda wallet/*.gcno
CLEANFILES += wallet/test/*.gcda wallet/test/*.gcno
CLEANFILES += zmq/*.gcda zmq/*.gcno

DISTCLEANFILES = obj/build.h

EXTRA_DIST = leveldb

clean-local:
	-$(MAKE) -C leveldb clean
	-$(MAKE) -C secp256k1 clean
	-$(MAKE) -C univalue clean
	-rm -f leveldb/*/*.gcda leveldb/*/*.gcno leveldb/helpers/memenv/*.gcda leveldb/helpers/memenv/*.gcno
	-rm -f config/credits-config.* config/stamp-h1

.rc.o:
	@test -f $(WINDRES)
	$(AM_V_GEN) $(WINDRES) -DWINDRES_PREPROC -i $< -o $@

.mm.o:
	$(AM_V_CXX) $(OBJCXX) $(DEFS) $(DEFAULT_INCLUDES) $(INCLUDES) $(AM_CPPFLAGS) \
	  $(CPPFLAGS) $(AM_CXXFLAGS) $(QT_INCLUDES) $(AM_CXXFLAGS) $(PIE_FLAGS) $(CXXFLAGS) -c -o $@ $<

check-symbols: $(bin_PROGRAMS)
if GLIBC_BACK_COMPAT
	@echo "Checking glibc back compat..."
	$(AM_V_at) READELF=$(READELF) CPPFILT=$(CPPFILT) $(top_srcdir)/contrib/devtools/symbol-check.py < $(bin_PROGRAMS)
endif

check-security: $(bin_PROGRAMS)
if HARDEN
	@echo "Checking binary security..."
	$(AM_V_at) READELF=$(READELF) OBJDUMP=$(OBJDUMP) $(top_srcdir)/contrib/devtools/security-check.py < $(bin_PROGRAMS)
endif

%.pb.cc %.pb.h: %.proto
	@test -f $(PROTOC)
	$(AM_V_GEN) $(PROTOC) --cpp_out=$(@D) --proto_path=$(abspath $(<D) $<)

if EMBEDDED_LEVELDB
include Makefile.leveldb.include
endif

if ENABLE_TESTS
include Makefile.test.include
endif

if ENABLE_BENCH
include Makefile.bench.include
endif

if ENABLE_QT
include Makefile.qt.include
endif

if ENABLE_QT_TESTS
include Makefile.qttest.include
endif
