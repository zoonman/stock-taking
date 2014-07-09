#include <QtSql>
#include <QMenu>
#include <QSpinBox>
#include <QCompleter>
#include <QSortFilterProxyModel>

#include "shippingdialog.h"
#include "ui_shippingdialog.h"
#include "dialogprintpreview.h"


QVector<int> ordered_cb_list;
int ship_id=0;

ShippingDialog::ShippingDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::ShippingDialog)
{
    ui->setupUi(this);

		QSettings settings(this);
		resize(settings.value("shipping_size", QSize(640, 480)).toSize());


		int c_ord_i = 0;
		QMenu *m1 = new QMenu(this);
			m1->addAction(ui->action_add_all_position);
			m1->addAction(ui->action_add_one_position);
			//m1.exec(pos,ui->action_add_one_position);
		ui->toolButton_add_to_shipping_list->setMenu(m1);


		ordered_cb_list.clear();
		//ui->comboBox_shipping_candidates->setModel();
		QSqlQuery query;
		// shipped(id integer primary key AUTOINCREMENT, shipping integer, ordered integer,count integer)

		query.prepare("SELECT orders.id ,ordered.name , ordered.cover , SUM(ordered.count) , SUM(shipped.count)  , ordered.id,ordered.size FROM ordered LEFT JOIN shipped ON (ordered.id  =  shipped.ordered) LEFT JOIN orders ON (ordered.porder = orders.id) WHERE ordered.id > 0 GROUP BY ordered.id ORDER BY ordered.id");

		ui->comboBox_shipping_candidates->setMaxCount(0);
		ui->comboBox_shipping_candidates->setMaxCount(10000);
		 if (query.exec()){
			 while (query.next()) {
				 int sum = query.value(3).toInt();
				 int sum_sh = query.value(4).toInt();
				 if(sum - sum_sh > 0 ){
				 /* пополнить связный список
						значение списка соответствует занчению  ordered.id - оплаченной позиции заказа
						индекс - номер строки в вып. списке

						*/
					 ordered_cb_list.append(query.value(5).toInt());
					 QString s;
					 s.setNum(sum - sum_sh);
					 ui->comboBox_shipping_candidates->addItem(query.value(0).toString() +". "+ query.value(1).toString()+". "+query.value(2).toString()+". "+query.value(6).toString()+" (" + s + trUtf8(" шт.) "));
					 if(query.value(0).toInt() == this->current_order) {c_ord_i = ui->comboBox_shipping_candidates->count() -1 ;}

				 }
			 }

		 }
		 else{
			 //qDebug() << query.lastError();
		 };
	ui->dateTimeEdit_ship->setDateTime(QDateTime::currentDateTime());
	//qDebug() << "ord: ";
	//qDebug() << c_ord_i;
	if (c_ord_i > 0 ) ui->comboBox_shipping_candidates->setCurrentIndex(c_ord_i);

	on_toolButton_shipping_search_clicked();

	//
	query.prepare("SELECT taker FROM shipping WHERE taker != '' GROUP BY taker ORDER BY taker");
	QStringList taker_list;
	if (query.exec()){
		while (query.next()) {
			taker_list << query.value(0).toString();
		}
	}
	QCompleter *taker_c = new QCompleter(taker_list,this);
	taker_c->setCaseSensitivity(Qt::CaseInsensitive);
	ui->lineEdit_to->setCompleter(taker_c);
	ui->lineEdit_shipping_search->setFocus();

	ui->tableWidget_shipping_list->setColumnHidden(3,true);
	ship_id=0;
}

ShippingDialog::~ShippingDialog()
{
	QSettings settings(this);
	settings.setValue("shipping_size", size());

	delete ui;
}

void ShippingDialog::changeEvent(QEvent *e)
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

void ShippingDialog::on_toolButton_add_to_shipping_list_customContextMenuRequested(QPoint pos)
{
	//qDebug() << "on_toolButton_add_to_shipping_list_customContextMenuRequested";


}

