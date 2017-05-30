// Copyright (c) 2015 The Bitcoin Core developers
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#ifndef CREDITS_ZMQ_ZMQCONFIG_H
#define CREDITS_ZMQ_ZMQCONFIG_H

#if defined(HAVE_CONFIG_H)
#include "config/credits-config.h"
#endif

#include "primitives/block.h"
#include "primitives/transaction.h"

#if ENABLE_ZMQ
#include <zmq.h>
#endif

#include <stdarg.h>
#include <string>

void zmqError(const char *str);

#endif // CREDITS_ZMQ_ZMQCONFIG_H
