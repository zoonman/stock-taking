#include <QtSql>
#include <QMessageBox>

#include "reportdialog.h"
#include "ui_reportdialog.h"
#include "dialogprintpreview.h"

ReportDialog::ReportDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::ReportDialog)
{
    ui->setupUi(this);



		//move(settings.value("rep_pos", QPoint(10, 10)).toPoint());


		ui->dateTimeEdit_report_end->setDateTime(QDateTime::currentDateTime());
		ui->dateTimeEdit_report_begin->setDateTime(QDateTime::currentDateTime().addMonths(-1));
		QSqlQueryModel * model = new QSqlQueryModel(this);

		model->setQuery("SELECT maker,id FROM makers  ORDER BY id");

		ui->listView_makers->setModel(model);

}

ReportDialog::~ReportDialog()
{
    delete ui;
}

void ReportDialog::changeEvent(QEvent *e)
{
    QDialog::changeEvent(e);
    switch (e->type()) {
    case QEvent::LanguageChange:
        ui->retranslateUi(this);
        break;
    default:
        break;
    }
		ui->groupBox_client_selector->setEnabled(false);
}

void ReportDialog::on_buttonBox_accepted()
{
	QApplication::setOverrideCursor(Qt::WaitCursor);
	QString htm,rpt_h;
	QSettings settings;
	rpt_h = settings.value("all_report_header", "").toString();
	if(rpt_h.length() > 0) {
		htm+=rpt_h;
	}
	else {
		htm="<html><head><title>Отчет</title><meta http-equiv=\"Content-Type\" content=\"text/html; charset=UTF-8\" /></head><body style='font-family: Old Standard TT;'>";
		htm += trUtf8("<table align=center width=100% cellspacing=0 cellpadding=2 border=0><tbody>");
		htm += trUtf8("<tr><td style='font-family: Georgia;font-weight: bold;font-style: italic'>E-mail: <a href='mailto:fabrikasnov@mail.ru'>fabrikasnov@mail.ru</a><br><a href='http://www.fabrikasnov.com/'>www.FabrikaSnov.com</a><br>тел.:8(928)304-304-8; 8(8652)233-669</td><td align=right><img src=\":/&lt;root&gt;/icons/fab.png\" height=45 width=191>");
		//htm += trUtf8("<h1 style=\" font-family:'SC_Solnyshko'; padding-bottom: 0px;margin-bottom: 0px\"><span style=\" font-family:'SC_Solnyshko'; font-size:8pt;\">okok Фабрика Снов</span></h1><h5 style='margin-top: 0px;padding-top: 0px'>Orthopedic mattress</h5>");
		htm += trUtf8("</td></tr></tbody>");
		htm += trUtf8("</table>");
		htm += trUtf8("<hr style='margin: 0px;padding: 0px;'>");
		htm += trUtf8("<p style='font-family: Georgia;font-weight: bold;font-style: italic;margin: 0px;padding: 0px;'>355000 г. Ставрополь, ул. Мичурина, 55а</p>");
	}

	QModelIndexList makers = ui->listView_makers->selectionModel()->selectedIndexes();
	QStringList ml;
	for(int mi=0;mi<makers.count();mi++){
		//clients <<	mil.at(i).model()->data(mil.at(i).model()->index(mil.at(i).row(),1)).toInt() ;
		if (makers.at(mi).column() == 1) continue;
		ml << makers.at(mi).model()->data(makers.at(mi).model()->index(makers.at(mi).row(),1)).toString() ;
	}
	QString sql_makers="";
	if(ml.length() > 0 ) {sql_makers = ml.join(",");
		sql_makers=" AND `orders`.`maker` IN ("+sql_makers+") ";
	}





	switch(ui->comboBox_report_selector->currentIndex()){
	case 0:{
			QSqlQuery query;
			//
			query.prepare("SELECT `ordered`.`name` || '. ' || `ordered`.`cover`,`ordered`.`size`,`ordered`.`count`,round(`ordered`.`price`*(100-`ordered`.`discount`)*0.01),`ordered`.`count` * round(`ordered`.`price` * (100-`ordered`.`discount`)*0.01),`orders`.`id`,"
										" customers.fio "
										" FROM orders INNER JOIN `ordered` ON (`orders`.`id` = `ordered`.`porder`) "
										" LEFT JOIN customers ON (`orders`.`customer` = `customers`.`id`) "
										"WHERE `orders`.`dt` BETWEEN ? AND ? "+sql_makers+" ORDER BY dt DESC");
			query.addBindValue(ui->dateTimeEdit_report_begin->dateTime());
			query.addBindValue(ui->dateTimeEdit_report_end->dateTime());
			htm += trUtf8("<h3 align=center>Отчет продаж</h3>");
			htm += trUtf8("<table align=center width=100% border=1 cellspacing=-1 cellpadding=2 style='border-color: #000000;border-collapse: collapse'>");
			htm += trUtf8("<thead><tr>");
			htm += trUtf8("<th>№ з.</th><th>Покупатель</th><th>Наименование</th><th>Размер</th><th>Кол-во</th><th>Цена за единицу</th><th>Сумма оплаты</th><th>Примечание</th>");
			htm += trUtf8("</tr></thead>");
			query.exec();

			while (query.next()) {

				htm += trUtf8("<tr>");
				htm += trUtf8("<td>") +query.value(5).toString()+ trUtf8("</td>");
				htm += trUtf8("<td>") +query.value(6).toString()+ trUtf8("</td>");
				htm += trUtf8("<td>") +query.value(0).toString()+ trUtf8("</td>");
				htm += trUtf8("<td align=center>") +query.value(1).toString()+ trUtf8("</td>");
				htm += trUtf8("<td align=center>") +query.value(2).toString()+ trUtf8("</td>");
				htm += trUtf8("<td align=right>") +query.value(3).toString()+ trUtf8("</td>");
				htm += trUtf8("<td align=right>") +query.value(4).toString()+ trUtf8("</td>");

				htm += trUtf8("<td>&nbsp;</td>");

				htm += trUtf8("</tr>");
			}

			htm += trUtf8("</table>");
			htm += trUtf8("<table width=100%><tr><td width=20 align=right>Сдал:</td><td style='padding-top: 5px'><hr size=1 noshade></td><td width=40 align=right>Принял:</td><td style='padding-top: 5px'><hr size=1 noshade></td></tr></table>");
		};break;

		/* */
	case 1:{
			QSqlQuery query;
			int summ=0;
			//
			query.prepare("SELECT strftime('%d.%m.%Y',`orders`.dt),customers.fio, "
										" sum(`ordered`.`count` * round(`ordered`.`price` * (100-`ordered`.`discount`)*0.01)) "
										" FROM	orders  "
										" INNER JOIN customers ON (orders.customer=customers.id) "
										" INNER JOIN ordered ON (orders.id=ordered.porder) WHERE `orders`.`dt` BETWEEN ? AND ?  "+sql_makers+" GROUP BY strftime('%Y.%m.%d',`orders`.dt),customers.fio");
			query.addBindValue(ui->dateTimeEdit_report_begin->dateTime());
			query.addBindValue(ui->dateTimeEdit_report_end->dateTime());
			htm += trUtf8("<h2 align=center>Список заявок за период с ") +ui->dateTimeEdit_report_begin->dateTime().date().toString("dd.MM.yyyy")+trUtf8(" по ") +ui->dateTimeEdit_report_end->dateTime().date().toString("dd.MM.yyyy")+"</h2>";
			htm += trUtf8("<table align=center width=100% border=1 cellspacing=-1 cellpadding=2 style='border-color: #000000;border-collapse: collapse'>");
			htm += trUtf8("<thead><tr>");
			htm += trUtf8("<th width=50>Дата</th><th>Партнер</th><th>Сумма</th");
			htm += trUtf8("</tr></thead>");
			query.exec();
			while (query.next()) {
				htm += trUtf8("<tr>");
				htm += trUtf8("<td align=center>") +query.value(0).toString()+ trUtf8("</td>");
				htm += trUtf8("<td>") +query.value(1).toString()+ trUtf8("</td>");
				htm += trUtf8("<td align=right>") +query.value(2).toString()+ trUtf8("</td>");
				summ+=query.value(2).toInt();




				htm += trUtf8("</tr>");
			}
			htm += trUtf8("<tr>");
			QString s;
			s.setNum(summ);
			htm += trUtf8("<td colspan=2><b>Итого:</b></td><td align=right><b>") +s+ trUtf8("</b></td>");
			htm += trUtf8("</tr>");
			htm += trUtf8("</table>");

		};break;
	/* отчет продаж розница за период  */
	case 2:
		{

		}
	case 4:
		{

			//
			QString pre;
			QSqlQuery query;
			QString sql="SELECT strftime('%d.%m.%Y',`orders`.dt),customers.fio, "
									" orders.* "
									" FROM	orders  "
									" INNER JOIN customers ON (orders.customer=customers.id AND ";
			if (ui->comboBox_report_selector->currentIndex() == 2) {
				sql +="customers.wholesaler<2";
				pre = trUtf8("розница");
			}
			else {
				sql +="customers.wholesaler=2";
				pre = trUtf8("опт");
			}

			htm += trUtf8("<h2 align=center>Отчет продаж ")+pre+trUtf8(" за период с ") +ui->dateTimeEdit_report_begin->dateTime().date().toString("dd.MM.yyyy")+trUtf8(" по ") +ui->dateTimeEdit_report_end->dateTime().date().toString("dd.MM.yyyy")+"</h2>";

			query.prepare( sql + " ) "
										" WHERE `orders`.`dt` BETWEEN ? AND ?  "+sql_makers+" GROUP BY orders.id ORDER BY strftime('%d.%m.%Y',`orders`.dt),customers.fio");
			query.addBindValue(ui->dateTimeEdit_report_begin->dateTime());
			query.addBindValue(ui->dateTimeEdit_report_end->dateTime());
			if(query.exec()){
				htm += trUtf8("<table align=center width=100% border=1 cellspacing=-1 cellpadding=2 style='border-color: #000000;border-collapse: collapse'>");
				htm += trUtf8("<thead><tr>");
				htm += trUtf8("<th width=50 rowspan=2>Дата</th><th rowspan=2>Заказ</th><th colspan=8>Состав заказа</th><th rowspan=2>Сумма к оплате</th><th rowspan=2>Оплачено</th><th rowspan=2>Долг</th><th rowspan=2>Дата отгрузки</th>");
				htm += trUtf8("<tr><td>Наименование</td><td>Чехол</td><td>Размер</td><td>Цена розн</td><td>Скидка</td><td>Цена</td><td>Кол-во</td><td>Стоимость</td></tr>");
				htm += trUtf8("</tr></thead><tbody>");
				float summ_total=0;
				float summ_paid=0;
				float summ_credit=0;
				while(query.next()){

					float summ=0;
					QStringList htl;
					QStringList shippings;

					QSqlQuery q;
					q.prepare("SELECT ordered.id,ordered.name,ordered.cover,ordered.size,round(ordered.price),ordered.discount,"
										" ordered.count,shipping.taker,strftime('%d.%m.%Y',shipping.dt), "
										" SUM(`ordered`.`count` * round(`ordered`.`price` * (100-`ordered`.`discount`)*0.01)) as sm, "
										"  round(`ordered`.`price` * (100-`ordered`.`discount`)*0.01) as oneprice "
										" FROM ordered "
										" LEFT JOIN shipped   ON (ordered.id  =  shipped.ordered) "
										" LEFT JOIN shipping  ON (shipping.id =  shipped.shipping) "
										"	WHERE ordered.porder = ?  GROUP BY ordered.id");
					q.addBindValue(query.value(2).toString());
					//
					int c=0;
					if(q.exec()){
						while(q.next()){
							QString hths="";
							hths += trUtf8("<td>") +q.value(1).toString()+ trUtf8("</td>");
							hths += trUtf8("<td>") +q.value(2).toString()+ trUtf8("</td>");
							hths += trUtf8("<td align=center>") +q.value(3).toString()+ trUtf8("</td>");
							hths += trUtf8("<td align=right>") +q.value(4).toString()+ trUtf8("</td>");
							hths += trUtf8("<td align=right>") +q.value(5).toString()+ trUtf8("</td>");
							hths += trUtf8("<td align=right>") +q.value(10).toString()+ trUtf8("</td>");
							hths += trUtf8("<td align=center>") +q.value(6).toString()+ trUtf8("</td>");
							hths += trUtf8("<td align=right>") +q.value(9).toString()+ trUtf8("</td>");
							summ +=q.value(9).toInt();
							if (!q.value(8).toString().isEmpty()) {
								shippings <<q.value(8).toString();
							}
							htl << hths;
							c++;
						}
					}

					summ += query.value(12).toFloat();
					summ += query.value(15).toFloat();
					htm += trUtf8("<tr>");
					QString htr="", s="";
					summ_total +=summ;


					s.setNum(summ,'f',2);
					htr.setNum(c);
					htm += trUtf8("<td align=center rowspan=")+htr +">" +query.value(0).toString()+ trUtf8("</td>");
					htm += trUtf8("<td align=center rowspan=")+htr +">" +query.value(2).toString()+ trUtf8("</td>");
					if (htl.length()>0)
					htm += htl.at(0);

//					Сумма к оплате
					htm += trUtf8("<td align=right rowspan=")+htr +">" +s+ trUtf8("</td>");
//					Оплачено
					htm += trUtf8("<td align=right rowspan=")+htr +">" +query.value(16).toString()+ trUtf8("</td>");
					summ_paid += query.value(16).toFloat();
					;
//					Долг
					summ -= query.value(16).toFloat();
					if(summ < 0) summ=0;
					summ_credit +=summ;
					s.setNum(summ,'f',2);
					htm += trUtf8("<td align=right rowspan=")+htr +">" +s+ trUtf8("</td>");
//					Дата отгрузки
					shippings.removeDuplicates();
					htm += trUtf8("<td align=right rowspan=")+htr +">" +shippings.join("<br>")+ trUtf8("</td>");
					htm += trUtf8("</tr>");

					for(int i=1;i< htl.size();i++){
						htm += trUtf8("<tr>") + htl.at(i) + trUtf8("</tr>");
					}
					htl.clear();
				}
				QString s;
				s.setNum(summ_total,'f',2);
				htm += trUtf8("<tr><td colspan=10><b>Итого</b></td><td align=right><b>") + s + "</b></td><td align=right><b>";
				s.setNum(summ_paid,'f',2);
				htm += s + "</b></td><td align=right><b>";
				s.setNum(summ_credit,'f',2);
				htm += s + "</b></td></tr>";
				htm += trUtf8("</tbody></table>");
			}

		};break;
	case 3:
	case 5:
		{

			// qDebug() <<  ui->listView_clients->model();
			QString pre;
			if (ui->comboBox_report_selector->currentIndex() == 3) {
				pre = trUtf8("розница");
			}
			else {
				pre = trUtf8("опт");
			}

			htm += trUtf8("<h2 align=center>Отчет продаж ")+pre+trUtf8(" по клиентам за период с ") +ui->dateTimeEdit_report_begin->dateTime().date().toString("dd.MM.yyyy")+trUtf8(" по ") +ui->dateTimeEdit_report_end->dateTime().date().toString("dd.MM.yyyy")+"</h2>";

			QModelIndexList mil = ui->listView_clients->selectionModel()->selectedIndexes();

			for(int i=0;i<mil.count();i++){
				//clients <<	mil.at(i).model()->data(mil.at(i).model()->index(mil.at(i).row(),1)).toInt() ;
				if (mil.at(i).column() == 1) continue;
				int custNo = mil.at(i).model()->data(mil.at(i).model()->index(mil.at(i).row(),1)).toInt() ;






				QSqlQuery query;
				QString sql="SELECT strftime('%Y.%m.%d',`orders`.dt),customers.fio, "
										" orders.* "
										" FROM	orders  "
										" INNER JOIN customers ON (orders.customer=customers.id AND ";
				if (ui->comboBox_report_selector->currentIndex() == 3) {
					sql +="customers.wholesaler<2";
				}
				else {
					sql +="customers.wholesaler=2";
				}
				query.prepare( sql + " ) "
											 " WHERE (`orders`.`customer` = ?)  AND (`orders`.`dt` BETWEEN ? AND ? ) "+sql_makers+" GROUP BY orders.id ORDER BY strftime('%Y.%m.%d',`orders`.dt),customers.fio");
				query.addBindValue(custNo);
				query.addBindValue(ui->dateTimeEdit_report_begin->dateTime());
				query.addBindValue(ui->dateTimeEdit_report_end->dateTime());
				if(query.exec()){
					int rown=0;
					//if (query.size() > 0)
					{

						float summ_total=0,summ_paid=0,summ_credit=0;

						while(query.next()){
							if (rown==0) {
								htm += "<h3>";
								if (ui->comboBox_report_selector->currentIndex() == 3) {htm +=trUtf8("Частное лицо: ");}else {htm +=trUtf8("Клиент: ");}
								htm += mil.at(i).data().toString()+"</h3>";
								htm += trUtf8("<table align=center width=100% border=1 cellspacing=-1 cellpadding=2 style='border-color: #000000;border-collapse: collapse'>");
								htm += trUtf8("<thead><tr>");

								if (ui->comboBox_report_selector->currentIndex() == 3){
									htm += trUtf8("<th width=50 rowspan=2>Дата</th><th rowspan=2>Заказ</th><th colspan=8>Состав заказа</th><th rowspan=2>Сумма к оплате</th><th rowspan=2>Оплачено</th><th rowspan=2>Долг</th><th rowspan=2>Дата отгрузки</th>");


								}
								else {
									htm += trUtf8("<th width=50 rowspan=2>Дата</th><th rowspan=2>Заказ</th><th colspan=8>Состав заказа</th><th rowspan=2>Сумма к оплате</th><th rowspan=2>Дата отгрузки</th>");
								}
								htm += trUtf8("<tr><td>Наименование</td><td>Чехол</td><td>Размер</td><td>Цена розн</td><td>Скидка</td><td>Цена</td><td>Кол-во</td><td>Стоимость</td></tr>");
								htm += trUtf8("</tr></thead><tbody>");
								rown=1;
							}

							float summ=0;
							QStringList htl;
							QStringList shippings;
							QSqlQuery q;
							q.prepare("SELECT ordered.id,ordered.name,ordered.cover,ordered.size,round(ordered.price),ordered.discount,"
												" ordered.count,shipping.taker,strftime('%d.%m.%Y',shipping.dt), "
												" round(SUM(`ordered`.`count` * (`ordered`.`price` * (100-`ordered`.`discount`)*0.01))) as sm, "
												"  round(`ordered`.`price` * (100-`ordered`.`discount`)*0.01) as oneprice "
												" FROM ordered "
												" LEFT JOIN shipped   ON (ordered.id  =  shipped.ordered) "
												" LEFT JOIN shipping  ON (shipping.id =  shipped.shipping) "
												"	WHERE ordered.porder = ? GROUP BY ordered.id");
							q.addBindValue(query.value(2).toString());
							//
							int c=0;
							if(q.exec()){
								while(q.next()){
									QString hths="";
									hths += trUtf8("<td>") +q.value(1).toString()+ trUtf8("</td>");
									hths += trUtf8("<td>") +q.value(2).toString()+ trUtf8("</td>");
									hths += trUtf8("<td align=center>") +q.value(3).toString()+ trUtf8("</td>");
									hths += trUtf8("<td align=right>") +q.value(4).toString()+ trUtf8("</td>");
									hths += trUtf8("<td align=right>") +q.value(5).toString()+ trUtf8("</td>");
									hths += trUtf8("<td align=right>") +q.value(10).toString()+ trUtf8("</td>");
									hths += trUtf8("<td align=center>") +q.value(6).toString()+ trUtf8("</td>");
									hths += trUtf8("<td align=right>") +q.value(9).toString()+ trUtf8("</td>");
									summ +=q.value(9).toInt();
									if (!q.value(8).toString().isEmpty()) {
										shippings <<q.value(8).toString();
									}
									htl << hths;
									c++;
								}
							}
							summ += query.value(12).toFloat();
							summ += query.value(15).toFloat();

							htm += trUtf8("<tr>");
							QString htr="", s="";
							summ_total +=summ;
							s.setNum(summ,'f',2);
							htr.setNum(c);
							htm += trUtf8("<td align=center rowspan=")+htr +">" +query.value(0).toString()+ trUtf8("</td>");
							htm += trUtf8("<td align=center rowspan=")+htr +">" +query.value(2).toString()+ trUtf8("</td>");
							htm += htl.at(0);
							//					Сумма к оплате
							htm += trUtf8("<td align=right rowspan=")+htr +">" +s+ trUtf8("</td>");

							if (ui->comboBox_report_selector->currentIndex() == 3)	{
								//					Оплачено
								htm += trUtf8("<td align=right rowspan=")+htr +">" +query.value(16).toString()+ trUtf8("</td>");
								summ_paid += query.value(16).toFloat();
								;
								//					Долг
								summ -= query.value(16).toFloat();
								if(summ < 0) summ=0;
								summ_credit +=summ;
								s.setNum(summ,'f',2);
								htm += trUtf8("<td align=right rowspan=")+htr +">" +s+ trUtf8("</td>");
							}

							//					Дата отгрузки
							shippings.removeDuplicates();
							htm += trUtf8("<td align=right rowspan=")+htr +">" +shippings.join("<br>")+ trUtf8("</td>");
							htm += trUtf8("</tr>");
							for(int i=1;i< htl.size();i++){
								htm += trUtf8("<tr>") + htl.at(i) + trUtf8("</tr>");
							}
							htl.clear();
						}
						if(rown==1) {
							QString s;
							s.setNum(summ_total,'f',2);
							if (ui->comboBox_report_selector->currentIndex() == 3)	{
								htm += trUtf8("<tr><td colspan=10><b>Итого</b></td><td align=right><b>") + s + "</b></td><td align=right><b>";
								s.setNum(summ_paid,'f',2);
								htm += s + "</b></td><td align=right><b>";
								s.setNum(summ_credit,'f',2);
								htm += s + "</b></td></tr>";
							}
							else {
								htm += trUtf8("<tr><td colspan=10><b>Итого</b></td><td align=right><b>") + s + "</b></td></tr>";

							}
							htm += trUtf8("</tbody></table>");
						}
					}
					/*else{
						//htm += trUtf8("Нет заказов");
					}*/
				}
			}

		};
		break;
	case 6:{
			//
			QSqlQuery query;
			query.prepare("SELECT ordered.name,ordered.cover,ordered.size,COUNT(*) AS cnt FROM ordered INNER JOIN orders ON (orders.id=ordered.porder) "
										" WHERE `orders`.`dt` BETWEEN ? AND ? "+sql_makers+" GROUP BY ordered.name,ordered.cover,ordered.size ORDER BY cnt DESC, ordered.name,ordered.cover,ordered.size LIMIT 100");
			query.addBindValue(ui->dateTimeEdit_report_begin->dateTime());
			query.addBindValue(ui->dateTimeEdit_report_end->dateTime());
			if(query.exec()){
				htm += trUtf8("<table align=center width=100% border=1 cellspacing=-1 cellpadding=2 style='border-color: #000000;border-collapse: collapse'>");
				htm += trUtf8("<thead>");
				htm += trUtf8("<tr><th>Наименование</th><th>Чехол</th><th>Размер</th><th>Кол-во</th>");
				htm += trUtf8("</tr></thead><tbody>");
				while(query.next()){
					htm += trUtf8("<tr><td>") +query.value(0).toString()+ trUtf8("</td>");
					htm += trUtf8("<td>") +query.value(1).toString()+ trUtf8("</td>");
					htm += trUtf8("<td align=center>") +query.value(2).toString()+ trUtf8("</td>");
					htm += trUtf8("<td align=center>") +query.value(3).toString()+ trUtf8("</td></tr>");
				}
				htm += trUtf8("</tbody></table>");
			}else{
				qDebug() << query.lastError();
			}
		}
		break;
	case 7:{

			htm += trUtf8("<h2 align=center>Отчет отгрузок ")+trUtf8("  за период с ") +ui->dateTimeEdit_report_begin->dateTime().date().toString("dd.MM.yyyy")+trUtf8(" по ") +ui->dateTimeEdit_report_end->dateTime().date().toString("dd.MM.yyyy")+"</h2>";

			QSqlQuery query;
			query.prepare("	SELECT shipping.id,strftime('%d.%m.%Y %H:%M',shipping.dt),makers.maker,shipping.taker,orders.id,ordered.name,ordered.cover,ordered.size,shipped.count,"
										" shipped.`count` * round(`ordered`.`price` * (100-`ordered`.`discount`)*0.01) FROM shipped "
										" INNER JOIN shipping ON (shipped.shipping = shipping.id) "
										" INNER JOIN ordered ON (shipped.ordered = ordered.id) "
										" INNER JOIN orders ON (ordered.porder = orders.id) "
										" INNER JOIN customers ON (orders.customer = customers.id) "
										" LEFT JOIN makers ON (orders.maker = makers.id) "
										" WHERE `shipping`.`dt` BETWEEN ? AND ? "+sql_makers+" GROUP BY shipped.id ORDER BY shipping.dt DESC, ordered.name,ordered.cover LIMIT 100");
			query.addBindValue(ui->dateTimeEdit_report_begin->dateTime());
			query.addBindValue(ui->dateTimeEdit_report_end->dateTime());
			if(query.exec()){
				htm += trUtf8("<table align=center width=100% border=1 cellspacing=-1 cellpadding=2 style='border-color: #000000;border-collapse: collapse'>");
				htm += trUtf8("<thead>");
				htm += trUtf8("<tr><th>№ от</th><th>Дата</th><th>Поставщик</th>");
				htm += trUtf8("<th>Покупатель</th>");
				htm += trUtf8("<th>№ зак</th>");
				htm += trUtf8("<th>Название</th>");
				htm += trUtf8("<th>Чехол</th>");
				htm += trUtf8("<th>Размер</th>");
				htm += trUtf8("<th>Кол-во</th>");
				htm += trUtf8("<th>На сумму</th>");
				htm += trUtf8("</tr></thead><tbody>");
				float tsm=0;
				while(query.next()){
					htm += trUtf8("<tr><td>") +query.value(0).toString()+ trUtf8("</td>");
					htm += trUtf8("<td>") +query.value(1).toString()+ trUtf8("</td>");
					htm += trUtf8("<td>") +query.value(2).toString()+ trUtf8("</td>");
					htm += trUtf8("<td>") +query.value(3).toString()+ trUtf8("</td>");
					htm += trUtf8("<td align=center>") +query.value(4).toString()+ trUtf8("</td>");
					htm += trUtf8("<td>") +query.value(5).toString()+ trUtf8("</td>");
					htm += trUtf8("<td>") +query.value(6).toString()+ trUtf8("</td>");
					htm += trUtf8("<td align=center>") +query.value(7).toString()+ trUtf8("</td>");
					htm += trUtf8("<td align=center>") +query.value(8).toString()+ trUtf8("</td>");
					QString s1;
					s1.setNum(query.value(9).toFloat(),'f',2);
					tsm += query.value(9).toFloat();
					htm += trUtf8("<td align=right>") +s1+ trUtf8("</td>");
					htm += trUtf8("</tr>");
				}
				QString s1;
				s1.setNum(tsm,'f',2);
				htm += trUtf8("<tr><td colspan=9>Итого:</td><td align=right>") +s1+ trUtf8("</td>");
				htm += trUtf8("</tr>");

				htm += trUtf8("</tbody></table>");
			}else{
				qDebug() << query.lastError();
			}

		};
		break;
	default:
		QApplication::restoreOverrideCursor();

		QMessageBox::warning(this, trUtf8("Внимание"),trUtf8("Запрошенный функционал в процессе разработки"));
		return;
	}

	DialogPrintPreview prv;
	prv.set_doc(htm);
	QApplication::restoreOverrideCursor();

	prv.exec();
	QApplication::restoreOverrideCursor();
}

