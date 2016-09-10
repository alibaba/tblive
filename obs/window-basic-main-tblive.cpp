
#include <time.h>
#include <obs.hpp>
#include <QMessageBox>
#include <QShowEvent>
#include <QDesktopServices>
#include <QFileDialog>
#include <QDesktopWidget>
#include <util/dstr.h>
#include <util/util.hpp>
#include <util/platform.h>
#include <util/profiler.hpp>

#include "obs-app.hpp"
#include "platform.hpp"
#include "visibility-item-widget.hpp"
#include "item-widget-helpers.hpp"
#include "window-basic-settings.hpp"
#include "window-namedialog.hpp"
#include "window-basic-source-select.hpp"
#include "window-basic-main.hpp"
#include "window-basic-main-outputs.hpp"
#include "window-basic-properties.hpp"
#include "window-log-reply.hpp"
#include "window-projector.hpp"
#include "window-remux.hpp"
#include "qt-wrappers.hpp"
#include "display-helpers.hpp"
#include "volume-control.hpp"
#include "remote-text.hpp"

#include "ui_OBSBasic.h"


#include "base/json/json_helper.h"
#include "base/command_line.h"
#include "base/bind.h"
#include "base/path_service.h"
#include "base/base_util.h"
#include "base/time/time.h"
#include "base/logshell.h"

#include "tblive_sdk/tblive_sdk.h"
#include "tblive_sdk/biz.h"
#include "commonToolFun.h"
#include "shell/tblive_cmd_dispatcher.h"

#include "switches.h"
#include "login/LoginBiz.h"
#include "task_pool.hpp"
#include <windowsx.h>
#include "window-hoverwidget.hpp"
#include "net/http_client/http_client_helper.h"

namespace {

	bool IsSourceAvailable( const char * sName ) 
	{
		static std::set<std::string> s_sourceNames;

		if ( s_sourceNames.empty() )
		{
			const char *type = nullptr;
			size_t idx = 0;
			while (obs_enum_input_types(idx++, &type)) {
				// const char *name = obs_source_get_display_name(type);
				if (strcmp(type, "scene") == 0)
					continue;

				s_sourceNames.insert(type);
			}
		}

		return s_sourceNames.find(sName) != s_sourceNames.end();
	}


}// namespace

static CLogShell tbliveLog("tblive");

Q_DECLARE_METATYPE(OBSSceneItem);

template <typename T>
static T GetOBSRef(QListWidgetItem *item)
{
	return item->data(static_cast<int>(QtDataRole::OBSRef)).value<T>();
}


struct TBLiveSourceConfig
{
	TBLiveSourceConfig() 
		: image_source(true) 
		, ffmpeg_source(true)
		, text_ft2_source(true)
		, window_capture(true)
		, monitor_capture(true)
		, dshow_input(true)
		, decklink_input(true)
	{}

	bool image_source;
	bool ffmpeg_source;
	bool text_ft2_source;
	bool window_capture;
	bool monitor_capture;
	bool dshow_input;
	bool decklink_input;
};

EX_SL_CLASS_MAPPING_BEGIN(TBLiveSourceConfig)
EX_SL_MEMBER_V(image_source);
EX_SL_MEMBER_V(ffmpeg_source);
EX_SL_MEMBER_V(text_ft2_source);
EX_SL_MEMBER_V(window_capture);
EX_SL_MEMBER_V(monitor_capture);
EX_SL_MEMBER_V(dshow_input);
EX_SL_MEMBER_V(decklink_input);
EX_SL_CLASS_MAPPING_END()


// OBSBasic

