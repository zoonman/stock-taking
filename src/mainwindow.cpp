#include <QtSql>
#include <QSqlDatabase>
#include <QSqlQueryModel>
#include <QSqlError>
#include <QtWidgets/QTableView>
#include <QModelIndex>
#include <QtWidgets/QStyleFactory>
#include <QtWidgets/QListWidget>
#include <QtWidgets/QListWidgetItem>
#include <QTextDocument>
#include <QPrinter>
#include <QPrintDialog>
#include <QSizeGrip>
#include <QCompleter>
#include <QTextBrowser>
#include <QCloseEvent>
#include <QSettings>
#include <QStandardPaths>
#include <QDir>
#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "dialogmysqlconfig.h"
#include "dialogprintpreview.h"
#include "reportdialog.h"
#include "shippingdialog.h"

int order_num=1;
int last_iid=1;
bool o_saved=true;
bool populating_customer_list=false;
QString htm;
MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent), ui(new Ui::MainWindow)
{
    ui->setupUi(this);
		QSettings settings(this);
		resize(settings.value("size", QSize(800, 510)).toSize());
		move(settings.value("pos", QPoint(10, 10)).toPoint());
		//appstyle
		QString qst;
		qst = settings.value("appstyle", "windowsxp").toString();
		QApplication::setStyle(QStyleFactory::create(qst));

		//QApplication::

    if (!QSqlDatabase::drivers().contains("QSQLITE"))
        QMessageBox::critical(this, "Unable to load database", "This application needs the SQLITE driver");

    QSqlDatabase db = QSqlDatabase::addDatabase("QSQLITE");

    #ifdef Q_OS_OSX
        QString location = QStandardPaths::writableLocation(QStandardPaths::DataLocation);
        QDir dir;
        dir.mkpath(location);
        location = location.append("/bank.sqlite");
        db.setDatabaseName(location);
    #else
        db.setDatabaseName("bank.db");
    #endif


    if (!db.open()) {
        QMessageBox::about(this,trUtf8("Ошибка"), ( db.lastError().text()) );
        return;
    }
    QStringList tables = db.tables();
    if (! tables.contains("customers", Qt::CaseInsensitive)) {
        QSqlQuery q;
				q.exec(QLatin1String("create table customers(id integer primary key AUTOINCREMENT, fio varchar, wholesaler integer)"));
        //q.result()->lastError().text()
    }
    if (! tables.contains("orders", Qt::CaseInsensitive)) {
      QSqlQuery q;
			q.exec(QLatin1String("create table orders(id integer primary key AUTOINCREMENT, dt datetime, customer integer, delivery integer, city varchar, street varchar, house varchar, flat varchar, phone varchar, contact varchar, delivery_price integer, delivery_dt datetime, lift_floor integer, lift_price integer, paid integer, maker integer)"));
    }
    if (! tables.contains("ordered", Qt::CaseInsensitive)) {
      QSqlQuery q;
			q.exec(QLatin1String("create table ordered(id integer primary key AUTOINCREMENT , porder integer, name varchar, cover varchar, size varchar, count integer, price integer, discount varchar)"));
    }

    if (! tables.contains("spr", Qt::CaseInsensitive)) {
      QSqlQuery q;
      q.exec(QLatin1String("create table spr(id integer primary key, spi integer, val varchar)"));
    }

		if (! tables.contains("shipping", Qt::CaseInsensitive)) {
			QSqlQuery q;
			q.exec(QLatin1String("create table shipping(id integer primary key AUTOINCREMENT, dt datetime,taker varchar, notes varchar)"));
		}

		if (! tables.contains("shipped", Qt::CaseInsensitive)) {
			QSqlQuery q;
			q.exec(QLatin1String("create table shipped(`id` integer primary key AUTOINCREMENT, `shipping` integer, `ordered` integer,`count` integer)"));
		}

		if (! tables.contains("makers", Qt::CaseInsensitive)) {
			QSqlQuery q;
			q.exec(QLatin1String("create table makers(`id` integer primary key AUTOINCREMENT, `maker` varchar, `header` varchar)"));
		}
    if (!db.open()) {
				//qDebug() << QObject::trUtf8("Ошибка инициализации БД приложения") << db.lastError().text();
        return ;
    }
    QSqlQuery query("SELECT MAX(id) FROM orders");
    while (query.next()) {
			order_num  = query.value(0).toInt();
    }
    ui->label_order_num->setNum(order_num);

    this->ui->tableWidget_ordered->resizeColumnsToContents();
		//


		query.prepare("SELECT fio,id FROM customers WHERE fio != '' GROUP BY id ORDER BY fio");
		ui->comboBox_customer->setMaxCount(0);
		QCompleter *qcc = static_cast<QCompleter *>(ui->comboBox_customer->completer());
		if(qcc){
			qcc->setCompletionMode (QCompleter::PopupCompletion );
			qcc->setModelSorting (QCompleter::CaseInsensitivelySortedModel);
		}
		QCoreApplication::processEvents(QEventLoop::ExcludeUserInputEvents);
		ui->comboBox_customer->setMaxCount(1000);
		if (query.exec()){
			while (query.next()) {
				ui->comboBox_customer->addItem(query.value(0).toString(),query.value(1).toString());
			}
		}
		else{
			//qDebug() << query.lastError();
		};
	query.prepare("SELECT city FROM orders WHERE city != '' GROUP BY city ORDER BY city");
	QStringList city_list;
	if (query.exec()){
		while (query.next()) {
			city_list << query.value(0).toString();
		}
	}
	QCompleter *city_c = new QCompleter(city_list,this);
	city_c->setCaseSensitivity(Qt::CaseInsensitive);
	ui->lineEdit_city->setCompleter(city_c);
	query.prepare("SELECT street FROM orders WHERE street != '' GROUP BY street ORDER BY street");
	QStringList street_list;
	if (query.exec()){
		while (query.next()) {
			street_list << query.value(0).toString();
		}
	}
	QCompleter *street_c = new QCompleter(street_list,this);
	street_c->setCaseSensitivity(Qt::CaseInsensitive);
	ui->lineEdit_street->setCompleter(street_c);


	query.prepare("SELECT * FROM makers  ORDER BY id");
	if (query.exec()){
		while (query.next()) {
			ui->comboBox_maker->addItem(query.value(1).toString(),query.value(0).toString());
		}
	}
	else{
		//qDebug() << query.lastError();
	};


	load_order(0);
	on_pushButton_begin_search_order_toggled(false);
	on_toolButton_livesearch_clicked();
	ui->toolBox->setCurrentIndex(2);
}

MainWindow::~MainWindow()
{
	QSettings settings(this);
	settings.setValue("size", size());
	settings.setValue("pos", pos());

	delete ui;
}

void MainWindow::on_pushButton_save_clicked()
{
		on_action_save_order_triggered();
}

void MainWindow::on_action_exit_triggered()
{
   //
    MainWindow::close();
}

void MainWindow::on_action_about_triggered()
{
    QMessageBox::about(this, trUtf8("Учет продаж"),
                       trUtf8("<h1>Учет продаж</h1><hr>") +
                       trUtf8("<p>Версия  ") + QApplication::applicationVersion() +
											 trUtf8("</p><p><a href='http://www.zoonman.ru/projects/stock-taking/'>Последняя версия</a><p><p>Отчет об ошибках отправлять на <a href='mailto:zoonman@gmail.com'>zoonman@gmail.com</a></p><p style='color: gray'>Qt ")+qVersion ()+"</p>" );
}

void MainWindow::on_action_mysql_config_triggered()
{
    //
		DialogMySQLConfig mcfg(this);
    mcfg.exec();
    //mcfg.show();
}

void MainWindow::on_pushButton_add_clicked()
{
	on_action_add_wholesaler_triggered();

}

void MainWindow::on_action_add_wholesaler_triggered()
{
	ui->toolBox->setCurrentIndex(0);
	ui->dateTimeEdit_order->setDateTime(QDateTime::currentDateTime ());
	ui->dateTimeEdit_delivery->setDateTime(QDateTime::currentDateTime ());
		//this->set
	QSqlQuery query("SELECT MAX(id) FROM orders");
	while (query.next()) {
		order_num  = query.value(0).toInt() + 1;
	}


	query.prepare("INSERT INTO orders (id, dt, delivery_dt) "
								"VALUES (?, ?, ?)");
	query.addBindValue(order_num);
	query.addBindValue(ui->dateTimeEdit_order->dateTime());
	query.addBindValue(ui->dateTimeEdit_delivery->dateTime());
	query.exec();

	ui->label_order_num->setNum(order_num);
	ui->tableWidget_ordered->setRowCount(0);
	//

	ui->lineEdit_city->setText("");
	ui->lineEdit_contact->setText("");
	ui->lineEdit_flat->setText("");
	ui->lineEdit_house->setText("");
	ui->lineEdit_lift_price->setText("");
	ui->lineEdit_payment->setText("");
	ui->lineEdit_phone->setText("");
	ui->lineEdit_street->setText("");
	ui->comboBox_customer->setCurrentIndex(-1);
	ui->lineEdit_delivery_price->setText("");
	ui->spinBox_lift->setValue(0);

	on_action_add_order_line_triggered();
	saved(false);
}

void MainWindow::on_actionMotif_triggered()
{
	 //QApplication::setStyle(QStyleFactory::create("Plastique"));
	 QApplication::setStyle(QStyleFactory::create("Motif"));
}


