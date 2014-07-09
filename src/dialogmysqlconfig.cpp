#include <QtDebug>
#include <QSqlQuery>
#include <QSqlError>
#include <QMessageBox>
#include <QStyleFactory>
#include <QSettings>
#include "dialogmysqlconfig.h"
#include "ui_dialogmysqlconfig.h"
#include "mainwindow.h"
DialogMySQLConfig::DialogMySQLConfig(QWidget *parent) :
    QDialog(parent),
    m_ui(new Ui::DialogMySQLConfig)
{

  m_ui->setupUi(this);

	QSettings settings;

	//appstyle
	QString qst, rpt_h,act_mx;
	qst = settings.value("appstyle", "Windows").toString();
	rpt_h = settings.value("all_report_header", "").toString();
	if(rpt_h.length() > 0) {
		m_ui->plainTextEdit_report_header->setPlainText(rpt_h);
	}

	act_mx = settings.value("act_mx", "").toString();
	if(act_mx.length() > 0) {
		m_ui->plainTextEdit_act_mx->setPlainText(act_mx);
	}

	m_ui->comboBox_ui_style->setCurrentIndex(	m_ui->comboBox_ui_style->findText(qst));
// makers(`id` integer primary key AUTOINCREMENT, `maker` varchar, `header` varchar)

	QSqlQuery query;
	query.prepare("SELECT * FROM makers  ORDER BY id");
	if (query.exec()){
		while (query.next()) {
			m_ui->comboBox_makers->addItem(query.value(1).toString(),query.value(0).toString());
		}
	}
	else{
		//qDebug() << query.lastError();
	};
	//m_ui->comboBox_makers->setModel();
}

DialogMySQLConfig::~DialogMySQLConfig()
{
    delete m_ui;
}

void DialogMySQLConfig::changeEvent(QEvent *e)
{
    QDialog::changeEvent(e);
    switch (e->type()) {
    case QEvent::LanguageChange:
        m_ui->retranslateUi(this);
        break;
    default:
        break;
    }
}



void DialogMySQLConfig::on_buttonBox_accepted()
{

	QSettings settings;
	settings.setValue("appstyle", m_ui->comboBox_ui_style->currentText());
	settings.setValue("alwayshowprinterconfig", m_ui->checkBox_always_printer_select->checkState());
	settings.setValue("all_report_header", m_ui->plainTextEdit_report_header->toPlainText());
	settings.setValue("act_mx", m_ui->plainTextEdit_act_mx->toPlainText());
}


void DialogMySQLConfig::on_comboBox_ui_style_currentIndexChanged(int index)
{
	QApplication::setStyle(QStyleFactory::create(m_ui->comboBox_ui_style->currentText()));
}

void DialogMySQLConfig::on_toolButton_add_maker_clicked()
{
QApplication::setOverrideCursor(Qt::WaitCursor);
	QSqlQuery query;
	query.prepare("INSERT INTO makers(maker,header) VALUES(?,?)");
	query.addBindValue(m_ui->comboBox_makers->currentText());
	query.addBindValue(m_ui->textEdit_doc_header->toHtml());
	int l_maker=-1;
	if(query.exec()) {
		if(query.lastInsertId().isValid()) {
			l_maker=query.lastInsertId().toInt();
		}
	}
	m_ui->comboBox_makers->setMaxCount(0);
	query.prepare("SELECT * FROM makers  ORDER BY id");
	if (query.exec()){
		m_ui->comboBox_makers->setMaxCount(1000);
		while (query.next()) {
			m_ui->comboBox_makers->addItem(query.value(1).toString(),query.value(0).toString());
		}
	}
QApplication::restoreOverrideCursor();
	if(l_maker >=0 ) {
		m_ui->comboBox_makers->setCurrentIndex( m_ui->comboBox_makers->findData(l_maker));
		QMessageBox::information(this, trUtf8("Добавлено"),trUtf8("Добавлен новый поставщик"));
	}
}

void DialogMySQLConfig::on_comboBox_makers_currentIndexChanged(int index)
{

}

void DialogMySQLConfig::on_comboBox_makers_activated(int index)
{
	 //
	int m_id = m_ui->comboBox_makers->itemData(index).toInt();
	QSqlQuery q;
	q.prepare("SELECT header FROM makers WHERE id= ? LIMIT 1");
	q.addBindValue(m_id);
	if(q.exec()){
		while(q.next()){
			m_ui->textEdit_doc_header->setHtml(
			q.value(0).toString());
		}
	}
}

void DialogMySQLConfig::on_toolButton_save_maker_clicked()
{
	QApplication::setOverrideCursor(Qt::WaitCursor);
	int m_id = m_ui->comboBox_makers->itemData(m_ui->comboBox_makers->currentIndex()).toInt();
	QSqlQuery q;
	q.prepare("UPDATE makers SET maker = ? , header =? WHERE id= ? ");
	q.addBindValue(m_ui->comboBox_makers->currentText());
	q.addBindValue(m_ui->textEdit_doc_header->toHtml());
	q.addBindValue(m_id);
	if(q.exec()){

		m_ui->comboBox_makers->setMaxCount(0);
		QSqlQuery query;
		query.prepare("SELECT * FROM makers  ORDER BY id");

		QComboBox *qcb = this->parentWidget()->findChild<QComboBox *>("comboBox_maker");
		qcb->clear();
		if (query.exec()){
			m_ui->comboBox_makers->setMaxCount(1000);
			while (query.next()) {
				m_ui->comboBox_makers->addItem(query.value(1).toString(),query.value(0).toString());
				qcb->addItem(query.value(1).toString(),query.value(0).toString());
			}
		}
		m_ui->comboBox_makers->setCurrentIndex( m_ui->comboBox_makers->findData(m_id));

	}


	QApplication::restoreOverrideCursor();
}
