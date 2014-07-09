#ifndef DIALOGPRINTPREVIEW_H
#define DIALOGPRINTPREVIEW_H

#include <QDialog>

namespace Ui {
    class DialogPrintPreview;
}

class DialogPrintPreview : public QDialog {
    Q_OBJECT
public:
    DialogPrintPreview(QWidget *parent = 0);
    ~DialogPrintPreview();

protected:
    void changeEvent(QEvent *e);

private:
    Ui::DialogPrintPreview *ui;


private slots:
		void on_buttonBox_accepted();
public slots:
		void set_doc(QString dht);

};

#endif // DIALOGPRINTPREVIEW_H