void MainWindow::on_tableWidget_ordered_cellPressed(int row, int column)
{

}

void MainWindow::on_toolButton_add_clicked()
{
  last_iid=1;
	ui->dateTimeEdit_delivery->setDateTime(QDateTime::currentDateTime ());
  QSqlQuery query("SELECT MAX(id) FROM spr");
  while (query.next()) {
    last_iid  = query.value(0).toInt() + 1;
		// qDebug() << QObject::trUtf8("+") << last_iid;
  }
  query.clear();

  if (ui->comboBox->currentIndex() > 0 && ui->lineEdit_cval->text().length() > 0) {
    query.prepare("INSERT INTO spr (id, spi, val) "
                  "VALUES (:id, :spi, :val)");
    query.bindValue(":id", last_iid );
    query.bindValue(":spi", ui->comboBox->currentIndex());
    query.bindValue(":val", ui->lineEdit_cval->text());
    query.exec();
    query.finish();
    query.prepare("SELECT id,val FROM spr WHERE spi = ? ORDER BY val");
    query.addBindValue(ui->comboBox->currentIndex());
    query.exec();
    this->ui->listWidget_listval->clear();
    while (query.next()) {
      this->ui->listWidget_listval->addItem(query.value(1).toString());

    }
    ui->lineEdit_cval->clear();
    ui->lineEdit_cval->setFocus();
  }
  else
    if(ui->comboBox->currentIndex() <= 0){
      QMessageBox::warning(this, trUtf8("Ошибка"),trUtf8("Не выбран справочник."));
      ui->comboBox->setFocus();
    }
    else
    {
      QMessageBox::warning(this, trUtf8("Ошибка"),trUtf8("Отсутствует значение!"));
      ui->lineEdit_cval->setFocus();
    }
}

void MainWindow::on_toolButton_update_clicked()
{
   // обновить значение справочника
  if (ui->comboBox->currentIndex() > 0 &&
      ui->lineEdit_cval->text().length() > 0 &&
      ui->listWidget_listval->count() > 0 &&
      ui->listWidget_listval->currentItem()->text().length() > 0) {
    QSqlQuery query;
    query.prepare("UPDATE spr SET val = :val "
                  "WHERE spi = :spi AND val = :fval");;
    query.bindValue(":fval", ui->listWidget_listval->currentItem()->text());
    query.bindValue(":spi", ui->comboBox->currentIndex());
    query.bindValue(":val", ui->lineEdit_cval->text());
    query.exec();
    query.finish();
    query.prepare("SELECT id,val FROM spr WHERE spi = ? ORDER BY val");
    query.addBindValue(ui->comboBox->currentIndex());
    query.exec();
    this->ui->listWidget_listval->clear();
    while (query.next()) {
      this->ui->listWidget_listval->addItem(query.value(1).toString());
    }
    ui->lineEdit_cval->clear();
  }
  else
    if(ui->comboBox->currentIndex() <= 0){
      QMessageBox::warning(this, trUtf8("Ошибка"),trUtf8("Не выбран справочник."));
      ui->comboBox->setFocus();
    }
    else
    {
      QMessageBox::warning(this, trUtf8("Ошибка"),trUtf8("Отсутствует значение!"));
      ui->lineEdit_cval->setFocus();
    }
}

void MainWindow::on_comboBox_currentIndexChanged(int index)
{

}

void MainWindow::on_comboBox_activated(int index)
{
  this->ui->listWidget_listval->clear();
  QSqlQuery query;
  query.prepare("SELECT id,val FROM spr WHERE spi = ? ORDER BY val");
  query.addBindValue(ui->comboBox->currentIndex());
  query.exec();
  while (query.next()) {
    ui->listWidget_listval->addItem(query.value(1).toString());
  }
	if(ui->comboBox->currentIndex()>0) {
		ui->toolButton_add->setEnabled(true);
		ui->toolButton_remove->setEnabled(true);
		ui->toolButton_update->setEnabled(true);
	}else {
		ui->toolButton_add->setEnabled(false);
		ui->toolButton_remove->setEnabled(false);
		ui->toolButton_update->setEnabled(false);
	}
}

void MainWindow::on_toolButton_remove_clicked()
{
  // удалить запись справочника
  if (ui->comboBox->currentIndex() > 0 &&
      ui->listWidget_listval->count() > 0 &&
      ui->listWidget_listval->currentItem()->text().length()>0) {
    QSqlQuery query;
    query.prepare("DELETE FROM spr WHERE spi = ? AND val = ?");
    query.addBindValue(ui->comboBox->currentIndex());
    query.addBindValue(ui->listWidget_listval->currentItem()->text().replace(QString("?"),QString("\?")));
    query.exec();
    on_comboBox_activated(ui->comboBox->currentIndex());
  }
  else{
    QMessageBox::warning(this, trUtf8("Ошибка"),trUtf8("Нечего удалять!"));
  }

}

void MainWindow::on_listWidget_listval_entered(QModelIndex index)
{

}

void MainWindow::on_listWidget_listval_itemClicked(QListWidgetItem* item)
{
  ui->lineEdit_cval->setText(ui->listWidget_listval->currentItem()->text());
  ui->lineEdit_cval->setFocus();
}

void MainWindow::on_tableWidget_ordered_cellActivated(int row, int column)
{
    //

}

void MainWindow::on_tableWidget_ordered_cellEntered(int row, int column)
{

}

