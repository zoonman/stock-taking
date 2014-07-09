#ifndef DIALOGMYSQLCONFIG_H
#define DIALOGMYSQLCONFIG_H

#include <QtWidgets/QDialog>


namespace Ui {
    class DialogMySQLConfig;
}

class DialogMySQLConfig : public QDialog {
    Q_OBJECT
    Q_DISABLE_COPY(DialogMySQLConfig)
public:
    explicit DialogMySQLConfig(QWidget *parent = 0);
    virtual ~DialogMySQLConfig();

protected:
    virtual void changeEvent(QEvent *e);

private:
    Ui::DialogMySQLConfig *m_ui;

private slots:
	void on_toolButton_save_maker_clicked();
 void on_comboBox_makers_activated(int index);
 void on_comboBox_makers_currentIndexChanged(int index);
 void on_toolButton_add_maker_clicked();
 void on_comboBox_ui_style_currentIndexChanged(int index);

	void on_buttonBox_accepted();

};



#endif // DIALOGMYSQLCONFIG_H