void ReportDialog::on_comboBox_report_selector_activated(int index)
{
	QSqlQueryModel * model = new QSqlQueryModel(this);
	QString s="SELECT fio,id FROM customers WHERE ";

	switch(index) {
	case  3: s += " wholesaler<2 ";
	case  5:

		ui->groupBox_client_selector->setEnabled(true);
		if(ui->comboBox_report_selector->currentIndex() == 5) s += " wholesaler=2 ";

		s += " GROUP BY id ORDER BY fio";
		model->setQuery(s);
		ui->listView_clients->setModel(model);


		break;
		//
	default:
		ui->groupBox_client_selector->setEnabled(false);
	}
}
	/*
void ReportDialog::on_checkBox_private_toggled(bool checked)
{
	QSqlQueryModel * model = new QSqlQueryModel(0);
		QString s="SELECT fio,id FROM customers WHERE ";
		if(ui->checkBox_wholesalers->checkState() == 2) {
			s += " wholesaler=2 ";
			if(ui->checkBox_private->checkState()==2) {
				s += " OR wholesaler=0 ";
			}
		}
		else{
			if(ui->checkBox_private->checkState()==2) {
				s += " wholesaler=0 ";
			}

		}
		s += " GROUP BY id ORDER BY fio";
		model->setQuery(s);
		ui->listView_clients->setModel(model);

}

void ReportDialog::on_checkBox_wholesalers_toggled(bool checked)
{
	on_checkBox_private_toggled(checked);
}
*/
void ReportDialog::on_toolButton_select_all_clicked()
{
	ui->listView_clients->selectAll();
}

void ReportDialog::setup_report(int ri){
	ui->comboBox_report_selector->setCurrentIndex(ri);
};

void ReportDialog::on_lineEdit_clients_filter_textChanged(QString )
{
	QSqlQueryModel * model = new QSqlQueryModel(this);
	QString s="SELECT fio,id FROM customers WHERE ";

	switch(ui->comboBox_report_selector->currentIndex()) {
	case  3: s += " wholesaler<2 ";
	case  5:

		ui->groupBox_client_selector->setEnabled(true);
		if(ui->comboBox_report_selector->currentIndex() == 5) s += " wholesaler=2 ";

		s += " AND fio LIKE '"+ui->lineEdit_clients_filter->text()+"%' ";
		s += " GROUP BY id ORDER BY fio";
		model->setQuery(s);
		ui->listView_clients->setModel(model);


		break;
		//
	default:
		ui->groupBox_client_selector->setEnabled(false);
	}
}