void MainWindow::on_tableWidget_ordered_cellChanged(int row, int column)
{

	// здесь должны быть функции пересчета
	//saved(false);

}
void MainWindow::order_line_size_changed(QString){
	//

	//qDebug()<< QObject::sender()->parent()->parent()->objectName();

	for (int i=0;i< ui->tableWidget_ordered->rowCount();i++) {
		QLineEdit *t_qe_size = static_cast<QLineEdit *>(ui->tableWidget_ordered->cellWidget(i,2));
		if(t_qe_size == QObject::sender()) {
			ui->tableWidget_ordered->setCurrentCell(i,2);
			break;
		}
	}
	QLineEdit *qe_size = static_cast<QLineEdit *>(ui->tableWidget_ordered->cellWidget(ui->tableWidget_ordered->currentRow(),2));


	QLineEdit *qe_price = static_cast<QLineEdit *>(ui->tableWidget_ordered->cellWidget(ui->tableWidget_ordered->currentRow(),4));

	QComboBox *qe_name = static_cast<QComboBox *>(ui->tableWidget_ordered->cellWidget(ui->tableWidget_ordered->currentRow(),0));

	QComboBox *qe_cover = static_cast<QComboBox *>(ui->tableWidget_ordered->cellWidget(ui->tableWidget_ordered->currentRow(),1));

	//qDebug()<< qe_size;


	if (qe_size && qe_name && qe_cover && qe_price) {
		 if (!qe_size->text().isEmpty() &&
				 !qe_name->currentText().isEmpty() &&
				 !qe_cover->currentText().isEmpty()
				 ) {
			 // поиск расценок для указанной стоимости
			 // name varchar, cover varchar, size varchar, count integer, price
			 //
			 QSqlQuery query;
			 query.prepare("SELECT price FROM ordered WHERE name = ? AND cover = ? AND size = ? AND price > 0 ORDER BY id DESC LIMIT 1");
			 query.addBindValue(qe_name->currentText());
			 query.addBindValue(qe_cover->currentText());
			 query.addBindValue(qe_size->text());

			 if(query.exec()){
				// qDebug() << query.lastQuery();
				 while(query.next()){
					 if (qe_price->text().toFloat() <= 0)
						 qe_price->setText( query.value(0).toString());
				 }
			 }else {
				 //qDebug() << query.lastError();
			 }


		 }
	}

	table_recalculate(0);
}
void MainWindow::on_action_add_order_line_triggered()
{
	// добавить строку в таблицу позиций заказа
  this->ui->tableWidget_ordered->insertRow(this->ui->tableWidget_ordered->rowCount());
  int row = this->ui->tableWidget_ordered->rowCount()-1;
  int column = 0;
	ui->toolBox->setCurrentIndex(0);
  {// ввод вида изделия
    QComboBox *cWidget = new QComboBox(this);
    QSqlQuery query("SELECT val,id FROM spr WHERE spi=1 ORDER BY val");
    while (query.next()) {
      cWidget->addItem(query.value(0).toString(),query.value(1).toInt());
    }
    cWidget->setFrame(false);
    cWidget->setMaxVisibleItems(25);
		cWidget->setCurrentIndex(-1);
		cWidget->setStyleSheet(" QComboBox::drop-down { image: url(:/find/icons/drop-down.png); } "
										" QComboBox::drop-down:hover,QComboBox::drop-down:on { image: url(:/find/icons/drop-down-hover.png); } ");
    this->ui->tableWidget_ordered->setCellWidget(row,column,cWidget);
		this->ui->tableWidget_ordered->setRowHeight(row,18);
    this->ui->tableWidget_ordered->resizeColumnsToContents();
		QObject::connect(cWidget, SIGNAL(currentIndexChanged(int)),
													this, SLOT(table_recalculate(int)));
  }
  column = 1; {// ввод чехла
    QComboBox *cWidget = new QComboBox(this);
    QSqlQuery query("SELECT val,id FROM spr WHERE spi=2 ORDER BY val");

		cWidget->view()->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOn);
		cWidget->view()->setCornerWidget(new QSizeGrip(cWidget));
				//setCornerWidget(new QSizeGrip(cWidget));
		while (query.next()) {
      cWidget->addItem(query.value(0).toString(),query.value(1).toInt());
    }
    cWidget->setFrame(false);
		cWidget->setStyleSheet(" QComboBox::drop-down { image: url(:/find/icons/drop-down.png); } "
										" QComboBox::drop-down:hover,QComboBox::drop-down:on { image: url(:/find/icons/drop-down-hover.png); } ");
    this->ui->tableWidget_ordered->setCellWidget(row,column,cWidget);
		QObject::connect(cWidget, SIGNAL(currentIndexChanged(int)),
													this, SLOT(table_recalculate(int)));
    this->ui->tableWidget_ordered->resizeColumnsToContents();

  }
  column = 2; {// ввод размеров
		QRegExp rx("\\d{2,3}[xXхХ*\\x0445]\\d{2,3}");
    QValidator *validator = new QRegExpValidator(rx, this);
    QLineEdit *cEdit = new QLineEdit(this);
    cEdit->setValidator(validator);
    cEdit->setFrame(false);
    cEdit->setMinimumWidth(20);
    cEdit->setBaseSize(20,18);
		cEdit->setAlignment(Qt::AlignHCenter | Qt::AlignVCenter);

		QObject::connect(cEdit, SIGNAL(textChanged(QString)),
													this, SLOT(order_line_size_changed(QString)));

    this->ui->tableWidget_ordered->setCellWidget(row,column,cEdit);

    this->ui->tableWidget_ordered->resizeColumnsToContents();

  }
  column = 3; {// ввод кол-ва
    QSpinBox *cSpBox = new QSpinBox(this);
    cSpBox->setFrame(false);
    QObject::connect(cSpBox, SIGNAL(valueChanged(int)),
                          this, SLOT(table_recalculate(int)));
		cSpBox->setValue(0);
		cSpBox->setMaximum(999);
		cSpBox->setStyleSheet(
													" QSpinBox::up-button { background-color: transparent;border-style: none;border-width: 1px;} "
													" QSpinBox::up-button:pressed { background-color: rgba(100, 100, 100, 50%) ;border-style: none;border-width: 1px} "
													" QSpinBox::up-arrow { image: url(:/find/icons/spinbox-up.png);} "
													" QSpinBox::up-arrow:hover { image: url(:/find/icons/spinbox-up-hover.png);} "
													" QSpinBox::down-button { background-color: transparent;border-style: none;border-width: 1px} "
													" QSpinBox::down-button:pressed { background-color: rgba(100, 100, 100, 50%) ;border-style: none;border-width: 1px} "
													" QSpinBox::down-arrow { image: url(:/find/icons/spinbox-down.png);} "
													" QSpinBox::down-arrow:hover { image: url(:/find/icons/spinbox-down-hover.png);} "
													);
		//cSpBox->setFixedHeight(17);
		cSpBox->setAlignment(Qt::AlignHCenter | Qt::AlignVCenter);
    this->ui->tableWidget_ordered->setCellWidget(row,column,cSpBox);

  }
  column = 4; {// ввод цены
    QRegExp rx("\\d{1,7}");
    QValidator *validator = new QRegExpValidator(rx, this);
    QLineEdit *cEdit = new QLineEdit(this);
    cEdit->setValidator(validator);
    cEdit->setFrame(false);
		cEdit->setAlignment(Qt::AlignRight | Qt::AlignVCenter);

    this->ui->tableWidget_ordered->setCellWidget(row,column,cEdit);
    QObject::connect(cEdit, SIGNAL(textChanged(QString)),
                          this, SLOT(table_recalculate(QString)));
  }
  column = 5; { // ввод скидки
    QComboBox *cWidget = new QComboBox(this);
    cWidget->setFrame(false);
    cWidget->setEditable(true);
		cWidget->setStyleSheet(" QComboBox::drop-down { image: url(:/find/icons/drop-down.png); } "
										" QComboBox::drop-down:hover { image: url(:/find/icons/drop-down-hover.png); } ");

    QRegExp rx("\\d{1,2}");
    QValidator *validator = new QRegExpValidator(rx, this);
    cWidget->setValidator(validator);

    for(int i=0;i<=50;i+=5) {
      cWidget->addItem(QString::number(i),i);

    }
    this->ui->tableWidget_ordered->setCellWidget(row,column,cWidget);
    QObject::connect(cWidget, SIGNAL(editTextChanged(QString)),
                          this, SLOT(table_recalculate(QString)));
  }

	QTableWidgetItem *newItem = new QTableWidgetItem(QString(""));
						ui->tableWidget_ordered->setItem(row, 8, newItem);
	saved(false);
}

void MainWindow::on_action_remove_order_line_triggered()
{
    //
  ui->tableWidget_ordered->removeRow(ui->tableWidget_ordered->currentRow());
	saved(false);
}

void MainWindow::table_recalculate(int c){
	saved(false);
	this->setCursor(Qt::WaitCursor);
  float total = 0, payment=0, credit=0;
  payment=ui->lineEdit_payment->text().toFloat();

  for(int row=0;row<ui->tableWidget_ordered->rowCount();row++){
    int count = 0;
    float price = 0,
    discount = 0,
    price_with_discount=0,
    summ = 0;
    QSpinBox *qsb = static_cast<QSpinBox *>(ui->tableWidget_ordered->cellWidget(row,3));
    if (qsb) {
      count = qsb->value();
    }

    QLineEdit *qe_p = static_cast<QLineEdit *>(ui->tableWidget_ordered->cellWidget(row,4));
    if (qe_p) {
      price = qe_p->text().toFloat();
    }

    QComboBox *qe_d = static_cast<QComboBox *>(ui->tableWidget_ordered->cellWidget(row,5));
    if (qe_d) {
      discount = qe_d->currentText().toFloat();
    }
		price_with_discount = round( price * (100 - discount) / 100 );
    QLabel *cQL = static_cast<QLabel *>(ui->tableWidget_ordered->cellWidget(row,6));
    if (!cQL) {
      cQL = new QLabel(this);
      ui->tableWidget_ordered->setCellWidget(row, 6, cQL);
    }
		QString s;
		s.setNum(price_with_discount,'f',2);
		cQL->setText(s);;
		cQL->setAlignment(Qt::AlignRight | Qt::AlignVCenter);

		summ =  count * price_with_discount;
    QLabel *cQLs = static_cast<QLabel *>(ui->tableWidget_ordered->cellWidget(row,7));
    if (!cQLs) {
      cQLs = new QLabel(this);
      ui->tableWidget_ordered->setCellWidget(row, 7, cQLs);
    }
		//
		s.setNum(summ,'f',2);
		cQLs->setAlignment(Qt::AlignRight | Qt::AlignVCenter);
    cQLs->setText(s);
    total += summ;
  }
  //
  QString s;
	total += ui->lineEdit_lift_price->text().toFloat();

	total += ui->lineEdit_delivery_price->text().toFloat();
	s.setNum(total,'f',2);
	ui->label_summary->setText(s);
	credit = total - payment;
  s.setNum(credit,'f',2);
  ui->label_credit->setText(s);
	this->setCursor(Qt::ArrowCursor);

}

void MainWindow::table_recalculate(const QString & text){
  table_recalculate(0);
}

