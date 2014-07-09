#ifndef REPORTDIALOG_H
#define REPORTDIALOG_H

#include <QDialog>

namespace Ui {
    class ReportDialog;
}

class ReportDialog : public QDialog {
    Q_OBJECT
public:
    ReportDialog(QWidget *parent = 0);
    ~ReportDialog();

protected:
    void changeEvent(QEvent *e);

private:
    Ui::ReportDialog *ui;
public slots:
	void setup_report(int ri);
private slots:
		void on_lineEdit_clients_filter_textChanged(QString );
	void on_toolButton_select_all_clicked();
	//void on_checkBox_wholesalers_toggled(bool checked);
	//void on_checkBox_private_toggled(bool checked);
	void on_comboBox_report_selector_activated(int index);
	void on_buttonBox_accepted();
};

#endif // REPORTDIALOG_H