void ShippingDialog::on_action_add_one_position_triggered()
{
	QApplication::setOverrideCursor(Qt::WaitCursor);
	int ordered_indx = ordered_cb_list.value(ui->comboBox_shipping_candidates->currentIndex());
	QSqlQuery query;
	query.prepare("SELECT orders.id ,ordered.name , ordered.cover , SUM(ordered.count) , SUM(shipped.count), ordered.id,ordered.size FROM ordered LEFT JOIN shipped ON (ordered.id  =  shipped.ordered) LEFT JOIN orders ON (ordered.porder = orders.id) WHERE ordered.id = ? GROUP BY ordered.id ORDER BY ordered.id");
	query.addBindValue(ordered_indx);
	if(query.exec()) {
		while(query.next()) {
			// добавляем строку =;
			int sum = query.value(3).toInt();
			int sum_sh = query.value(4).toInt();
			if (sum - sum_sh > 0) {
				ui->tableWidget_shipping_list->insertRow(ui->tableWidget_shipping_list->rowCount());
				{
					QTableWidgetItem *newItem = new QTableWidgetItem(query.value(0).toString());
					ui->tableWidget_shipping_list->setItem(ui->tableWidget_shipping_list->rowCount()-1, 0, newItem);
				}
				//
				{
					QTableWidgetItem *newItem = new QTableWidgetItem(	query.value(1).toString()+". "+query.value(2).toString()+" "+query.value(6).toString() +" (" + query.value(3).toString() + trUtf8(" шт.) "));
					ui->tableWidget_shipping_list->setItem(ui->tableWidget_shipping_list->rowCount()-1, 1, newItem);
				}
				QSpinBox *cSpBox = new QSpinBox(this);
				cSpBox->setFrame(false);
				/* QObject::connect(cSpBox, SIGNAL(valueChanged(int)),
																	this, SLOT(table_recalculate(int))); */
				cSpBox->setValue(sum - sum_sh);
				cSpBox->setMaximum(sum - sum_sh);
				cSpBox->setMinimum(1);
				this->ui->tableWidget_shipping_list->setCellWidget(ui->tableWidget_shipping_list->rowCount()-1,2,cSpBox);
				ordered_cb_list.remove(ui->comboBox_shipping_candidates->currentIndex());
				ui->comboBox_shipping_candidates->removeItem(ui->comboBox_shipping_candidates->currentIndex());
				QTableWidgetItem *newItemId = new QTableWidgetItem( query.value(5).toString()	);
				ui->tableWidget_shipping_list->setItem(ui->tableWidget_shipping_list->rowCount()-1, 3, newItemId);
			}
		}
	}
	this->ui->tableWidget_shipping_list->resizeColumnsToContents();
	QApplication::restoreOverrideCursor();
}

void ShippingDialog::on_action_add_all_position_triggered()
{
	QApplication::setOverrideCursor(Qt::WaitCursor);
	int ordered_indx = ordered_cb_list.value(ui->comboBox_shipping_candidates->currentIndex());
	QSqlQuery query;
	query.prepare("SELECT orders.id FROM ordered INNER JOIN orders ON (ordered.porder = orders.id) WHERE ordered.id = ? GROUP BY ordered.id ORDER BY ordered.id");
	query.addBindValue(ordered_indx);
	if(query.exec()){
		while(query.next()){
			QSqlQuery q;
			q.prepare("SELECT orders.id ,ordered.name , ordered.cover , SUM(ordered.count) , SUM(shipped.count), ordered.id FROM ordered LEFT JOIN shipped ON (ordered.id  =  shipped.ordered) LEFT JOIN orders ON (ordered.porder = orders.id) WHERE orders.id = ? GROUP BY ordered.id ORDER BY ordered.id");
			q.addBindValue(query.value(0).toInt());
			if(q.exec()){
				while(q.next()){
					//
					// добавляем строку =;
					int sum = q.value(3).toInt();
					int sum_sh = q.value(4).toInt();
					if (sum - sum_sh > 0) {
						ui->tableWidget_shipping_list->insertRow(ui->tableWidget_shipping_list->rowCount());
						{
							QTableWidgetItem *newItem = new QTableWidgetItem(q.value(0).toString());
							ui->tableWidget_shipping_list->setItem(ui->tableWidget_shipping_list->rowCount()-1, 0, newItem);
						}
						//
						{
							QTableWidgetItem *newItem = new QTableWidgetItem(	q.value(1).toString()+". "+q.value(2).toString()  +" (" + q.value(3).toString() + trUtf8(" шт.) "));
							ui->tableWidget_shipping_list->setItem(ui->tableWidget_shipping_list->rowCount()-1, 1, newItem);
						}
						QSpinBox *cSpBox = new QSpinBox(this);
						cSpBox->setFrame(false);
						/* QObject::connect(cSpBox, SIGNAL(valueChanged(int)),
																			this, SLOT(table_recalculate(int))); */
						cSpBox->setValue(sum - sum_sh);
						cSpBox->setMaximum(sum - sum_sh);
						cSpBox->setMinimum(1);
						this->ui->tableWidget_shipping_list->setCellWidget(ui->tableWidget_shipping_list->rowCount()-1,2,cSpBox);
						// удаление связных компонетов
						int c_i = ordered_cb_list.indexOf(q.value(5).toInt());
						ordered_cb_list.remove(c_i);
						ui->comboBox_shipping_candidates->removeItem(c_i);

						QTableWidgetItem *newItemId = new QTableWidgetItem( q.value(5).toString()	);
						ui->tableWidget_shipping_list->setItem(ui->tableWidget_shipping_list->rowCount()-1, 3, newItemId);
					}

				}

			}else{
				//qDebug() << q.lastError();

			}
		}
	}else {
		//qDebug() << query.lastError();
	}
	this->ui->tableWidget_shipping_list->resizeColumnsToContents();
	QApplication::restoreOverrideCursor();
}