void MainWindow::on_action_print_check_triggered()
{


  //
  QTextDocument *pd = new QTextDocument(this);
	htm = trUtf8("<html><head><title></title></head><body style='font-family: Old Standard TT'>");
	QSqlQuery q;
	q.prepare("SELECT header FROM makers WHERE id= ? LIMIT 1");
	q.addBindValue(ui->comboBox_maker->itemData(ui->comboBox_maker->currentIndex()).toString());
	if(q.exec()){
		while(q.next()){
			htm +=	q.value(0).toString();
		}
	}

	htm += ui->comboBox_customer->currentText();
	QString s;
	s.setNum(order_num);
	htm += trUtf8("<h1 align=center>Товарный чек №")+
				 s+
				 trUtf8(" от ")+
				 ui->dateTimeEdit_order->dateTime().date().toString("&laquo;d&raquo; MMMM yyyy")
				 +
				 trUtf8(" г. </h1>");
	htm += trUtf8("<table align=center width=100% cellspacing=-1 cellpadding=2 border=1 style='border-color: #000000;border-collapse: collapse'>");
	//style=\"border-style: solid;border-color: #AAAAAA;border-width: 1px\"
	htm += trUtf8("<thead><tr>");
	htm += trUtf8("<th>№</th><th>Наименование</th><th>Ед.изм.</th><th>Кол-во</th><th>Цена</th><th>Сумма</th>");
	htm += trUtf8("</tr></thead>");
	htm += trUtf8("<tbody>");

	float total = 0,payment=0, credit=0;
	payment=ui->lineEdit_payment->text().toFloat();;


	for(int row=0;row<ui->tableWidget_ordered->rowCount();row++){
		htm += trUtf8("<tr>");
		int count = 0;
		float price = 0,
		discount = 0,
		price_with_discount=0,
		summ = 0;
		s.setNum(row+1);htm += trUtf8("<td align=right width=5%>") + s + trUtf8("</td>");
		QComboBox *qe_n = static_cast<QComboBox *>(ui->tableWidget_ordered->cellWidget(row,0));
		if (qe_n) {
			htm += trUtf8("<td width=50%>")+ qe_n->currentText();
		}

		QLineEdit *qe_s = static_cast<QLineEdit *>(ui->tableWidget_ordered->cellWidget(row,2));
		if (qe_s) {
			if(!qe_s->text().isEmpty())	htm += trUtf8(", ")+ qe_s->text();
		}

		QComboBox *qe_c = static_cast<QComboBox *>(ui->tableWidget_ordered->cellWidget(row,1));
		if (qe_c) {
			htm += trUtf8(", ")+ qe_c->currentText();
		}
		htm += trUtf8("</td>");


		QSpinBox *qsb = static_cast<QSpinBox *>(ui->tableWidget_ordered->cellWidget(row,3));
		if (qsb) {
			count = qsb->value();
		}
		htm += trUtf8("<td align=center width=5%>шт.</td>");
		s.setNum(count);
		htm += trUtf8("<td align=center width=10%>") + s + trUtf8("</td>");

		QLineEdit *qe_p = static_cast<QLineEdit *>(ui->tableWidget_ordered->cellWidget(row,4));
		if (qe_p) {
			price = qe_p->text().toFloat();
		}

		QComboBox *qe_d = static_cast<QComboBox *>(ui->tableWidget_ordered->cellWidget(row,5));
		if (qe_d) {
			discount = qe_d->currentText().toFloat();
		}
		price_with_discount = round( price * (100 - discount) / 100);
		s.setNum(price_with_discount,'f',2);htm += trUtf8("<td align=right>") + s + trUtf8("</td>");

		summ = count * price_with_discount;
		s.setNum(summ,'f',2);htm += trUtf8("<td align=right>") + s + trUtf8("</td>");

		QString z;
		z.setNum(summ,'f',2);

		total += summ;
		htm += trUtf8("</tr>");
	}


	if (ui->lineEdit_lift_price->text().toFloat() > 0) {
		total += ui->lineEdit_lift_price->text().toFloat();
		s.setNum(ui->lineEdit_lift_price->text().toFloat(),'f',2);
		htm += trUtf8("<tr><td colspan=5>Подъем:</td><td align=right>") + s + trUtf8("</td></tr>");
	}

	if (ui->lineEdit_delivery_price->text().toFloat() > 0) {
		total += ui->lineEdit_delivery_price->text().toFloat();
		s.setNum(ui->lineEdit_delivery_price->text().toFloat(),'f',2);
		htm += trUtf8("<tr><td colspan=5>Доставка:</td><td align=right>") + s + trUtf8("</td></tr>");
	}
	credit = total - payment;

	s.setNum(total,'f',2);
	htm += trUtf8("<tr bgcolor='#eeeeee'><td colspan=5><b>Итого:</b></td><td align=right><b>") + s + trUtf8("</b></td></tr>");

	s.setNum(payment,'f',2);
	htm += trUtf8("<tr><td colspan=5>Оплачено:</td><td align=right>") + s + trUtf8("</td></tr>");

	if(credit > 0) {
		s.setNum(credit,'f',2);
		htm += trUtf8("<tr><td colspan=5>Долг:</td><td align=right>") + s + trUtf8("</td></tr>");
	}
	if(credit < 0) {
		s.setNum(qAbs(credit),'f',2);
		htm += trUtf8("<tr><td colspan=5>Сдача:</td><td align=right>") + s + trUtf8("</td></tr>");
	}

	htm += trUtf8("</tbody>");
	htm += trUtf8("</table>");

	htm += trUtf8("<p><b>Сумма, прописью:</b> <br><u>");

	htm += number_in_words(payment,1,trUtf8("рублей"),trUtf8("рубль"),trUtf8("рубля"),5);

	/*float to = total mod (float)1.00;
	to *= 100;
	*/
	s.setNum(payment,'f',2);
	int to = s.right(2).toInt();

	htm += trUtf8(" ");
	htm += number_in_words(to,2,trUtf8("копеек"),trUtf8("копейка"),trUtf8("копейки"),5);

	htm += trUtf8("</u></p>");
	htm += trUtf8("<table width=100%><tr><td width=70%>&nbsp;</td><td width=30%><hr></td></tr></table>");

	pd->setHtml(htm);
	//printer.setOutputFormat(QPrinter::PdfFormat);
	//printer.setOutputFileName(tr("tst.pdf"));
	DialogPrintPreview prv;
	prv.set_doc(htm);
	prv.exec();
	//pd->print(&printer);
}

void MainWindow::on_lineEdit_payment_textChanged(QString )
{
	table_recalculate(0);
}

void MainWindow::on_action_save_order_triggered()
{

	QString statusm=trUtf8("Сохраняем");
	ui->statusBar->showMessage(statusm,50);
	this->setCursor(Qt::WaitCursor);
	QCoreApplication::processEvents(QEventLoop::ExcludeUserInputEvents);
	{
//on_pushButton_add_clicked();
		QSqlQuery query;
		query.prepare("UPDATE orders SET dt = :dt , customer = :customer, delivery = :delivery, city = :city, street = :street, house = :house, flat = :flat, phone = :phone,contact = :contact, delivery_price = :delivery_price, delivery_dt = :delivery_dt, lift_floor = :lift_floor, lift_price = :lift_price, paid = :paid, maker = :maker  WHERE  id = :id ");;
  query.bindValue(":id", order_num);
  query.bindValue(":dt", ui->dateTimeEdit_order->dateTime());
	/* поиск или добавление покупателя */
	label_customer_add:
	QSqlQuery q;q.prepare("SELECT id FROM customers WHERE fio=?");
	q.addBindValue(ui->comboBox_customer->currentText());
	q.exec();
	int custId=0;
	while(q.next()) {custId = q.value(0).toInt();};
	if (custId <= 0) {
		QSqlQuery qc;qc.prepare("INSERT INTO customers (fio,wholesaler) VALUES(?,0) ");
		qc.addBindValue(ui->comboBox_customer->currentText());
		qc.exec();
		goto label_customer_add;
	}
	query.bindValue(":customer", custId);
	query.bindValue(":city", ui->lineEdit_city->text());
	query.bindValue(":street", ui->lineEdit_street->text());
	query.bindValue(":house", ui->lineEdit_house->text());
	query.bindValue(":flat", ui->lineEdit_flat->text());
	query.bindValue(":phone", ui->lineEdit_phone->text());
	query.bindValue(":contact", ui->lineEdit_contact->text());
	query.bindValue(":delivery", ui->checkBox_delivery->checkState());
	query.bindValue(":delivery_price", ui->lineEdit_delivery_price->text());
	query.bindValue(":delivery_dt", ui->dateTimeEdit_delivery->dateTime());
	query.bindValue(":lift_floor", ui->spinBox_lift->value());
	query.bindValue(":lift_price", ui->lineEdit_lift_price->text());
	query.bindValue(":paid", ui->lineEdit_payment->text());
	query.bindValue(":maker", ui->comboBox_maker->itemData(ui->comboBox_maker->currentIndex()).toString());
	if (!query.exec()){
		//qDebug() << query.lastError();
	}
	else{
		QSqlQuery qcl;
		qcl.prepare("DELETE FROM ordered WHERE porder = :porder");
		qcl.bindValue(":porder", order_num);
		if(!qcl.exec()){
			//qDebug() << query.lastError();
		}
		else{
			qcl.finish();
		}

		for(int row=0;row<ui->tableWidget_ordered->rowCount();row++){

			statusm += ".";
			ui->statusBar->showMessage(statusm);

			QSqlQuery qpos;

			if( ui->tableWidget_ordered->item(row,8)->text().toInt()> 0 ){
				qpos.prepare("INSERT INTO ordered (`porder`, `name`, `cover`, `size`, `count`, `price`, `discount`,`id`) VALUES(:porder, :name, :cover, :size, :count, :price, :discount, :id)");
				qpos.bindValue(":id", ui->tableWidget_ordered->item(row,8)->text() );
			}
			else {
				qpos.prepare("INSERT INTO ordered (`porder`, `name`, `cover`, `size`, `count`, `price`, `discount`) VALUES(:porder, :name, :cover, :size, :count, :price, :discount)");
			}
			//qDebug() << row;
			//qDebug() << qpos.lastQuery();


			int count = 0;
			float price = 0,
			discount = 0;
			QSpinBox *qsb = static_cast<QSpinBox *>(ui->tableWidget_ordered->cellWidget(row,3));
			if (qsb) {
				count = qsb->value();
			}

			QLineEdit *qe_p = static_cast<QLineEdit *>(ui->tableWidget_ordered->cellWidget(row,4));
			if (qe_p) {
				price = qe_p->text().toFloat();
			}

			QLineEdit *qe_s = static_cast<QLineEdit *>(ui->tableWidget_ordered->cellWidget(row,2));
			if (qe_s) {

				qpos.bindValue(":size", qe_s->text());
				//qDebug()<<qe_s->text();
			}

			QComboBox *qe_d = static_cast<QComboBox *>(ui->tableWidget_ordered->cellWidget(row,5));
			if (qe_d) {
				discount = qe_d->currentText().toFloat();
			}


			QComboBox *qe_n = static_cast<QComboBox *>(ui->tableWidget_ordered->cellWidget(row,0));
			if (qe_n) {
				qpos.bindValue(":name", qe_n->currentText());
			}
			QComboBox *qe_c = static_cast<QComboBox *>(ui->tableWidget_ordered->cellWidget(row,1));
			if (qe_c) {
				qpos.bindValue(":cover", qe_c->currentText());
			}
			qpos.bindValue(":porder", order_num);
			qpos.bindValue(":count", count);
			qpos.bindValue(":discount", discount);
			qpos.bindValue(":price", price);


			if(!qpos.exec()){
				//qDebug() << query.lastError();
			}
			else{
				if (qpos.lastInsertId().isValid()) {
					QTableWidgetItem *newItem = new QTableWidgetItem(qpos.lastInsertId().toString());
					ui->tableWidget_ordered->setItem(row, 8, newItem);
				}
			}
		}
	};
	this->setCursor(Qt::ArrowCursor);
	statusm=trUtf8("Сохранено");
	saved(true);
	on_pushButton_begin_search_order_toggled(false);
	on_toolButton_livesearch_clicked();
	ui->statusBar->showMessage(statusm,500);
};
}

