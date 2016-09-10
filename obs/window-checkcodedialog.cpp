
#include "window-checkcodedialog.hpp"
#include "obs-app.hpp"

using namespace std;

CheckCodeDialog::CheckCodeDialog(QWidget *parent)
	: QDialog(parent, Qt::WindowCloseButtonHint),
	  ui(new Ui::CheckCodeDialog)
{
	ui->setupUi(this);
}

void CheckCodeDialog::LoadUrl(QString url)
{
	ui->webview->load(QUrl(url));
}

QString CheckCodeDialog::GetCheckCode() 
{
	return ui->userText->text();
}
