#include "ptyterminal.h"
#include <pty.h>
#include <unistd.h>
#include <fcntl.h>
#include <termios.h>
#include <sys/ioctl.h>
#include <linux/input.h>
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
        setenv("TERM", "vt100", 1);
        setenv("HOME", "/home/ceres", 1);
        setenv("USER", "ceres", 1);
        chdir("/home/ceres");
        execl("/bin/sh", "sh", "-l", nullptr);
        _exit(1);
    } else if (m_childPid > 0) {
        m_ptyNotifier = new QSocketNotifier(m_masterFd, QSocketNotifier::Read, this);
        connect(m_ptyNotifier, &QSocketNotifier::activated, this, &PtyTerminal::onPtyReadyRead);
    }

    tryOpenKeyboard();

    m_kbdWatchTimer = new QTimer(this);
    connect(m_kbdWatchTimer, &QTimer::timeout, this, &PtyTerminal::tryOpenKeyboard);
    m_kbdWatchTimer->start(2000);
}

PtyTerminal::~PtyTerminal() {
    if (m_masterFd >= 0) close(m_masterFd);
    if (m_kbdFd >= 0) close(m_kbdFd);
}

void PtyTerminal::tryOpenKeyboard() {
    if (m_kbdFd >= 0) {
        char name[256] = {0};
        if (ioctl(m_kbdFd, EVIOCGNAME(sizeof(name)), name) >= 0) {
            return;
        }
        if (m_kbdNotifier) {
            delete m_kbdNotifier;
            m_kbdNotifier = nullptr;
        }
        close(m_kbdFd);
        m_kbdFd = -1;
    }

    for (int i = 0; i < 16; ++i) {
        QString devPath = QString("/dev/input/event%1").arg(i);
        int fd = open(devPath.toUtf8().constData(), O_RDONLY | O_NONBLOCK);
        if (fd >= 0) {
            char name[256] = {0};
            if (ioctl(fd, EVIOCGNAME(sizeof(name)), name) >= 0) {
                QString devName = QString::fromUtf8(name);
                if (devName.contains("Keyboard", Qt::CaseInsensitive)) {
                    m_kbdFd = fd;
                    m_kbdNotifier = new QSocketNotifier(m_kbdFd, QSocketNotifier::Read, this);
                    connect(m_kbdNotifier, &QSocketNotifier::activated, this, &PtyTerminal::onKbdReadyRead);
                    return;
                }
            }
            close(fd);
        }
    }
}

void PtyTerminal::onKbdReadyRead() {
    struct input_event ev;
    while (read(m_kbdFd, &ev, sizeof(ev)) == sizeof(ev)) {
        if (ev.type == EV_KEY) {
            handleKeyEvent(ev.code, ev.value);
        }
    }
}

void PtyTerminal::handleKeyEvent(int code, int value) {
    if (code == KEY_LEFTSHIFT || code == KEY_RIGHTSHIFT) {
        m_shiftPressed = (value != 0);
        return;
    }
    if (code == KEY_LEFTCTRL || code == KEY_RIGHTCTRL) {
        m_ctrlPressed = (value != 0);
        return;
    }

    if (value == 0) return;

    if (m_ctrlPressed) {
        if (code == KEY_C) { sendKey("\x03"); return; }
        if (code == KEY_D) { sendKey("\x04"); return; }
        if (code == KEY_L) { sendKey("\x0c"); return; }
        if (code == KEY_Z) { sendKey("\x1a"); return; }
    }

    switch (code) {
        case KEY_ENTER:      sendKey("\r"); return;
        case KEY_BACKSPACE:  sendKey("\x7f"); return; // Standard DEL/BS
        case KEY_TAB:        sendKey("\t"); return;
        case KEY_ESC:        sendKey("\x1b"); return;
        case KEY_UP:         sendKey("\x1b[A"); return;
        case KEY_DOWN:       sendKey("\x1b[B"); return;
        case KEY_RIGHT:      sendKey("\x1b[C"); return;
        case KEY_LEFT:       sendKey("\x1b[D"); return;
        case KEY_SPACE:      sendKey(" "); return;
    }

    const char *lower = "  1234567890-=\b\tqwertyuiop[]\r asdfghjkl;'` \\zxcvbnm,./";
    const char *upper = "  !@#$%^&*()_+\b\tQWERTYUIOP{}\r ASDFGHJKL:\"~ |ZXCVBNM<>?";

    if (code < 60) {
        char ch = m_shiftPressed ? upper[code] : lower[code];
        if (ch != ' ' || code == KEY_SPACE) {
            QString s(ch);
            sendKey(s);
        }
    }
}

void PtyTerminal::onPtyReadyRead() {
    char buf[1024];
    ssize_t bytesRead = read(m_masterFd, buf, sizeof(buf) - 1);
    if (bytesRead > 0) {
        buf[bytesRead] = '\0';
        QString clean = cleanAnsi(QString::fromUtf8(buf));

        // Process terminal characters (including backspaces)
        for (int i = 0; i < clean.length(); ++i) {
            QChar c = clean[i];
            if (c == '\b') {
                // If shell echoes backspace, delete the last character from the buffer!
                if (!m_textBuffer.isEmpty() && m_textBuffer.back() != '\n') {
                    m_textBuffer.chop(1);
                }
            } else if (c == '\r') {
                // Ignore raw carriage returns
            } else {
                m_textBuffer.append(c);
            }
        }

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