void ShippingDialog::on_toolButton_add_to_shipping_list_clicked()
{
		on_action_add_all_position_triggered();
}

void ShippingDialog::on_toolButton_new_shipping_act_clicked()
{
	ui->dateTimeEdit_ship->setDateTime(QDateTime::currentDateTime());
	ui->lineEdit_to->setText("");
	ui->textEdit_shipping_note->setPlainText("");
	ui->tableWidget_shipping_list->setRowCount(0);

	QSqlQuery query;
	query.prepare("INSERT INTO shipping(dt,taker,notes) VALUES('"+QDateTime::currentDateTime().toString("yyyy-MM-ddThh:mm:ss")+"','','')");
	if(query.exec()) {
		if(query.lastInsertId().isValid()) {
			ship_id=query.lastInsertId().toInt();
			QString s;
			s.setNum(ship_id);
			ui->label_shipping_id->setText(s);
		}
	}

	on_toolButton_shipping_search_clicked();

}

void ShippingDialog::on_actionSave_shipping_act_triggered()
{
	QApplication::setOverrideCursor(Qt::WaitCursor);
	// id integer primary key AUTOINCREMENT, dt datetime,taker varchar, notes varchar
	// id integer primary key AUTOINCREMENT, `shipping` integer, `ordered` integer,`count` integer
	if(ship_id < 1) {
		QSqlQuery query;
		query.prepare("INSERT INTO shipping(dt,taker,notes) VALUES('"+QDateTime::currentDateTime().toString("yyyy-MM-ddThh:mm:ss")+"','','')");
		if(query.exec()) {
			if(query.lastInsertId().isValid()) {
				ship_id=query.lastInsertId().toInt();
				QString s;
				s.setNum(ship_id);
				ui->label_shipping_id->setText(s);
			}
		}
	}
	QSqlQuery query;
	query.prepare("UPDATE shipping SET dt = ? ,taker= ? , notes = ? WHERE id = ? ");
	query.addBindValue(ui->dateTimeEdit_ship->dateTime());
	query.addBindValue(ui->lineEdit_to->text());
	query.addBindValue(ui->textEdit_shipping_note->toPlainText());
	query.addBindValue(ship_id);
	if(query.exec()) {
		{
			QSqlQuery q;
			q.prepare("DELETE FROM shipped WHERE shipping = ? ");
			q.addBindValue(ship_id);
			q.exec();
		}
		for(int i=0;i < ui->tableWidget_shipping_list->rowCount(); i++) {
			QSqlQuery q;

			q.prepare("INSERT INTO shipped (shipping,ordered,count) VALUES(?,?,?)");
			q.addBindValue(ship_id);
			q.addBindValue( ui->tableWidget_shipping_list->item(i,3)->text() );
			QSpinBox *qsb = static_cast<QSpinBox *>(ui->tableWidget_shipping_list->cellWidget(i,2));
			if (qsb) {
				q.addBindValue( qsb->value() );
			}
			q.exec();
		}

	}
	QApplication::restoreOverrideCursor();

	on_toolButton_shipping_search_clicked();

}