void MainWindow::on_action_go_previos_order_triggered()
{
		// загрузка предыдущего заказа, автоматическое сохранение текущего!
	//on_action_save_order_triggered();
	load_order(-1);
}

void MainWindow::on_action_go_next_order_triggered()
{
	load_order(1);
}

void MainWindow::on_action_remove_order_triggered()
{
	/* */

	if (QMessageBox::question(this,trUtf8("Удаление заказа"), trUtf8("Текущий заказ будет удален безвозвратно.\nВы уверены в том, что нужно удалить текущий заказ?"),QMessageBox::Yes | QMessageBox::No,QMessageBox::No ) == QMessageBox::Yes) {
		QSqlQuery query;
		query.prepare("DELETE  FROM orders WHERE id = :parid ");
		query.bindValue(":parid", order_num);
		if (query.exec()){
			query.prepare("DELETE FROM ordered WHERE porder = :parid ");
			query.bindValue(":parid", order_num);
			if (query.exec()){
				QMessageBox::information(this,trUtf8("Удаление заказа"),trUtf8("Заказ удален успешно."),trUtf8("Хорошо"));
				load_order(-1);
			};
		};
	};
	on_toolButton_livesearch_clicked();
}
void MainWindow::load_order(int g) {

	if (!o_saved) {
		if (QMessageBox::question(this,trUtf8("Сохранение заказа"), trUtf8("Хотите сохранить изменения в заказе?"),QMessageBox::Yes | QMessageBox::No,QMessageBox::No ) == QMessageBox::Yes) {
			on_action_save_order_triggered();
		}
		else {
			//return;
		}
	}
	this->setCursor(Qt::WaitCursor);
	QString statusm=trUtf8("Загрузка заказа");
	ui->statusBar->showMessage(statusm,50);
	QCoreApplication::processEvents(QEventLoop::ExcludeUserInputEvents);
	ui->tableWidget_ordered->setRowCount(0);
	QSqlQuery query;
	if (g == 0) {
			query.prepare("SELECT orders.id,dt,customers.fio, delivery,city,street,house,flat,phone,contact,delivery_price,delivery_dt,lift_floor, lift_price,paid,maker FROM orders "
										"LEFT JOIN customers ON (customers.id = orders.customer) "
										"WHERE orders.id = :parid ORDER BY orders.id DESC LIMIT 1");
	}
	else
	if (g > 0) {
	query.prepare("SELECT orders.id,dt,customers.fio, delivery,city,street,house,flat,phone,contact,delivery_price,delivery_dt,lift_floor, lift_price,paid,maker FROM orders 	LEFT JOIN customers ON (customers.id = orders.customer)  WHERE orders.id > :parid ORDER BY orders.id ASC LIMIT 1");
	}
	else {
		query.prepare("SELECT orders.id,dt,customers.fio , delivery,city,street,house,flat,phone,contact,delivery_price,delivery_dt,lift_floor, lift_price,paid,maker FROM orders LEFT JOIN customers ON (customers.id = orders.customer)  WHERE orders.id < :parid ORDER BY orders.id DESC LIMIT 1");
	}
	query.bindValue(":parid", order_num);
	if (!query.exec()){
		//qDebug() << query.lastError();
	};
	while (query.next()) {
		statusm += ".";
		ui->statusBar->showMessage(statusm);
		QCoreApplication::processEvents(QEventLoop::ExcludeUserInputEvents);
		order_num  = query.value(0).toInt();
		ui->dateTimeEdit_order->setDateTime(query.value(1).toDateTime());
		ui->comboBox_customer->setEditText(query.value(2).toString());
		ui->checkBox_delivery->setChecked(query.value(3).toBool());
		ui->lineEdit_city->setText(query.value(4).toString());
		ui->lineEdit_street->setText(query.value(5).toString());
		ui->lineEdit_house->setText(query.value(6).toString());
		ui->lineEdit_flat->setText(query.value(7).toString());
		ui->lineEdit_phone->setText(query.value(8).toString());
		ui->lineEdit_contact->setText(query.value(9).toString());
		ui->lineEdit_delivery_price->setText(query.value(10).toString());
		ui->dateTimeEdit_delivery->setDateTime(query.value(11).toDateTime());
		ui->spinBox_lift->setValue(query.value(12).toInt());
		ui->lineEdit_lift_price->setText(query.value(13).toString());
		ui->lineEdit_payment->setText(query.value(14).toString());

		ui->comboBox_maker->setCurrentIndex(ui->comboBox_maker->findData(query.value(15).toString()));

		QSqlQuery q;
		q.prepare("SELECT `id`, `name`, `cover`, `size`, `count`, `price`, `discount` FROM ordered WHERE porder = :parid");
		q.bindValue(":parid", order_num);
		if(q.exec()){
			while (q.next()) {
				QCoreApplication::processEvents(QEventLoop::ExcludeUserInputEvents);
				on_action_add_order_line_triggered();
				int row = ui->tableWidget_ordered->rowCount() - 1;
				// название
				QComboBox *qe_n = static_cast<QComboBox *>(ui->tableWidget_ordered->cellWidget(row,0));
				if (qe_n) {
					qe_n->setCurrentIndex(qe_n->findText(q.value(1).toString()));;
				}
				// чехол
				QComboBox *qe_c = static_cast<QComboBox *>(ui->tableWidget_ordered->cellWidget(row,1));
				if (qe_c) {
					qe_c->setCurrentIndex(qe_c->findText(q.value(2).toString()));;
				}
				// размеры
				QLineEdit *qe_s = static_cast<QLineEdit *>(ui->tableWidget_ordered->cellWidget(row,2));
				if (qe_s) {
					qe_s->setText(q.value(3).toString());
				}
				// количество
				QSpinBox *qsb = static_cast<QSpinBox *>(ui->tableWidget_ordered->cellWidget(row,3));
				if (qsb) {
					qsb->setValue(q.value(4).toInt());
				}
				// цена
				QLineEdit *qe_p = static_cast<QLineEdit *>(ui->tableWidget_ordered->cellWidget(row,4));
				if (qe_p) {
					qe_p->setText(q.value(5).toString());
				}
				// скидка
				QComboBox *qe_d = static_cast<QComboBox *>(ui->tableWidget_ordered->cellWidget(row,5));
				if (qe_d) {
					int ri = qe_d->findText(q.value(6).toString());
					if (ri != -1) {
						qe_d->setCurrentIndex(ri);
					}
					else {
						qe_d->setEditText(q.value(6).toString());
					}
				}

				{// ввод
					QTableWidgetItem *newItem = new QTableWidgetItem(q.value(0).toString());
					ui->tableWidget_ordered->setItem(row, 8, newItem);
				}

			}
		}

	}
	ui->tableWidget_ordered->setColumnHidden(8,true);

	ui->label_order_num->setNum(order_num);
	this->setCursor(Qt::ArrowCursor);
	statusm=trUtf8("Загружено");
	saved(true);
	ui->statusBar->showMessage(statusm,500);

};

void MainWindow::on_action_add_retail_triggered()
{

}

void MainWindow::on_actionWindows_triggered()
{
	 QApplication::setStyle(QStyleFactory::create("Windows"));
}

void MainWindow::on_actionPlastique_triggered()
{
	QApplication::setStyle(QStyleFactory::create("Plastique"));
}

void MainWindow::on_actionCDE_triggered()
{
	QApplication::setStyle(QStyleFactory::create("CDE"));
}

void MainWindow::on_actionCleanlooks_triggered()
{
	 QApplication::setStyle(QStyleFactory::create("Cleanlooks"));
}

void MainWindow::on_lineEdit_customer_textChanged(QString )
{
	on_toolButton_livesearch_clicked();
}

void MainWindow::on_comboBox_customer_editTextChanged(QString )
{
		//

}


