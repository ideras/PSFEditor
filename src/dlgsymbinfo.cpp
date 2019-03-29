#include "dlgsymbinfo.h"
#include "ui_dlgsymbinfo.h"

DlgSymbInfo::DlgSymbInfo(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::DlgSymbInfo)
{
    ui->setupUi(this);
}

DlgSymbInfo::~DlgSymbInfo()
{
    delete ui;
}

void DlgSymbInfo::on_buttonBox_accepted()
{
    width = ui->edtWidth->text().toInt();
    height = ui->edtHeight->text().toInt();
    accept();
}