void OBSBasic::InitTbliveUI()
{
	#if defined(DISABLE_OBS_UI)

	statusbar = new OBSBasicStatusBar(this);
	QPalette pe;
	pe.setColor(QPalette::WindowText, Qt::gray);
	statusbar->setPalette(pe);

	statusbar->setObjectName(QStringLiteral("statusbar"));
	statusbar->setMaximumHeight(19);
	statusbar->setSizeGripEnabled(false);
	ui->centralwidgetLayout->addWidget(statusbar);

	// visible
	ui->menubar->setVisible(false);
	ui->recordButton->setVisible(false);
	ui->modeSwitch->setVisible(false);

	// set obs control area hidden
 	ui->obsSceneControl->setVisible(false);

#endif	

	m_resizeBorderWidth = 8;
	m_nDragMode = Drag_Undefine;

	setWindowFlags(Qt::FramelessWindowHint);

	//用户昵称显示
	QString strUserName = QString::fromWCharArray(CLoginBiz::GetInstance()->GetLoginedUser().GetString());
	QString strElideNick = GetElidedText(ui->label_titlebar_nick->font(), strUserName, ui->label_titlebar_nick->width());
	ui->label_titlebar_nick->setText(strElideNick);
	if (strUserName != strElideNick)
	{
		ui->label_titlebar_nick->setToolTip(strUserName);
	}

	ui->pushbt_main_restore->setVisible(false);

	ui->pauseStreamBtn->setVisible(false);
	ui->restartStreamBtn->setVisible(false);
	ui->stopStreamBtn->setVisible(false);
	ui->switchSceneBtn->setVisible(false);

	ui->sceneTabBar->setDrawBase(false);
	ui->sceneTabBar->setExpanding(false);

	connect(ui->sceneTabBar, SIGNAL(currentChanged(int)), this, SLOT(CurrentTabSceneChanged(int)));
}

void OBSBasic::OBSInitFinish()
{
	if (!IsSourceAvailable("decklink-input"))
	{
		ui->selectBlackMagic->setVisible(false);
	}
}
void OBSBasic::ImpInstallChildrenEventFilter(QWidget *pW)
{
	QList<QWidget *> pWidgetList = pW->findChildren<QWidget *>();
	for (int i = 0; i < pWidgetList.count(); i++)
	{
		pWidgetList.at(i)->installEventFilter(this);
		ImpInstallChildrenEventFilter(pWidgetList.at(i));
	}
}

void OBSBasic::AddSourceWithProperty(const char *id)
{
	size_t curTime = base::Time::Now().ToTimeT();

	const char *src_name = obs_source_get_display_name(id);

	std::ostringstream os;
	os << src_name << "_" << curTime;

	std::string name = os.str();

	OBSScene scene = GetCurrentScene();
	if (!scene)
		return;

	obs_source_t * source = obs_source_create(id, name.c_str(), NULL, nullptr);
	if (source) {
		obs_scene_atomic_update(scene, [](void *data, obs_scene_t *scene){
			obs_scene_add(scene, (obs_source_t *)data);
		}, source);
	}

	CreatePropertiesWindow(source, true);
	obs_source_release(source);
}

void OBSBasic::on_selectBlackMagic_clicked()
{
	AddSourceWithProperty("decklink-input");
}

void OBSBasic::on_selectCamera_clicked()
{
	AddSourceWithProperty("dshow_input");
}

void OBSBasic::on_selectLocalVideo_clicked()
{
	AddSourceWithProperty("ffmpeg_source");
}

void OBSBasic::on_selectPic_clicked()
{
	AddSourceWithProperty("image_source");
}

void OBSBasic::on_selectWindow_clicked()
{
	if (!IsAeroEnabled())
	{
		QMessageBox msgBox(this);
		msgBox.setText(QApplication::translate("OBSBasic", "TBLive.Aero.Disabled", 0));
		msgBox.addButton(QTStr("ok"), QMessageBox::AcceptRole);
		msgBox.setIcon(QMessageBox::Information);
		msgBox.setWindowTitle(QTStr("TBLive.MsgTip"));
		msgBox.exec();
	}

	AddSourceWithProperty("window_capture");
}

void OBSBasic::on_selectMonitor_clicked()
{
	AddSourceWithProperty("monitor_capture");
}