void MainWindow::on_action_print_act_triggered()
{
	QString s;
	QString rpt_h;
	QSettings settings;
	rpt_h = settings.value("all_report_header", "").toString();
	if(rpt_h.length() > 0) {
		htm=rpt_h;
	}
	else {
		htm="<html><head><title></title></head><body style='font-family: Old Standard TT'>";
		// DejaVu Serif
		htm += trUtf8("<table align=center width=100% cellspacing=0 cellpadding=2 border=0><tbody>");
		htm += trUtf8("<tr><td style='font-family: Georgia;font-weight: bold;font-style: italic'>E-mail: <a href='mailto:fabrikasnov@mail.ru'>fabrikasnov@mail.ru</a><br><a href='http://www.fabrikasnov.com/'>www.FabrikaSnov.com</a><br>тел.:8(928)304-304-8; 8(8652)233-669</td><td align=right ><img src=\":/&lt;root&gt;/icons/fab.png\" height=45 width=191>");
		//htm += trUtf8("<h1 style='font-family: SC_Solnyshko, Times New Roman;font-style: italic;padding-bottom: 0px;margin-bottom: 0px'>Фабрика Снов</h1><h5 style='margin-top: 0px;padding-top: 0px'>Orthopedic mattress</h5>");
		htm += trUtf8("</td></tr></tbody>");
		htm += trUtf8("</table>");
		htm += trUtf8("<hr style='margin: 0px;padding: 0px;'>");
		htm += trUtf8("<p style='font-family: Georgia;font-weight: bold;font-style: italic;margin: 0px;padding: 0px;'>355000 г. Ставрополь, ул. Мичурина, 55а</p>");
	};
	htm += trUtf8("<table align=center width=100% cellspacing=0 cellpadding=2 border=0><tr>");
	htm += trUtf8("<td align=center style='padding-left: 150px;font-family: Times New Roman;'><h1>Акт приема-сдачи товара</h1></td><td width=150 valign=bottom>дата:<u>")+ ui->dateTimeEdit_delivery->date().toString("dd.MM.yyyy") + trUtf8("</u></td>");
	htm += trUtf8("</tr></table>");

	htm += trUtf8("<table align=center width=100% border=1 cellspacing=-1 cellpadding=2 style='border-color: #000000'><tr>");
	htm += trUtf8("<td align=center width=40%>Адрес доставки:</td><td width=60% align=center>")+ui->comboBox_customer->currentText()+"</td></tr>";
	htm += trUtf8("<tr><td><b>Город:</b> ")+ ui->lineEdit_city->text() + trUtf8("<br><b>Улица:</b> ")+ui->lineEdit_street->text() +trUtf8(" <b>Дом:</b> ")+ui->lineEdit_house->text() +trUtf8(" <b>Кв.:</b> ")+ui->lineEdit_flat->text() +trUtf8("<br><b>Контактное лицо:</b> ")+ui->lineEdit_contact->text()+trUtf8("<br><b>Телефон:</b> ")+ui->lineEdit_phone->text();

	htm += trUtf8("<table width=100% cellspacing=-1 cellpadding=1 border=1><thead><tr><th>Сумма долга</th><th>подъём</th><th>этаж</th></tr></thead><tbody>");
	s.setNum(ui->spinBox_lift->value());

	htm += trUtf8("<tr><td align=center>")+ ui->label_credit->text() + trUtf8("</td><td align=center>") + ui->lineEdit_lift_price->text() +
				 trUtf8("</td><td align=center>") + s;
	htm += trUtf8("</td></tr></tbody></table>");
	htm +=  trUtf8("</td>");
	htm += trUtf8("<td width=60% style='padding: 1px'>");


	htm += trUtf8("<table width=99% cellspacing=-1 cellpadding=1 border=1><thead><tr><th>№</th><th>Наименование</th><th>Размер</th><th>Кол-во</th></tr></thead><tbody>");
	for(int row=0;row<ui->tableWidget_ordered->rowCount();row++){
		htm += trUtf8("<tr>");
		int count = 0;

		s.setNum(row+1);htm += trUtf8("<td align=right width=15 style='border-style: solid'>") + s + trUtf8("</td>");
		QComboBox *qe_n = static_cast<QComboBox *>(ui->tableWidget_ordered->cellWidget(row,0));
		if (qe_n) {
			htm += trUtf8("<td>")+ qe_n->currentText();
		}



		QComboBox *qe_c = static_cast<QComboBox *>(ui->tableWidget_ordered->cellWidget(row,1));
		if (qe_c) {
			htm += trUtf8(", ")+ qe_c->currentText();
		}
		htm += trUtf8("</td><td align=center>");
		QLineEdit *qe_s = static_cast<QLineEdit *>(ui->tableWidget_ordered->cellWidget(row,2));
		if (qe_s) {
			if(!qe_s->text().isEmpty())	htm += qe_s->text();
		}
		htm += trUtf8("</td>");


		QSpinBox *qsb = static_cast<QSpinBox *>(ui->tableWidget_ordered->cellWidget(row,3));
		if (qsb) {
			count = qsb->value();
		}

		s.setNum(count);
		htm += trUtf8("<td align=center width=10>") + s + trUtf8("</td>");
		htm += trUtf8("</tr>");
	}
	htm += trUtf8("</tbody></table>");
/*
	htm += trUtf8("<p><b>Отпустил Ф.И.О.</b><u>");
	for(int i=0;i<25;i++) htm += trUtf8("&nbsp;");
	htm += trUtf8("</u><b>подпись</b><u>");
	for(int i=0;i<10;i++) htm += trUtf8("&nbsp;");
	htm += trUtf8("</u></p>");
*/
	htm += trUtf8("<table width=100%><tr><td width=100 align=right>Отпустил Ф.И.О.:</td><td><u>&nbsp; "
								"&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;"
								" &nbsp;</u></td><td width=40 align=right>подпись:</td><td width=70><u>"
								"&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;"
								"&nbsp;&nbsp;</u></td></tr></table>");

	htm += trUtf8("</td></tr></table>");

	QString act_mx;

	act_mx = settings.value("act_mx", "").toString();
	if(act_mx.length() > 0) {
		htm+=act_mx;
	}
	else {
		htm += trUtf8("<p style='margin: 1px;padding: 1px;font-size: 8px'>Внимание! Товар сертифицирован и находится на гарантии. Гарантийным талоном является этикетка с указанными на ней наименованием фирмы изготовителя, ее фактического адреса, наименованием модели, даты изготовления и номера технической документации по которой был произведен товар. В сервисный центр на гарантийное обслуживание товар доставляется за счет покупателя. Срок гарантии 18 месяцев со дня покупки.</p>");
		htm += trUtf8("<p style='margin: 1px;padding: 1px;font-size: 8px'>Обращаем ваше внимание на то, что большинство наших товаров не подлежат возврату или обмену, так как входят в «Перечень непродовольственных товаров надлежащего качества, не подлежащих возврату или обмену на аналогичный товар других размера, формы, габарита, фасона, расцветки или комплектации». Постановление Правительства РФ №55 от 19.01.1998г. п.8 Не подлежит возврату и обмену мебель бытовая (по ГОСТу 19917-93 матрас – мебель для сидения и лежания).</p>");
		htm += trUtf8("<p style='margin: 1px;padding: 1px;font-size: 8px'>Правила эксплуатации:</p><ol style='margin: 1px;padding: 1px;font-size: 8px'>");
		htm += trUtf8("<li>Матрас должен лежать на ровной поверхности или на специальном ортопедическом основании «Титан» компании «Фабрика Снов».</li>");
		htm += trUtf8("<li>Запрещено складывать, сгибать или сворачивать пружинный матрас.</li>");
		htm += trUtf8("<li>Качественные матрасы компании «Фабрика Снов» достаточно переворачивать  1 раз в полгода (min.).</li>");
		htm += trUtf8("<li>Матрас нельзя подвергать действию влаги и прямых солнечных лучей, вследствие чего могут возникнуть повреждения чехла или внутренних слоев.</li>");
		htm += trUtf8("<li>Не разрешается на матрас становиться, прыгать или осуществлять другие грубые воздействия на поверхность матраса.</li>");
		htm += trUtf8("<li>Не рекомендуется сидеть на краю матраса, этим вы можете необратимо деформировать его конструкцию.");
		htm += trUtf8("<br>Данная рекомендация не относится к матрасам компании «Фабрика Снов» т.к. ею применяется инновационная система жесткого каркаса System Strong Frame (SSF), которая позволяет поддерживать форму матраса по периметру в первозданном виде на протяжении долгого срока эксплуатации.</li>");
		htm += trUtf8("<li> При эксплуатации матраса рекомендуется использовать наматрасник это позволит защитить обивку матраса.</li>");
		htm += trUtf8("<li>Запрещено превышать допустимую нагрузку на матрас.</li>");
		htm += trUtf8("<li>За изделие, изготовленное по индивидуальному заказу клиента (не стандарт) компания &laquo;Фабрика Снов&raquo; ответственность не несёт.</li></ol>");
	}

	htm += trUtf8("<p><b>С вышеуказанными правилами ознакомлен и согласен. Товар надлежащего качества и количества получил<br>  Ф.И.О.<u>&nbsp; &nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp; &nbsp;</u> подпись <u>&nbsp;"
								"&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;"
								"&nbsp;</u></b></p>");

	htm = "<table width=100% height=100% border=0 cellspacing=0 cellpadding=0><tr><td height=49%>" + htm + "</td></tr><tr><td><hr></td></tr><tr><td height=49%>" + htm + "</td></tr></table>";

	htm += trUtf8("</body>");
	htm += trUtf8("</html>");
	DialogPrintPreview prv;
	prv.set_doc(htm);
	prv.exec();
}

void MainWindow::on_action_help_triggered()
{

	QProcess*   m_process    = new QProcess(this);
	m_process->start("explorer file://" + QApplication::applicationDirPath() + "/index.html");

}

void MainWindow::on_lineEdit_city_returnPressed()
{
	ui->lineEdit_street->setFocus();
}

void MainWindow::on_lineEdit_street_returnPressed()
{
		ui->lineEdit_house->setFocus();
}

