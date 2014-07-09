#ifndef SHIPPINGDIALOG_H
#define SHIPPINGDIALOG_H

#include <QDialog>
#include <QModelIndex>

namespace Ui {
    class ShippingDialog;
}

class ShippingDialog : public QDialog {
    Q_OBJECT
public:
    ShippingDialog(QWidget *parent = 0);
    ~ShippingDialog();

protected:
    void changeEvent(QEvent *e);

private:
    Ui::ShippingDialog *ui;
		int current_order;

private slots:
		void on_toolButton_remove_from_shipping_list_clicked();
	void on_toolButton_print_shipping_act_clicked();
	void on_tableView_shipping_activated(QModelIndex index);
	void on_toolButton_shipping_search_clicked();

	void on_toolButton_save_shipping_act_clicked();
	void on_actionSave_shipping_act_triggered();
	void on_toolButton_new_shipping_act_clicked();
	void on_toolButton_add_to_shipping_list_clicked();
	void on_action_add_all_position_triggered();
	void on_action_add_one_position_triggered();
	void load_shipping_act(int id);
	void on_toolButton_add_to_shipping_list_customContextMenuRequested(QPoint pos);


public slots:
	void set_current_order(int o, QString s);
	void set_shipping_tab(int id);
};

#endif // SHIPPINGDIALOG_H