void OBSBasic::DispatchTBLiveCmd(std::wstring cmd, std::map<std::wstring, std::wstring> args)
{
	if ( cmd == L"start_live" )
	{
		if (!m_bServiceInited)
		{
			// async init
			InitServiceData(base::Bind(&OBSBasic::DispatchTBLiveCmd, base::Unretained(this), cmd, args));
			return;
		}

		std::wstring biz = args[L"biz"];

		// tblive protocol match config
		auto fit = std::find_if(m_serviceData.service_list.begin(), m_serviceData.service_list.end(), [biz](ServiceItem item) {
			return biz == item.biz;
		});
		if (fit == m_serviceData.service_list.end())
		{// Not match
			return;
		}

		std::wstring url = args[L"url"];
		std::wstring loginuser = args[L"loginuser"];

		PrgString user = CLoginBiz::GetInstance()->GetLoginedUser();
		if (!loginuser.empty() && loginuser != user.GetString())
		{
			// Notify user not match
			QMessageBox messageBox(this);

			messageBox.setWindowTitle(QTStr("TBLive.NotifyTitle"));
			messageBox.setText(QTStr("TBLive.NotifyLoginUserNotMatch"));
			messageBox.exec();
			return;
		}

		// Notify if stream active
		if (outputHandler->StreamingActive())
		{
			BringToTop();

			QMessageBox messageBox(this);

			messageBox.setWindowTitle(QTStr("TBLive.NotifyTitle"));
			messageBox.setText(QTStr("TBLive.ChangeLiveRoomWhenActive"));
			messageBox.exec();

			return;
		}

		PrgString destUrl;
		CLoginBiz::GetInstance()->TranslateUrl(url, destUrl);

		ui->bizWebview->load(QString::fromStdWString(destUrl.GetString()));

		BringToTop();
	}
	else if ( cmd == L"set_focus" )
	{
		BringToTop();
	}
}

void OBSBasic::BringToTop()
{
	showNormal();

	::SetWindowPos((HWND)winId(), HWND_TOPMOST, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE | SWP_SHOWWINDOW);
	::SetWindowPos((HWND)winId(), HWND_NOTOPMOST, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE | SWP_SHOWWINDOW);
}

void OBSBasic::TBLiveNeedUpdate(std::string ver, std::string msg, std::string download_url)
{
	QString     str = QTStr("TBLiveUpdateAvailable.Text");
	QMessageBox messageBox(this);

	str = str.arg(QT_UTF8(ver.c_str()),	QT_UTF8(download_url.c_str()));

	messageBox.setWindowTitle(QTStr("TBLiveUpdateAvailable"));
	messageBox.setTextFormat(Qt::RichText);
	messageBox.setText(str);
	messageBox.setInformativeText(QT_UTF8(msg.c_str()));
	messageBox.exec();
}

void OBSBasic::InitServiceData(base::Closure callback)
{
	if ( m_bServiceInited )
	{
		return;
	}

	task_pool::PostTask(base::Lambda([=](){
		base::FilePath exepath;
		PathService::Get(base::FILE_EXE, &exepath);
		base::FilePath path = exepath.DirName().DirName().DirName().Append("data/obs-studio/service_list.json");

		ServiceData serviceData;
		json::JsonFileToClass(base::FilePath(path), serviceData);

		base::GetUIMessageLoop()->PostTask(FROM_HERE, base::Lambda([=](){
			m_serviceData = serviceData;
			m_bServiceInited = true;

			if ( !callback.is_null() )
			{
				callback.Run();
			}			
		}));
	}));
}