void MainWindow::on_lineEdit_house_returnPressed()
{
		ui->lineEdit_flat->setFocus();
}

void MainWindow::on_lineEdit_flat_returnPressed()
{
		ui->lineEdit_phone->setFocus();
}

void MainWindow::on_lineEdit_phone_returnPressed()
{
		ui->lineEdit_contact->setFocus();
}

void MainWindow::on_lineEdit_contact_returnPressed()
{
		ui->lineEdit_delivery_price->setFocus();
}

void MainWindow::on_lineEdit_delivery_price_returnPressed()
{
		ui->dateTimeEdit_delivery->setFocus();
}

void MainWindow::on_lineEdit_lift_price_returnPressed()
{
		ui->lineEdit_payment->setFocus();
}

void MainWindow::on_lineEdit_payment_returnPressed()
{
		ui->pushButton_save->setFocus();
}

void MainWindow::on_pushButton_print_check_clicked()
{
	MainWindow::on_action_print_check_triggered();
}

void MainWindow::on_pushButton_act_clicked()
{
	MainWindow::on_action_print_act_triggered();
}

void MainWindow::on_lineEdit_delivery_price_editingFinished()
{
	table_recalculate(0);
}

void MainWindow::on_lineEdit_lift_price_editingFinished()
{
	table_recalculate(0);
}

void MainWindow::on_pushButton_begin_search_order_toggled(bool checked)
{
		//
	QSqlQueryModel * model = new QSqlQueryModel(0);
	QString sql = "SELECT `orders`.`id`,strftime('%d.%m.%Y %H:%M',`orders`.dt),"
								"(SELECT `ordered`.`name`|| ', '|| `ordered`.`size`|| ', '||`ordered`.`cover` FROM `ordered` WHERE `ordered`.`porder`=orders.id LIMIT 1) bind"
								",customers.fio,city || ' ' ||  street || ' ' || house || ' ' || flat,phone,contact,paid, "
								" (SELECT strftime('%d.%m.%Y %H:%M',shipping.dt) FROM ordered "
								" LEFT JOIN shipped   ON (ordered.id  =  shipped.ordered) "
								" LEFT JOIN shipping  ON (shipping.id =  shipped.shipping) WHERE ordered.porder = orders.id)"

								" FROM orders LEFT  JOIN customers ON (customers.id = orders.customer)";

	if (checked)	{
		switch (ui->comboBox_order_search->currentIndex()) {
		case 0: {
				sql += "WHERE customer LIKE '%"+ui->lineEdit_order_search->text()+"%'";
				sql += " OR city LIKE '%"+ui->lineEdit_order_search->text()+"%'";
				sql += " OR street LIKE '%"+ui->lineEdit_order_search->text()+"%'";
				sql += " OR phone LIKE '%"+ui->lineEdit_order_search->text()+"%'";
				sql += " OR contact LIKE '%"+ui->lineEdit_order_search->text()+"%'";
			};break;
		case 1:{
				sql += "WHERE dt LIKE '%"+ui->lineEdit_order_search->text()+"%' or strftime('%d.%m.%Y %H:%M',`orders`.dt) LIKE '%"+ui->lineEdit_order_search->text()+"%'";
			};break;
		case 2:{
				sql += "WHERE customers.fio LIKE '%"+ui->lineEdit_order_search->text()+"%'";
			};break;
		case 3:{
				sql += "WHERE city LIKE '%"+ui->lineEdit_order_search->text()+"%'";
				//sql += " OR city LIKE '%"+ui->lineEdit_order_search->text()+"%'";
				sql += " OR street LIKE '%"+ui->lineEdit_order_search->text()+"%'";
				sql += " OR house LIKE '%"+ui->lineEdit_order_search->text()+"%'";
				sql += " OR flat LIKE '%"+ui->lineEdit_order_search->text()+"%'";
			};break;
		case 4:{
				sql += "WHERE orders.id LIKE '%"+ui->lineEdit_order_search->text()+"%'  ";
			};break;
		case 5:{
				sql += "INNER JOIN `ordered` ON (`orders`.`id` = `ordered`.`porder`) WHERE `orders`.`id` LIKE '%"+ui->lineEdit_order_search->text()+"%'";
				sql += " OR `ordered`.`name` LIKE '%"+ui->lineEdit_order_search->text()+"%'";
				sql += " OR `ordered`.`size` LIKE '%"+ui->lineEdit_order_search->text()+"%'";
				sql += " OR `ordered`.`cover` LIKE '%"+ui->lineEdit_order_search->text()+"%'";

			};break;

		}
	}
	model->setQuery(sql + " GROUP BY `orders`.`id` ORDER BY dt DESC LIMIT 50");

	model->setHeaderData(0, Qt::Horizontal, QObject::trUtf8("№"));
	model->setHeaderData(1, Qt::Horizontal, QObject::trUtf8("Дата"));
	model->setHeaderData(3, Qt::Horizontal, QObject::trUtf8("Покупатель"));
	model->setHeaderData(2, Qt::Horizontal, QObject::trUtf8("Товар"));
	model->setHeaderData(4, Qt::Horizontal, QObject::trUtf8("Адрес доставки"));
	model->setHeaderData(5, Qt::Horizontal, QObject::trUtf8("Телефон"));
	model->setHeaderData(6, Qt::Horizontal, QObject::trUtf8("Контакное лицо"));
	model->setHeaderData(7, Qt::Horizontal, QObject::trUtf8("Оплачено"));
	model->setHeaderData(8, Qt::Horizontal, QObject::trUtf8("Отгрузка"));

	ui->tableView_order_list->setModel(model);
	//ui->tableView_order_list->setColumnHidden(0,true);
	ui->tableView_order_list->resizeColumnsToContents();

	//ui->tableView_order_list

}

void MainWindow::on_tableView_order_list_activated(QModelIndex index)
{
		order_num = index.sibling(index.row(),0).data().toInt();
		load_order(0);
		ui->toolBox->setCurrentIndex(0);
}

void MainWindow::on_lineEdit_order_search_textChanged(QString )
{
	if (ui->pushButton_begin_search_order->isChecked())
		on_pushButton_begin_search_order_toggled(true);
}

void MainWindow::saved(bool v ){
	o_saved = v;
	if (v){
		this->setWindowTitle(trUtf8("Учет продаж"));
	}else {
		this->setWindowTitle(trUtf8("* Учет продаж"));
	}
}
// Проверка на необходимость закрытия окна
void MainWindow::closeEvent(QCloseEvent * event)
{
	if (! o_saved ){
			if (QMessageBox::question(this,trUtf8("Сохранение заказа"), trUtf8("Хотите сохранить изменения в заказе?"),QMessageBox::Yes | QMessageBox::No,QMessageBox::No ) == QMessageBox::Yes) {
			on_action_save_order_triggered();
			QMainWindow::closeEvent(event);
		}
		else
			QMainWindow::closeEvent(event);
	}
	else {
		QMainWindow::closeEvent(event);
	}
}

void MainWindow::on_action_report_retail_triggered()
{
		ReportDialog rd;
		rd.setup_report(2);
		rd.exec();
}

void MainWindow::on_toolButton_livesearch_clicked()
{
	populating_customer_list=true;
	//
	QString q_w;
	q_w=(ui->checkBox_only_wholesalers->isChecked())?" AND wholesaler > 0 ":"";


	QSqlQuery q;
	if(ui->toolButton_livesearch->isChecked()) {
		q.prepare("SELECT `customers`.`fio`,`customers`.`wholesaler`,`customers`.`id` FROM `customers` WHERE fio LIKE '%"+ui->lineEdit_customer->text()+"%' "+q_w+" ORDER BY `fio`,`id` DESC");
	}
	else {
		q.prepare("SELECT `customers`.`fio`,`customers`.`wholesaler`,`customers`.`id` FROM `customers` "+q_w.replace("AND","WHERE")+" ORDER BY `fio`,`id` DESC");
	}
	q.exec();
	ui->tableWidget_customers->setRowCount(0);
	ui->tableWidget_customers->setColumnHidden(2,true);
	while(q.next()){
		ui->tableWidget_customers->insertRow(this->ui->tableWidget_customers->rowCount());
		int row = this->ui->tableWidget_customers->rowCount()-1;
		int column = 0; {// ввод
			QTableWidgetItem *newItem = new QTableWidgetItem(q.value(0).toString());
			ui->tableWidget_customers->setItem(row, column, newItem);
		}
		column = 2; {// ввод
			QTableWidgetItem *newItem = new QTableWidgetItem(q.value(2).toString());
			ui->tableWidget_customers->setItem(row, column, newItem);
		}
		column = 1; {// ввод
			QTableWidgetItem *newItem = new QTableWidgetItem(0);
			newItem->setCheckState(Qt::Unchecked);
			if(q.value(1).toBool())		newItem->setCheckState(Qt::Checked);
			newItem->setTextAlignment(Qt::AlignHCenter);
			newItem->setFlags(Qt::ItemIsUserCheckable	| Qt::ItemIsEnabled );
			ui->tableWidget_customers->setItem(row, column, newItem);
		}
	}
	ui->tableWidget_customers->resizeColumnsToContents();
	populating_customer_list=false;
}

