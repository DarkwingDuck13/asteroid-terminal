#include <QGuiApplication>
#include <QQuickView>
#include <QQmlContext>
#include "ptyterminal.h"

int main(int argc, char *argv[]) {
    QGuiApplication app(argc, argv);
    app.setApplicationName("asteroid-terminal");

    QQuickView view;
    PtyTerminal pty;

    view.rootContext()->setContextProperty("pty", &pty);
    view.setSource(QUrl("qrc:/qml/main.qml"));
    view.setResizeMode(QQuickView::SizeRootObjectToView);
    view.showFullScreen();
    
    // Explicitly demand keyboard focus from the Wayland compositor
    view.requestActivate();

    return app.exec();
}