void ShippingDialog::on_toolButton_save_shipping_act_clicked()
{
		on_actionSave_shipping_act_triggered();

}
void  ShippingDialog::load_shipping_act(int id)
{
//
	QApplication::setOverrideCursor(Qt::WaitCursor);

	ship_id=id;
	QString s;
	s.setNum(ship_id);
	ui->label_shipping_id->setText(s);
	QSqlQuery query;
	query.prepare("SELECT * FROM shipping WHERE id = ? ");
	query.addBindValue(ship_id);
	if(query.exec()) {
		while(query.next()) {
			ui->dateTimeEdit_ship->setDateTime(query.value(1).toDateTime());
			ui->lineEdit_to->setText(query.value(2).toString());
			ui->textEdit_shipping_note->setPlainText(query.value(3).toString());
		}
		QSqlQuery q;
		q.prepare("SELECT `id`,`shipping`,`ordered`,`count` FROM shipped WHERE shipping = ? ");
		q.addBindValue(ship_id);
		ui->tableWidget_shipping_list->setRowCount(0);
		if(q.exec()){
			while(q.next()){
				ui->tableWidget_shipping_list->insertRow(ui->tableWidget_shipping_list->rowCount());

				QSqlQuery qr;
				qr.prepare("SELECT orders.id ,ordered.name , ordered.cover ,SUM(ordered.count) , SUM(shipped.count), ordered.id,ordered.size FROM ordered LEFT JOIN shipped ON (ordered.id  =  shipped.ordered) LEFT JOIN orders ON (ordered.porder = orders.id) WHERE ordered.id = ? GROUP BY ordered.id ORDER BY ordered.id");
				//qDebug() <<q.value(1).toString();
				//qDebug() <<q.value(2).toString();
				qr.addBindValue(q.value(2).toString());

				if(qr.exec()) {
					// номер заказа
					qr.next();

					QTableWidgetItem *newItemOrdNum = new QTableWidgetItem(qr.value(0).toString());
					ui->tableWidget_shipping_list->setItem(ui->tableWidget_shipping_list->rowCount()-1, 0, newItemOrdNum);

					// содержимое заказа
					QTableWidgetItem *newItemSC = new QTableWidgetItem(	qr.value(1).toString()+". "+qr.value(2).toString()+". "+qr.value(6).toString() );
					ui->tableWidget_shipping_list->setItem(ui->tableWidget_shipping_list->rowCount()-1, 1, newItemSC);
					// управление количеством
					QSpinBox *cSpBox = new QSpinBox(this);
					cSpBox->setFrame(false);
					int sum = qr.value(3).toInt();

					cSpBox->setMaximum(sum );
					cSpBox->setMinimum(1);
					cSpBox->setValue(q.value(3).toInt());
					this->ui->tableWidget_shipping_list->setCellWidget(ui->tableWidget_shipping_list->rowCount()-1,2,cSpBox);
					// номер позиции заказа
					QTableWidgetItem *newItemId = new QTableWidgetItem( qr.value(5).toString()	);
					ui->tableWidget_shipping_list->setItem(ui->tableWidget_shipping_list->rowCount()-1, 3, newItemId);
				}else {
					//qDebug() << qr.lastError();
				}
			}
		}else {
			//qDebug() << q.lastError();
		}
	}else {
		//qDebug() << query.lastError();
	}
	this->ui->tableWidget_shipping_list->resizeColumnsToContents();
	ui->tableWidget_shipping_list->hideColumn(3);
	QApplication::restoreOverrideCursor();
};


void ShippingDialog::on_toolButton_shipping_search_clicked()
{
	QApplication::setOverrideCursor(Qt::WaitCursor);

	QSqlQueryModel * model = new QSqlQueryModel(0);
	// id integer primary key AUTOINCREMENT, dt datetime,taker varchar, notes varchar
	QString sql = "SELECT id,strftime('%d.%m.%Y %H:%M',dt),taker,notes FROM shipping ";

	if (! ui->lineEdit_shipping_search->text().isEmpty()) {
		sql += "WHERE dt LIKE '%"+ ui->lineEdit_shipping_search->text() +"%' OR ";
		sql += " taker LIKE '%"+ ui->lineEdit_shipping_search->text() +"%' OR ";
		sql += " notes LIKE '%"+ ui->lineEdit_shipping_search->text() +"%'  ";
	}
	sql += " ORDER BY dt DESC LIMIT 100";
	model->setQuery(sql);

	model->setHeaderData(0, Qt::Horizontal, QObject::trUtf8("№ акта"));
	model->setHeaderData(1, Qt::Horizontal, QObject::trUtf8("Дата"));
	model->setHeaderData(2, Qt::Horizontal, QObject::trUtf8("Кому отгружено"));
	model->setHeaderData(3, Qt::Horizontal, QObject::trUtf8("Примечания"));

	QSortFilterProxyModel* sfpm = new QSortFilterProxyModel(ui->tableView_shipping);
	sfpm->setSourceModel(model);
	ui->tableView_shipping->setModel(sfpm);



	ui->tableView_shipping->resizeColumnsToContents();
	ui->tableView_shipping->resizeRowsToContents();
	QApplication::restoreOverrideCursor();
}