void OBSBasic::OnTaoBaoTokenOk()
{
	{// refresh cookie
		if (!m_hideWebview)
		{
			QMainWindow * hideSSO = new QMainWindow(this);
			m_hideWebview = new QWebEngineView(this);

			hideSSO->setCentralWidget(m_hideWebview);
			hideSSO->hide();

			base::GetUIMessageLoop()->PostDelayedTask(FROM_HERE, base::Lambda([=]() {
				std::wstring url = L"https://my.taobao.com/";
				PrgString destUrl;
				CLoginBiz::GetInstance()->TranslateUrl(url, destUrl);

				m_hideWebview->load(QString::fromStdWString(destUrl.GetString()));
			}), base::TimeDelta::FromSeconds(30));
		}
		else
		{
			// load sso hidden page 
			std::wstring url = L"https://my.taobao.com/";
			PrgString destUrl;
			CLoginBiz::GetInstance()->TranslateUrl(url, destUrl);

			m_hideWebview->load(QString::fromStdWString(destUrl.GetString()));
		}
	}

	// Init webview for first time
	if (!m_bBizWebviewInited)
	{
		InitBizWebview();

		m_bBizWebviewInited = true;
	}
}

void OBSBasic::InitBizWebview()
{
	if (!m_bServiceInited)
	{
		// async init
		InitServiceData(base::Bind(&OBSBasic::InitBizWebview, base::Unretained(this)));
		return;
	}

	// dispatch command line if exist
	if (!CTBLiveShell::GetInstance()->IsEmpty())
	{
		std::map<std::wstring, std::wstring> cmds = CTBLiveShell::GetInstance()->GetCmds();
		DispatchTBLiveCmd(switches::cmdStartLive, cmds);
	}
	else
	{
		// Show default main url
		if (!m_serviceData.service_list.empty())
		{
			std::wstring url = m_serviceData.service_list[0].main_url;
			if (!url.empty())
			{
				PrgString destUrl;
				CLoginBiz::GetInstance()->TranslateUrl(url, destUrl);

				ui->bizWebview->load(QString::fromStdWString(destUrl.GetString()));
			}
		}
	}
}

void OBSBasic::on_tabAddSceneBtn_clicked()
{
	QString format{ QTStr("Basic.Main.DefaultSceneName.Text") };

	QString sceneName = format.arg(m_curNewSceneNameIndex++);

	{
		std::string name = QT_TO_UTF8(sceneName);
		obs_scene_t *scene = obs_scene_create(name.c_str());

		obs_source_t *source = obs_scene_get_source(scene);
		AddScene(source);
		SetCurrentScene(source);
		obs_scene_release(scene);
	}
}

void OBSBasic::CurrentTabSceneChanged(int idx)
{
	QString sceneName = ui->sceneTabBar->tabText(idx);
	obs_source_t * source = obs_get_source_by_name(QT_TO_UTF8(sceneName));
	if ( source )
	{
		SetCurrentScene(source);
		CurrentTabSceneUpdateControls();
		obs_source_release(source);
	}
}

void OBSBasic::SetTabStreamingStatus(QString sceneName)
{
	for (int i = 0; i < ui->sceneTabBar->count(); ++i)
	{
		bool isCurrent = false;
		if (ui->sceneTabBar->tabText(i) == sceneName)
		{
			isCurrent = true;
			// Set streaming status
			ui->sceneTabBar->setTabIcon(i, QIcon(":/res/images/scene_status_streaming.png"));
		}
		else
		{
			// clear
			ui->sceneTabBar->setTabIcon(i, QIcon(""));
		}

		QWidget * rightButton = ui->sceneTabBar->tabButton(i, QTabBar::RightSide);
		if ( rightButton )
		{
			rightButton->setVisible(!isCurrent);
		}
	}
}

void OBSBasic::CurrentTabSceneUpdateControls()
{
	bool streaming_started = outputHandler->Active();

	if (m_bIsStartedOnceBefore)
	{
		if (streaming_started)
		{// started
			if (m_streamingScene == GetCurrentSceneSource())
			{
				ui->switchSceneBtn->setVisible(false);

				ui->pauseStreamBtn->setVisible(true);
				ui->stopStreamBtn->setVisible(true);
			}
			else
			{
				ui->switchSceneBtn->setVisible(true);

				ui->pauseStreamBtn->setVisible(false);
				ui->stopStreamBtn->setVisible(false);
			}
		}
		else
		{// stopped
			ui->switchSceneBtn->setVisible(false);

			ui->restartStreamBtn->setVisible(true);
			ui->stopStreamBtn->setVisible(true);
		}
	}
}

