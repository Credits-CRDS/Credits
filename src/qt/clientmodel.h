// Copyright (c) 2009-2017 Satoshi Nakamoto
// Copyright (c) 2009-2017 The Bitcoin Developers
// Copyright (c) 2014-2017 The Dash Core Developers
// Copyright (c) 2017 Credits Developers
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#ifndef CREDITS_QT_CLIENTMODEL_H
#define CREDITS_QT_CLIENTMODEL_H

#include <QObject>
#include <QDateTime>

class AddressTableModel;
class BanTableModel;
class OptionsModel;
class PeerTableModel;
class TransactionTableModel;

class CBlockIndex;
class CWallet;

QT_BEGIN_NAMESPACE
class QTimer;
QT_END_NAMESPACE

enum BlockSource {
    BLOCK_SOURCE_NONE,
    BLOCK_SOURCE_REINDEX,
    BLOCK_SOURCE_DISK,
    BLOCK_SOURCE_NETWORK
};

enum NumConnections {
    CONNECTIONS_NONE = 0,
    CONNECTIONS_IN   = (1U << 0),
    CONNECTIONS_OUT  = (1U << 1),
    CONNECTIONS_ALL  = (CONNECTIONS_IN | CONNECTIONS_OUT),
};

/** Model for Credits network client. */
class ClientModel : public QObject
{
    Q_OBJECT

public:
    explicit ClientModel(OptionsModel *optionsModel, QObject *parent = 0);
    ~ClientModel();

    OptionsModel *getOptionsModel();
    PeerTableModel *getPeerTableModel();
    BanTableModel *getBanTableModel();

    //! Return number of connections, default is in- and outbound (total)
    int getNumConnections(unsigned int flags = CONNECTIONS_ALL) const;
    QString getMasternodeCountString() const;
    int getNumBlocks() const;
    int getHeaderTipHeight() const;
    int64_t getHeaderTipTime() const;

    //! Return number of transactions in the mempool
    long getMempoolSize() const;
    //! Return the credits memory usage of the mempool
    size_t getMempoolCreditsUsage() const;
    
    quint64 getTotalBytesRecv() const;
    quint64 getTotalBytesSent() const;

    double getVerificationProgress(const CBlockIndex *tip) const;
    QDateTime getLastBlockDate() const;

    //! Return true if core is doing initial block download
    bool inInitialBlockDownload() const;
    //! Return true if core is importing blocks
    enum BlockSource getBlockSource() const;
    //! Return warnings to be displayed in status bar
    QString getStatusBarWarnings() const;

    QString formatFullVersion() const;
    QString formatSubVersion() const;
    QString formatBuildDate() const;
    bool isReleaseVersion() const;
    QString clientName() const;
    QString formatClientStartupTime() const;

private:
    OptionsModel *optionsModel;
    PeerTableModel *peerTableModel;
    QString cachedMasternodeCountString;
    BanTableModel *banTableModel;

    QTimer *pollTimer;
    QTimer *pollSnTimer;

    void subscribeToCoreSignals();
    void unsubscribeFromCoreSignals();

Q_SIGNALS:
    void numConnectionsChanged(int count);
    void strMasternodesChanged(const QString &strMasternodes);
    void numBlocksChanged(int count, const QDateTime& blockDate, double nVerificationProgress, bool header);
    void additionalDataSyncProgressChanged(double nSyncProgress);
    void mempoolSizeChanged(long count, size_t mempoolSizeInBytes);
    void alertsChanged(const QString &warnings);
    void bytesChanged(quint64 totalBytesIn, quint64 totalBytesOut);

    //! Fired when a message should be reported to the user
    void message(const QString &title, const QString &message, unsigned int style);

    // Show progress dialog e.g. for verifychain
    void showProgress(const QString &title, int nProgress);

public Q_SLOTS:
    void updateTimer();
    void updateSnTimer();
    void updateNumConnections(int numConnections);
    void updateAlert(const QString &hash, int status);
    void updateBanlist();
};

#endif // CREDITS_QT_CLIENTMODEL_H
