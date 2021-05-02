// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2020-2021, The Monero Project.

#include "debuginfodialog.h"
#include "ui_debuginfodialog.h"
#include "config-feather.h"
#include "utils/WebsocketClient.h"
#include "utils/TorManager.h"
#include "utils/WebsocketNotifier.h"

DebugInfoDialog::DebugInfoDialog(AppContext *ctx, QWidget *parent)
        : QDialog(parent)
        , ui(new Ui::DebugInfoDialog)
        , m_ctx(ctx)
{
    ui->setupUi(this);

    connect(ui->btn_Copy, &QPushButton::clicked, this, &DebugInfoDialog::copyToClipboad);

    m_updateTimer.start(5000);
    connect(&m_updateTimer, &QTimer::timeout, this, &DebugInfoDialog::updateInfo);
    this->updateInfo();

    this->adjustSize();
}

void DebugInfoDialog::updateInfo() {
    QString torStatus;

    // Special case for Tails because we know the status of the daemon by polling tails-tor-has-bootstrapped.target
    if(m_ctx->isTails) {
        if(torManager()->torConnected)
            torStatus = "Connected";
        else
            torStatus = "Disconnected";
    }
    else if(Utils::isTorsocks())
        torStatus = "Torsocks";
    else if(torManager()->isLocalTor())
        torStatus = "Local (assumed to be running)";
    else if(torManager()->torConnected)
        torStatus = "Running";
    else
        torStatus = "Unknown";

    ui->label_featherVersion->setText(QString("%1-%2").arg(FEATHER_VERSION, FEATHER_BRANCH));
    ui->label_moneroVersion->setText(QString("%1-%2").arg(MONERO_VERSION, MONERO_BRANCH));

    ui->label_walletHeight->setText(QString::number(m_ctx->currentWallet->blockChainHeight()));
    ui->label_daemonHeight->setText(QString::number(m_ctx->currentWallet->daemonBlockChainHeight()));
    ui->label_targetHeight->setText(QString::number(m_ctx->currentWallet->daemonBlockChainTargetHeight()));
    ui->label_restoreHeight->setText(QString::number(m_ctx->currentWallet->getWalletCreationHeight()));
    ui->label_synchronized->setText(m_ctx->currentWallet->isSynchronized() ? "True" : "False");

    auto node = m_ctx->nodes->connection();
    ui->label_remoteNode->setText(node.toAddress());
    ui->label_walletStatus->setText(this->statusToString(m_ctx->currentWallet->connectionStatus()));
    ui->label_torStatus->setText(torStatus);
    ui->label_websocketStatus->setText(Utils::QtEnumToString(websocketNotifier()->websocketClient.webSocket.state()).remove("State"));

    QString seedType = [this](){
        if (m_ctx->currentWallet->isHwBacked())
            return "Hardware";
        if (m_ctx->currentWallet->getCacheAttribute("feather.seed").isEmpty())
            return "25 word";
        else
            return "14 word";
    }();

    QString deviceType = [this](){
        if (m_ctx->currentWallet->isHwBacked()) {
            if (m_ctx->currentWallet->isLedger())
                return "Ledger";
            else if (m_ctx->currentWallet->isTrezor())
                return "Trezor";
            else
                return "Unknown";
        }
        else {
            return "Software";
        }
    }();

    ui->label_netType->setText(Utils::QtEnumToString(m_ctx->currentWallet->nettype()));
    ui->label_seedType->setText(seedType);
    ui->label_deviceType->setText(deviceType);
    ui->label_viewOnly->setText(m_ctx->currentWallet->viewOnly() ? "True" : "False");
    ui->label_primaryOnly->setText(m_ctx->currentWallet->balance(0) == m_ctx->currentWallet->balanceAll() ? "True" : "False");

    QString os = QSysInfo::prettyProductName();
    if (m_ctx->isTails) {
        os = QString("Tails %1").arg(TailsOS::version());
    }
    if (m_ctx->isWhonix) {
        os = QString("Whonix %1").arg(WhonixOS::version());
    }
    ui->label_OS->setText(os);
    ui->label_timestamp->setText(QString::number(QDateTime::currentSecsSinceEpoch()));
}

QString DebugInfoDialog::statusToString(Wallet::ConnectionStatus status) {
    switch (status) {
        case Wallet::ConnectionStatus_Disconnected:
            return "Disconnected";
        case Wallet::ConnectionStatus_WrongVersion:
            return "Daemon wrong version";
        case Wallet::ConnectionStatus_Connecting:
            return "Connecting";
        case Wallet::ConnectionStatus_Synchronizing:
            return "Synchronizing";
        case Wallet::ConnectionStatus_Synchronized:
            return "Synchronized";
        default:
            return "Unknown";
    }
}

void DebugInfoDialog::copyToClipboad() {
    // Two spaces at the end of each line are for newlines in Markdown
    QString text = "";
    text += QString("Feather version: %1  \n").arg(ui->label_featherVersion->text());
    text += QString("Monero version: %1  \n").arg(ui->label_moneroVersion->text());

    text += QString("Wallet height: %1  \n").arg(ui->label_walletHeight->text());
    text += QString("Daemon height: %1  \n").arg(ui->label_daemonHeight->text());
    text += QString("Target height: %1  \n").arg(ui->label_targetHeight->text());
    text += QString("Restore height: %1  \n").arg(ui->label_restoreHeight->text());
    text += QString("Synchronized: %1  \n").arg(ui->label_synchronized->text());

    text += QString("Remote node: %1  \n").arg(ui->label_remoteNode->text());
    text += QString("Wallet status: %1  \n").arg(ui->label_walletStatus->text());
    text += QString("Tor status: %1  \n").arg(ui->label_torStatus->text());
    text += QString("Websocket status: %1  \n").arg(ui->label_websocketStatus->text());

    text += QString("Network type: %1  \n").arg(ui->label_netType->text());
    text += QString("Seed type: %1  \n").arg(ui->label_seedType->text());
    text += QString("Device type: %1  \n").arg(ui->label_deviceType->text());
    text += QString("View only: %1  \n").arg(ui->label_viewOnly->text());
    text += QString("Primary only: %1 \n").arg(ui->label_primaryOnly->text());

    text += QString("Operating system: %1  \n").arg(ui->label_OS->text());
    text += QString("Timestamp: %1  \n").arg(ui->label_timestamp->text());

    Utils::copyToClipboard(text);
}

DebugInfoDialog::~DebugInfoDialog() {
    delete ui;
}