void OBSBasic::changeEvent(QEvent *event)
{
	if (event->type() == QEvent::WindowStateChange)
	{
		const Qt::WindowStates nState = this->windowState();
		if (nState == Qt::WindowMaximized)
		{
			ui->pushbt_main_restore->show();
			ui->pushbt_main_max->hide();
		}
		else if (nState == Qt::WindowNoState)
		{
			ui->pushbt_main_restore->hide();
			ui->pushbt_main_max->show();
		}
	}

	__super::changeEvent(event);
}

bool OBSBasic::nativeEvent(const QByteArray &eventType, void *message, long *result)
{
	MSG* msg = (MSG*)message;
	switch (msg->message)
	{
		//因为我们截获了自定义标题栏区域的WM_NCHITTEST，所以此区域鼠标双击也变成了WM_NCLBUTTONDBLCLK，而非WM_LBUTTONDBLCLK
	case WM_NCLBUTTONDBLCLK:
		if (isMaximized())
		{
			showNormal();
		}
		else
		{
			showMaximized();
		}
		return true;

	case WM_NCHITTEST:
		{
			int xPos = GET_X_LPARAM(msg->lParam) - this->frameGeometry().x();
			int yPos = GET_Y_LPARAM(msg->lParam) - this->frameGeometry().y();

			if (HitTestDrag(QPoint(xPos, yPos), result))
			{
				return true;
			}
		}

		break;
	}
	return false;
}
bool OBSBasic::HitTestDrag(QPoint mouseWndPos, long *result)
{
	//优先判断对角线方向的resize
	if (mouseWndPos.y() <= m_resizeBorderWidth && mouseWndPos.x() <= m_resizeBorderWidth)
	{
		*result = HTTOPLEFT;
		return true;
	}
	else if (mouseWndPos.y() <= m_resizeBorderWidth && mouseWndPos.x() >= this->width() - m_resizeBorderWidth)
	{
		*result = HTTOPRIGHT;
		return true;
	}
	else if (mouseWndPos.y() >= this->height() - m_resizeBorderWidth && mouseWndPos.x() >= this->width() - m_resizeBorderWidth)
	{
		*result = HTBOTTOMRIGHT;
		return true;
	}
	else if (mouseWndPos.y() >= this->height() - m_resizeBorderWidth && mouseWndPos.x() <= m_resizeBorderWidth)
	{
		*result = HTBOTTOMLEFT;
		return true;
	}

	//然后判断平拉拖动
	if (mouseWndPos.x() <= m_resizeBorderWidth)
	{
		*result = HTLEFT;
		return true;
	}
	else if (mouseWndPos.x() >= this->width() - m_resizeBorderWidth)
	{
		*result = HTRIGHT;
		return true;
	}
	else if (mouseWndPos.y() >= this->height() - m_resizeBorderWidth)
	{
		*result = HTBOTTOM;
		return true;
	}
	else if (mouseWndPos.y() <= m_resizeBorderWidth)
	{
		*result = HTTOP;
		return true;
	}
	else
	{
		QWidget *childAtWidget = childAt(mouseWndPos);
		if (ui->titlewidget == childAtWidget || ui->tabArea == childAtWidget)
		{
			*result = HTCAPTION;
			return true;
		}
	}

	return false;
}


void OBSBasic::on_startStreamBtn_clicked()
{
	tbliveLog.Log(lss_info, L"on_startStreamBtn_clicked");

	if (!outputHandler->StreamingActive()) {
		ui->startStreamBtn->setEnabled(false);
		StartStreaming();
	}
}

