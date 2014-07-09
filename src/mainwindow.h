#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QtWidgets/QMainWindow>
#include <QtWidgets/QTableWidgetItem>
#include <QtWidgets/QListWidgetItem>
#include <QtWidgets/QMessageBox>



namespace Ui
{
    class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = 0);
    ~MainWindow();

private:
    Ui::MainWindow *ui;
protected:
		void closeEvent(QCloseEvent * event);

private slots:


void on_comboBox_order_search_activated(QString );
void on_action_report_wholesalers_triggered();
void on_toolButton_remove_customer_clicked();
void on_action_spr_merchandise_triggered();
void on_action_client_list_triggered();
void on_comboBox_maker_activated(QString );
QString number_in_words(int Number, int Rod  = 1,	 QString R5 = "", QString R1= "", QString R2= "", int  Sklonenie=5);
QString plural3(int Number, int Sklonenie, int Rod, QString R5, QString R1, QString R2);
 void on_pushButton_shipping_clicked();
 void on_action_shipping_triggered();
 void on_checkBox_only_wholesalers_toggled(bool checked);
 void on_toolButton_add_customer_clicked();
 void on_tableWidget_customers_itemChanged(QTableWidgetItem* item);
 void on_toolButton_livesearch_clicked();
 void on_action_report_retail_triggered();
 void on_lineEdit_order_search_textChanged(QString );
 void on_tableView_order_list_activated(QModelIndex index);
 void on_pushButton_begin_search_order_toggled(bool checked);
 void on_lineEdit_lift_price_editingFinished();
 void on_lineEdit_delivery_price_editingFinished();
 void on_pushButton_act_clicked();
 void on_pushButton_print_check_clicked();
 void on_lineEdit_payment_returnPressed();
 void on_lineEdit_lift_price_returnPressed();
 void on_lineEdit_delivery_price_returnPressed();
 void on_lineEdit_contact_returnPressed();
 void on_lineEdit_phone_returnPressed();
 void on_lineEdit_flat_returnPressed();
 void on_lineEdit_house_returnPressed();
 void on_lineEdit_street_returnPressed();
 void on_lineEdit_city_returnPressed();
 void on_action_help_triggered();
 void on_action_print_act_triggered();
 void on_comboBox_customer_editTextChanged(QString );
	void on_lineEdit_customer_textChanged(QString );
	void on_actionCleanlooks_triggered();
	void on_actionCDE_triggered();
	void on_actionPlastique_triggered();
	void on_actionWindows_triggered();
	void on_action_add_retail_triggered();
	void on_action_remove_order_triggered();
	void on_action_go_next_order_triggered();
	void on_action_go_previos_order_triggered();
		void on_action_save_order_triggered();
    void on_lineEdit_payment_textChanged(QString );
    void on_action_print_check_triggered();
    void on_action_remove_order_line_triggered();
    void on_action_add_order_line_triggered();
    void on_tableWidget_ordered_cellChanged(int row, int column);
    void on_tableWidget_ordered_cellEntered(int row, int column);
    void on_tableWidget_ordered_cellActivated(int row, int column);
    void on_listWidget_listval_itemClicked(QListWidgetItem* item);
    void on_listWidget_listval_entered(QModelIndex index);
    void on_toolButton_remove_clicked();
    void on_comboBox_activated(int index);
    void on_comboBox_currentIndexChanged(int index);
    void on_toolButton_update_clicked();
    void on_toolButton_add_clicked();
    void on_tableWidget_ordered_cellPressed(int row, int column);
    void on_actionMotif_triggered();
    void on_action_add_wholesaler_triggered();
    void on_pushButton_add_clicked();
    void on_action_mysql_config_triggered();
    void on_action_about_triggered();
    void on_action_exit_triggered();
    void on_pushButton_save_clicked();
    void table_recalculate(int c);
    void table_recalculate(const QString & text);
		void load_order(int g);
		void saved(bool v);
		void order_line_size_changed(QString );
};

#endif // MAINWINDOW_H
