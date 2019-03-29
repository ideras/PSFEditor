#ifndef DLGSYMBINFO_H
#define DLGSYMBINFO_H

#include <QDialog>

namespace Ui {
class DlgSymbInfo;
}

class DlgSymbInfo : public QDialog
{
    Q_OBJECT

public:
    explicit DlgSymbInfo(QWidget *parent = nullptr);
    ~DlgSymbInfo();

    int getWidth() { return width; }
    int getHeight() { return height; }

private slots:
    void on_buttonBox_accepted();

private:
    Ui::DlgSymbInfo *ui;
    int width;
    int height;
};

#endif // DLGSYMBINFO_H