void ShippingDialog::on_tableView_shipping_activated(QModelIndex index)
{
	load_shipping_act(index.sibling(index.row(),0).data(0).toInt());
	ui->tabWidget->setCurrentIndex(0);
}

void ShippingDialog::set_current_order(int o, QString customer) {
	this->current_order = o;
	ui->lineEdit_to->setText(customer);
	QString s;
	s.setNum(o);
	s +=". ";
	//qDebug() << s;
	int fp = ui->comboBox_shipping_candidates->findText(s,Qt::MatchStartsWith);

	if(fp>0) ui->comboBox_shipping_candidates->setCurrentIndex(fp);
};
void ShippingDialog::set_shipping_tab(int id){
	ui->tabWidget->setCurrentIndex(id);
};
void ShippingDialog::on_toolButton_print_shipping_act_clicked()
{
		//
	QString htm,s;
	htm="<html><head><title></title></head><body style='font-family: Times New Roman;'>";
	htm += trUtf8("<table align=center width=100% cellspacing=0 cellpadding=2 border=0><tbody>");
	htm += trUtf8("<tr><td style='font-family: Georgia;font-weight: bold;font-style: italic'>E-mail: <a href='mailto:fabrikasnov@mail.ru'>fabrikasnov@mail.ru</a><br><a href='http://www.fabrikasnov.com/'>www.FabrikaSnov.com</a><br>тел.:8(928)304-304-8; 8(8652)233-669</td><td align=right ><img src=\":/&lt;root&gt;/icons/fab.png\" height=45 width=191>");
	//htm += trUtf8("<h1 style='font-family: SC_Solnyshko, Times New Roman;font-style: italic;padding-bottom: 0px;margin-bottom: 0px'>Фабрика Снов</h1><h5 style='margin-top: 0px;padding-top: 0px'>Orthopedic mattress</h5>");
	htm += trUtf8("</td></tr></tbody>");
	htm += trUtf8("</table>");
	htm += trUtf8("<hr style='margin: 0px;padding: 0px;'>");
	htm += trUtf8("<p style='font-family: Georgia;font-weight: bold;font-style: italic;margin: 0px;padding: 0px;'>355000 г. Ставрополь, ул. Мичурина, 55а</p>");

	s.setNum(ship_id);
	htm += trUtf8("<h1 align='center'>Акт отгрузки № ")+s+"</h1>";
	htm += trUtf8("<table width=100%><tr><td>Отгружено: ")+ui->lineEdit_to->text()+trUtf8("</td><td align=right>Дата/время: ")+ui->dateTimeEdit_ship->dateTime().toString("dd.MM.yyyy H:m")+"</td></tr></table>";
	htm += trUtf8("<table align=center width=100% border=1 cellspacing=-1 cellpadding=2 style='border-color: #000000'><thead><tr>");
	htm += trUtf8("<th width='50'>№заказа</th><th>Состав позиции</th><th width='50'>Кол-во</th></tr></thead><tbody>");

	for(int i=0;i < ui->tableWidget_shipping_list->rowCount(); i++) {
		htm +="<tr><td align=center>"+		ui->tableWidget_shipping_list->item(i,0)->text() + "</td><td>";
		htm += ui->tableWidget_shipping_list->item(i,1)->text() +  + "</td><td align=center>";

		QSpinBox *qsb = static_cast<QSpinBox *>(ui->tableWidget_shipping_list->cellWidget(i,2));
		if (qsb) {
			s.setNum(qsb->value());
			htm +=s;
		}
		htm +="</td></tr>";

	}

	htm += trUtf8("</tbody>");
	htm += trUtf8("</table>");
	htm += trUtf8("<p>Примечания: ")+ui->textEdit_shipping_note->toPlainText()+"</p>";
	htm += trUtf8("</body>");
	htm += trUtf8("</html>");

	DialogPrintPreview prv;
	prv.set_doc(htm);
	prv.exec();
}

void ShippingDialog::on_toolButton_remove_from_shipping_list_clicked()
{
		//
	if(ui->tableWidget_shipping_list->currentRow() > -1 ) {
		ui->tableWidget_shipping_list->removeRow(ui->tableWidget_shipping_list->currentRow());
	}
}