void MainWindow::on_tableWidget_customers_itemChanged(QTableWidgetItem* item)
{
	if (!populating_customer_list) {
		QSqlQuery query;
		query.prepare("UPDATE customers SET fio = ? , wholesaler = ? WHERE id = ? ");
		query.addBindValue( ui->tableWidget_customers->item(item->row(),0)->text() );
		query.addBindValue( ui->tableWidget_customers->item(item->row(),1)->checkState() );
		query.addBindValue( ui->tableWidget_customers->item(item->row(),2)->text() );
		if(!query.exec()){
			//
		}
		else {
			query.prepare("SELECT fio,id FROM customers WHERE fio != '' GROUP BY id ORDER BY fio");
			int ci=0;
			ci=ui->comboBox_customer->findText(ui->comboBox_customer->currentText());
			//qDebug() << ci;
			ui->comboBox_customer->setMaxCount(0);
			ui->comboBox_customer->setMaxCount(10000);
			 if (query.exec()){
				 while (query.next()) {
					 ui->comboBox_customer->addItem(query.value(0).toString());
				 }
			 }
			 else{
				 //qDebug() << query.lastError();
			 };
			ui->comboBox_customer->setCurrentIndex(ci);
			ui->comboBox_customer->setEditText(ui->comboBox_customer->itemText(ci));
			//ui->comboBox_customer->setEditText(ui->tableWidget_customers->item(item->row(),0)->text());

		}
		on_toolButton_livesearch_clicked();
	}
}


void MainWindow::on_toolButton_add_customer_clicked()
{
	ui->toolButton_livesearch->setChecked(false);
	ui->checkBox_only_wholesalers->setChecked(false);
	QSqlQuery query("INSERT INTO customers (fio,wholesaler) VALUES('',0) ");
	on_toolButton_livesearch_clicked();
	ui->tableWidget_customers->setCurrentCell(0,0,QItemSelectionModel::SelectCurrent);
	ui->tableWidget_customers->currentItem()->setSelected(true);
	ui->tableWidget_customers->setFocus();
}

void MainWindow::on_checkBox_only_wholesalers_toggled(bool checked)
{
	on_toolButton_livesearch_clicked();
}

void MainWindow::on_action_shipping_triggered()
{
	ShippingDialog sd;
	sd.set_current_order(order_num, ui->comboBox_customer->currentText());
	if (ui->toolBox->currentIndex() == 0)
		sd.set_shipping_tab(0);
	else
		sd.set_shipping_tab(1);
	sd.exec();
	on_pushButton_begin_search_order_toggled(false);
}

void MainWindow::on_pushButton_shipping_clicked()
{
	on_action_shipping_triggered();
}


QString MainWindow::plural3(int Number, int Sklonenie, int Rod, QString R5, QString R1, QString R2){
	QString p;
	int n,d1,d2,d3;
	n = abs(Number);
	d1 = n % 10;
	d2 = ((n - d1) / 10) % 10;
	d3 = ((n - 10 * d2 - d1) / 100) % 10;
	Sklonenie = 5;
	if(d2 == 1) {
		switch(d1){
		case 0:
			p = trUtf8("десять ");break;
		case 1:
			p = trUtf8("одиннадцать ");break;
		case 2:
			p = trUtf8("двенадцать ");break;
		case 3:
			p = trUtf8("тринадцать ");break;
		case 4:
			p = trUtf8("четырнадцать ");break;
		case 5:
			p = trUtf8("пятнадцать ");break;
		case 6:
			p = trUtf8("шестнадцать ");break;
		case 7:
			p = trUtf8("семнадцать ");break;
		case 8:
			p = trUtf8("восемнадцать ");break;
		case 9:
			p = trUtf8("девятнадцать ");break;
		}
	}
	else {
		switch(d2){
		case 0:
					p = "";break;
		case 2:
					p = trUtf8("двадцать ");break;
		case 3:
					p = trUtf8("тридцать ");break;
		case 4:
					p = trUtf8("сорок ");break;
		case 5:
					p = trUtf8("пятьдесят ");break;
		case 6:
					p = trUtf8("шестьдесят ");break;
		case 7:
					p = trUtf8("семьдесят ");break;
		case 8:
					p = trUtf8("восемьдесят ");break;
		case 9:
					p = trUtf8("девяносто ");break;
		}
		switch( d1) {
		case 0:
			p = p;break;
		case 1:
			switch( Rod) {
			case 2:
				p = p + trUtf8("одна ");break;
			case 3:
				p = p + trUtf8("одно ");break;
			default:
				p = p + trUtf8("один ");
			}
			Sklonenie = 1;;break;
		case 2:{
				switch( Rod){
				case 2:
					p = p + trUtf8("две ");break;
				case 3:
					p = p + trUtf8("два ");break;
				default:
					p = p + trUtf8("два ");
				}
				Sklonenie = 2;
			}
			break;
		case 3:
			p = p + trUtf8("три ");
			Sklonenie = 2;break;
								case 4:
			p = p + trUtf8("четыре ");
			Sklonenie = 2;;break;
								case 5:
			p = p + trUtf8("пять ");break;
								case 6:
			p = p + trUtf8("шесть ");break;
								case 7:
			p = p + trUtf8("семь ");break;
								case 8:
			p = p + trUtf8("восемь ");break;
								case 9:
			p = p + trUtf8("девять ");break;
		}
	}
	switch( d3){
	case 0:
		p = p;break;
	case 1:
		p = trUtf8("сто ") + p;break;
	case 2:
		p = trUtf8("двести ") + p;break;
	case 3:
		p = trUtf8("триста ") + p;break;
	case 4:
		p = trUtf8("четыреста ") + p;break;
	case 5:
		p = trUtf8("пятьсот ") + p;break;
	case 6:
		p = trUtf8("шестьсот ") + p;break;
	case 7:
		p = trUtf8("семьсот ") + p;break;
	case 8:
		p = trUtf8("восемьсот ") + p;break;
	case 9:
		p = trUtf8("девятьсот ") + p;
	}

	switch(Sklonenie){
	case 5:
		p = p + R5;break;
	case 1:
		p = p + R1;break;
	case 2:
		p = p + R2;break;
	}
	if (Number < 0)  p = trUtf8("минус ") + p;
	if (Number == 0)  p = "";

	return p;

};

/*
' Записывает прописью число с существительным
' Number - собственно число (от -2*147*483*648 до 2*147*483*647)
' Rod - род существительного (1 - муж. (по умолчанию), 2 - жен., 3 - ср.)
' R5 - существительное в форме "пять штук" (по умолчанию - "")
' R1 - существительное в форме "одна штука" (по умолчанию - "")
' R2 - существительное в форме "две штуки" (по умолчанию - "")
' Sklonenie - переменная, в которую будет помещено склонение
'    существительного (5 - "пять штук", 1 - "одна штука", 2 - "две штуки")
*/
QString MainWindow::number_in_words(int Number, int Rod ,	 QString R5 , QString R1, QString R2, int  Sklonenie) {
	QString p;
	int skl,n;
	n = abs(Number);
	Sklonenie = 5;
	skl = 5;
	p="";
	p = p + plural3(n / 1000000000, skl, 1, trUtf8("миллиардов "), trUtf8("миллиард "), trUtf8("миллиарда "));

	n = n % 1000000000;
	p = p + plural3(n / 1000000, skl, 1, trUtf8("миллионов "), trUtf8("миллион "), trUtf8("миллиона "));
	n = n % 1000000;
	p = p + plural3(n / 1000, skl, 2, trUtf8("тысяч "), trUtf8("тысяча "), trUtf8("тысячи "));
	n = n % 1000;
	p = p + plural3(n, Sklonenie, Rod, R5, R1, R2);
	if (n == 0)  p = p + R5;
	if (Number < 0)  p = trUtf8("минус ") + p;
	if (Number == 0)  p = trUtf8("ноль ") + R5;

	return p;

}

void MainWindow::on_comboBox_maker_activated(QString )
{
		saved(false);
}

void MainWindow::on_action_client_list_triggered()
{
		//
	ui->toolBox->setCurrentIndex(2);
	ui->tabWidget->setCurrentIndex(0);
}

void MainWindow::on_action_spr_merchandise_triggered()
{
	ui->toolBox->setCurrentIndex(2);
	ui->tabWidget->setCurrentIndex(1);
	ui->comboBox->setCurrentIndex(1);
	on_comboBox_activated(1);
}

void MainWindow::on_toolButton_remove_customer_clicked()
{
	if (QMessageBox::question(this,trUtf8("Удаление заказчика"), trUtf8("Покупатель будет удален безвозвратно.\nВся информация связанная с ним будет утеряна."),QMessageBox::Yes | QMessageBox::No,QMessageBox::No ) == QMessageBox::Yes) {

		QSqlQuery query;
		query.prepare("DELETE FROM customers WHERE id = ? ");
		query.addBindValue( ui->tableWidget_customers->item(ui->tableWidget_customers->currentItem()->row(),2)->text() );
		if(query.exec()){
			on_toolButton_livesearch_clicked();

			QMessageBox::information(this,trUtf8("Удаление успешно"),trUtf8("Покупатель удален успешно."),trUtf8("OK"));
		}
	}

}

void MainWindow::on_action_report_wholesalers_triggered()
{
	ReportDialog rd;
	rd.setup_report(4);
	rd.exec();

}

void MainWindow::on_comboBox_order_search_activated(QString )
{
		//
	on_pushButton_begin_search_order_toggled(ui->pushButton_begin_search_order->isChecked());
}
