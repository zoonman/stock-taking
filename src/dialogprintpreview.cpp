#include <QTextEdit>
#include <QPrinter>
#include <QPrintDialog>
#include <QSettings>
#include "dialogprintpreview.h"
#include "ui_dialogprintpreview.h"

DialogPrintPreview::DialogPrintPreview(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::DialogPrintPreview)
{
	QApplication::setOverrideCursor(Qt::WaitCursor);
	ui->setupUi(this);
	QSettings settings(this);
	resize(settings.value("rep_size", QSize(640, 480)).toSize());
	ui->checkBox_album_orientation->setCheckState((Qt::CheckState)settings.value("rep_orient", Qt::Unchecked).toUInt() );

	ui->checkBox_fields->setCheckState((Qt::CheckState)settings.value("rep_fields", Qt::Unchecked).toUInt() );
	if (ui->checkBox_fields->checkState() == Qt::Checked){
		ui->spinBox_field_width->setEnabled(true);
	}
	ui->spinBox_field_width->setValue(settings.value("rep_fields_width", 5).toInt());

	QApplication::restoreOverrideCursor();
	ui->buttonBox->setFocus();
}

DialogPrintPreview::~DialogPrintPreview()
{
	QSettings settings(this);
	settings.setValue("rep_size", size());
	settings.setValue("rep_orient", ui->checkBox_album_orientation->checkState());
	settings.setValue("rep_fields", ui->checkBox_fields->checkState());
	settings.setValue("rep_fields_width", ui->spinBox_field_width->value());

	delete ui;
}

void DialogPrintPreview::changeEvent(QEvent *e)
{
    QDialog::changeEvent(e);
    switch (e->type()) {
    case QEvent::LanguageChange:
        ui->retranslateUi(this);
        break;
    default:
        break;
    }
}


void DialogPrintPreview::set_doc(QString dht) {
	QApplication::setOverrideCursor(Qt::WaitCursor);
	ui->textEdit_preview->setHtml(dht);
	QApplication::restoreOverrideCursor();
}

void DialogPrintPreview::on_buttonBox_accepted()
{
	QApplication::restoreOverrideCursor();
	QPrinter printer;
	QPrintDialog *dialog = new QPrintDialog(&printer, this);
	dialog->setWindowTitle(tr("Print Document"));
			 /*if (editor->textCursor().hasSelection())
					 dialog->addEnabledOption(QAbstractPrintDialog::PrintSelection);*/
	// если отказ от печати, то вернуться
	if (dialog->exec() != QDialog::Accepted)  return;

	if (ui->checkBox_album_orientation->checkState() == 2) {
		printer.setOrientation(QPrinter::Landscape);
	}
	//printer.setOutputFormat(QPrinter::PdfFormat);
	//printer.setOutputFileName(tr("tst.pdf"));

	if(ui->checkBox_fields->checkState() == 2) {
		printer.setPageMargins(ui->spinBox_field_width->value(),
													 ui->spinBox_field_width->value(),ui->spinBox_field_width->value(),
													 ui->spinBox_field_width->value(),QPrinter::Millimeter);

	}

	ui->textEdit_preview->print(&printer);


	//webView

}
