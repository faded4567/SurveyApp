#ifndef GLOBALSTYLE_H
#define GLOBALSTYLE_H

#include <QString>
#include <QWidget>

class GlobalStyle
{
public:
    static QString getStyleSheet();
    static QString loadStyleSheet(const QString& fileName);
    static void applyToWidget(QWidget* widget);
};

#endif // GLOBALSTYLE_H