void OBSBasic::on_switchSceneBtn_clicked()
{
	tbliveLog.Log(lss_info, L"on_switchSceneBtn_clicked");

	OBSScene curScene = GetCurrentScene();
	obs_source_t * source = obs_scene_get_source(curScene);
	SetCurrentScene(source, false);

	TransitionToScene(source, false);

	m_streamingScene = source;
	SetTabStreamingStatus(QT_UTF8(obs_source_get_name(m_streamingScene)));

	CurrentTabSceneUpdateControls();
}

void OBSBasic::on_pauseStreamBtn_clicked()
{
	tbliveLog.Log(lss_info, L"on_pauseStreamBtn_clicked");

	if (outputHandler->StreamingActive()) {
		// Already streaming
		QMessageBox::StandardButton button =
			QMessageBox::question(this,
			QTStr("ConfirmPause.Title"),
			QTStr("ConfirmPause.Text"));

		if (button == QMessageBox::No)
			return;

		m_bPauseClicked = true;
		ui->pauseStreamBtn->setEnabled(false);
		StopStreaming();
	}
}

void OBSBasic::on_restartStreamBtn_clicked()
{
	tbliveLog.Log(lss_info, L"on_restartStreamBtn_clicked");

	if (!outputHandler->StreamingActive()) {
		m_bRestartCliecked = true;
		ui->restartStreamBtn->setEnabled(false);
		StartStreaming();
	}
}

void OBSBasic::on_stopStreamBtn_clicked()
{
	tbliveLog.Log(lss_info, L"on_stopStreamBtn_clicked");

	if (outputHandler->StreamingActive()) {
		QMessageBox::StandardButton button =
			QMessageBox::question(this,
			QTStr("ConfirmStop.Title"),
			QTStr("ConfirmStop.Text"));

		if (button == QMessageBox::No)
			return;

		StopStreaming();
	}
}

void OBSBasic::on_pushbt_main_set_clicked()
{
	on_settingsButton_clicked();
}

void OBSBasic::on_volCtrlBtn_released()
{
	QPoint pt = QCursor::pos();
	//悬浮在鼠标上方的位置
	pt.setY(pt.ry() - ui->volCtrlBtn->height() / 2);
	m_pHoverVolCtrlWidget->ShowAt(pt.rx(), pt.ry());
}

void OBSBasic::InitHoverVolWidget()
{
	if (m_pHoverVolCtrlWidget == nullptr)
	{
		m_pHoverVolCtrlWidget = new VolHoverWidget(NULL);
		m_pHoverVolCtrlWidget->hide();
	}
}

void OBSBasic::SetSourceConfig(std::wstring sourceConfigJson)
{
	TBLiveSourceConfig sourceConfig;
	json::JsonStrToClass(sourceConfigJson, sourceConfig);

	if (IsSourceAvailable("decklink-input"))
	{
		ui->selectBlackMagic->setVisible(sourceConfig.decklink_input);
	}

	ui->selectCamera->setVisible(sourceConfig.decklink_input);
	ui->selectLocalVideo->setVisible(sourceConfig.ffmpeg_source);
	ui->selectPic->setVisible(sourceConfig.image_source);
	ui->selectWindow->setVisible(sourceConfig.window_capture);
	ui->selectMonitor->setVisible(sourceConfig.monitor_capture);
}

void OBSBasic::RemoveSceneItemFromSource(const char* name)
{
	std::string str_name = name;
	auto delayDelete = [=](){
		obs_source_t * source = obs_get_source_by_name(str_name.c_str());
		for (int i = 0; i < ui->sources->count(); i++) {
			QListWidgetItem *listItem = ui->sources->item(i);

			OBSSceneItem item = GetOBSRef<OBSSceneItem>(listItem);
			obs_source_t * item_source = obs_sceneitem_get_source(item);
			if (source == item_source) {
				DeleteListItem(ui->sources, listItem);
				obs_source_release(source);
				obs_sceneitem_remove(item);
				return;
			}
		}

		obs_source_release(source);
	};

	base::GetUIMessageLoop()->PostTask(FROM_HERE, base::Lambda(delayDelete));
}
