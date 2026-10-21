#include "ptyterminal.h"
#include <pty.h>
#include <unistd.h>
#include <termios.h>
#include <sys/ioctl.h>
#include <QRegularExpression>
#include <QDebug>

PtyTerminal::PtyTerminal(QObject *parent) : QObject(parent) {
    struct winsize ws;
    ws.ws_col = 35;
    ws.ws_row = 18;
    ws.ws_xpixel = 400;
    ws.ws_ypixel = 400;

    m_childPid = forkpty(&m_masterFd, nullptr, nullptr, &ws);

    if (m_childPid == 0) {
        // Child shell process
        setenv("TERM", "vt100", 1);
        setenv("HOME", "/home/ceres", 1);
        setenv("USER", "ceres", 1);
        chdir("/home/ceres");
        execl("/bin/sh", "sh", "-l", nullptr);
        _exit(1);
    } else if (m_childPid > 0) {
        // Parent process
        m_notifier = new QSocketNotifier(m_masterFd, QSocketNotifier::Read, this);
        connect(m_notifier, &QSocketNotifier::activated, this, &PtyTerminal::onPtyReadyRead);
    }
}

PtyTerminal::~PtyTerminal() {
    if (m_masterFd >= 0) {
        close(m_masterFd);
    }
}

void PtyTerminal::onPtyReadyRead() {
    char buf[1024];
    ssize_t bytesRead = read(m_masterFd, buf, sizeof(buf) - 1);
    if (bytesRead > 0) {
        buf[bytesRead] = '\0';
        QString raw = QString::fromUtf8(buf);
        m_textBuffer.append(cleanAnsi(raw));

        // Keep buffer manageable on watch RAM
        if (m_textBuffer.length() > 8000) {
            m_textBuffer = m_textBuffer.right(5000);
        }

        emit textBufferChanged();
    }
}

void PtyTerminal::sendKey(const QString &str) {
    if (m_masterFd >= 0 && !str.isEmpty()) {
        QByteArray data = str.toUtf8();
        write(m_masterFd, data.constData(), data.size());
    }
}

void PtyTerminal::clear() {
    m_textBuffer.clear();
    emit textBufferChanged();
}

QString PtyTerminal::cleanAnsi(const QString &input) {
    static QRegularExpression ansiRegex("\x1B\\[[0-9;]*[a-zA-Z]");
    return QString(input).remove(ansiRegex);
}