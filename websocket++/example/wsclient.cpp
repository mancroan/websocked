//WebSocketClientOperate.h
#include <websocketpp/config/asio_no_tls_client.hpp>
#include <websocketpp/client.hpp>
#include <iostream>
#include <boost/thread.hpp>
 
typedef websocketpp::client<websocketpp::config::asio_client> client;
 
using websocketpp::lib::placeholders::_1;
using websocketpp::lib::placeholders::_2;
using websocketpp::lib::bind;
 
// pull out the type of messages sent by our config
typedef websocketpp::config::asio_client::message_type::ptr message_ptr;
 
enum LoginStatus
{
	LoginStatus_LoginSuccessed = 0,			//登录成功
	LoginStatus_LoginFail = 1,					//登录失败
	LoginStatus_NotLogin = 2,					//未登录
	LoginStatus_Logining = 3,					//登录中
};
 
class WebSocketClientOperate
{
public:
	WebSocketClientOperate();
	virtual ~WebSocketClientOperate();
 
	int Init(char *pHostname, unsigned short usPort,char *pBaseUri, char *pLoginUsername, char *pLoginPassword);
	int Init(char *pHostname, unsigned short usPort, char *pUri);
	int Uninit();
 
	int StartWork();
	int StopWork();
 
 
protected:
	int login();
	int logout();
 
	int ThreadProccess();
 
protected:
	//消息回调函数
	void on_message(client* c, websocketpp::connection_hdl hdl, message_ptr msg);
	//连接上服务器回调函数
	void on_open(client* c, websocketpp::connection_hdl hdl);
	//服务器断开连接回调函数
	void on_close(client* c, websocketpp::connection_hdl hdl);
	//无法连接上服务器回调函数，socket在内部已关闭上层无需再关闭
	void on_termination_handler(client* c, websocketpp::connection_hdl hdl);
 
	int closeConnect();
	int  sendTextData(char *pText);
	int  sendBinaryData(unsigned char *pData, int nSize);
 
protected:
	boost::thread * m_threadMain;
	bool m_bThreadExit;
	bool m_bIsConnectedServer;
	bool m_bSubscribeDataSuccess;
	std::string m_strHostname;
	std::string m_strWSUrl;
	std::string m_strUsername;
	client m_client;
	client::connection_ptr m_connection;
	int m_nLoginStatus;
	std::string m_strLoginSessionId;
	DWORD m_dwLoginRequestTime;
};
 
//WebSocketClientOperate.cpp
#include "stdafx.h"
#include "WebSocketClientOperate.h"
#include "InnerFunction.h"
#include <mmsystem.h>
#include "jsoncpp/include/json/json.h"
 
#ifdef _DEBUG
#pragma comment(lib,"jsoncpp/lib/debug/lib_json.lib")
#else
#pragma comment(lib,"jsoncpp/lib/release/lib_json.lib")
#endif
#pragma comment(lib,"winmm.lib")
 
#define Min_Login_Time				30000
 
WebSocketClientOperate::WebSocketClientOperate()
{
	m_threadMain = NULL;
	m_bThreadExit = true;
	m_bIsConnectedServer = false;
	m_strHostname.clear();
	m_strWSUrl.clear();
	m_strUsername.clear();
	m_strLoginSessionId.clear();
	m_connection = NULL;
	m_nLoginStatus = LoginStatus_NotLogin;
	m_bSubscribeDataSuccess = false;
	m_dwLoginRequestTime = 0;
}
 
WebSocketClientOperate::~WebSocketClientOperate()
{
}
 
int WebSocketClientOperate::Init(char *pHostname, unsigned short usPort, char *pBaseUri, char *pLoginUsername, char *pLoginPassword)
{
	m_strHostname = pHostname;
	m_strUsername = pLoginUsername;
	char szUrl[1024] = { 0 };
	sprintf_s(szUrl, "ws://%s:%d/%s?uid=%s&pwd=%s", pHostname, usPort, pBaseUri, pLoginUsername, pLoginPassword);
	m_strWSUrl = szUrl;
	try {
		// Set logging to be pretty verbose (everything except message payloads)
		m_client.set_access_channels(websocketpp::log::alevel::all);
		m_client.clear_access_channels(websocketpp::log::alevel::frame_payload);
 
		// Initialize ASIO
		m_client.init_asio();
		m_client.set_reuse_addr(true);
 
		// Register our message handler
		m_client.set_message_handler(bind(&WebSocketClientOperate::on_message, this, &m_client, ::_1, ::_2));
		m_client.set_open_handler(bind(&WebSocketClientOperate::on_open, this, &m_client, ::_1));
		m_client.set_close_handler(bind(&WebSocketClientOperate::on_close, this, &m_client, ::_1));
	}
	catch (websocketpp::exception const & e) {
		std::cout << e.what() << std::endl;
		return -2;
	}
 
	return 0;
}
int WebSocketClientOperate::Init(char *pHostname, unsigned short usPort, char *pUri)
{
	m_strHostname = pHostname;
	char szUrl[1024] = { 0 };
	sprintf_s(szUrl, "ws://%s:%d/%s", pHostname, usPort, pUri);
	m_strWSUrl = szUrl;
	try {
		// Set logging to be pretty verbose (everything except message payloads)
		m_client.set_access_channels(websocketpp::log::alevel::all);
		m_client.clear_access_channels(websocketpp::log::alevel::frame_payload);
 
		// Initialize ASIO
		m_client.init_asio();
		m_client.set_reuse_addr(true);
 
		// Register our message handler
		m_client.set_message_handler(bind(&WebSocketClientOperate::on_message, this, &m_client, ::_1, ::_2));
		m_client.set_open_handler(bind(&WebSocketClientOperate::on_open, this, &m_client, ::_1));
		m_client.set_close_handler(bind(&WebSocketClientOperate::on_close, this, &m_client, ::_1));
	}
	catch (websocketpp::exception const & e) {
		std::cout << e.what() << std::endl;
		return -2;
	}
 
	return 0;
}
int WebSocketClientOperate::Uninit()
{
	return 0;
}
 
int WebSocketClientOperate::StartWork()
{
	m_bThreadExit = false;
	m_threadMain = new boost::thread(boost::bind(&WebSocketClientOperate::ThreadProccess, this));
	if (m_threadMain == NULL)
	{
		m_bThreadExit = true;
		return -1;
	}
	else
	{
		return 0;
	}
}
int WebSocketClientOperate::StopWork()
{
	int nRet = 0;
	m_bThreadExit = true;
	m_client.stop();
	if (m_threadMain != NULL)
	{
		m_threadMain->join();
		delete m_threadMain;
		m_threadMain = NULL;
	}
	m_nLoginStatus = LoginStatus_NotLogin;
	return nRet;
}
 
int WebSocketClientOperate::login()
{
	int nRet = 0;
	try {
		//防止频繁登录
		if (m_dwLoginRequestTime != 0 && m_nLoginStatus != LoginStatus_LoginSuccessed)
		{
			DWORD dwCurrentTime = timeGetTime();
			if ((dwCurrentTime - m_dwLoginRequestTime) <= Min_Login_Time)
			{
				return 1;
			}
		}
 
		m_nLoginStatus = LoginStatus_Logining;
		m_dwLoginRequestTime = timeGetTime();
 
		websocketpp::lib::error_code ec;
		m_connection = m_client.get_connection(m_strWSUrl, ec);
		if (ec)
		{
			std::cout << "could not create connection because: " << ec.message() << std::endl;
			m_nLoginStatus = LoginStatus_LoginFail;
			return -1;
		}
 
		// Note that connect here only requests a connection. No network messages are
		// exchanged until the event loop starts running in the next line.
		m_client.connect(m_connection);
		//设置连接断开通知handler
		m_connection->set_termination_handler(bind(&WebSocketClientOperate::on_termination_handler, this, &m_client, ::_1));
	}
	catch (websocketpp::exception const & e) {
		std::cout << e.what() << std::endl;
		m_nLoginStatus = LoginStatus_LoginFail;
		return -2;
	}
	return nRet;
}
 
int WebSocketClientOperate::logout()
{
	m_nLoginStatus = LoginStatus_NotLogin;
	return -1;
}
 
int WebSocketClientOperate::ThreadProccess()
{
	while (true)
	{
		if (m_bThreadExit)
		{
			break;
		}
 
		if (!m_bIsConnectedServer)
		{
			if (m_nLoginStatus == LoginStatus_LoginFail || m_nLoginStatus == LoginStatus_NotLogin)
			{
				login();
			}
		}
		else
		{
			sendTextData("aaaaaa");
		}
		if (m_nLoginStatus == LoginStatus_LoginSuccessed)
		{
			m_client.poll_one();
			Sleep(100);
		}
		else
		{
			//首次连接之后，需要快速握手，不能等待长时间，否则websocket握手失败；websocket握手成功之后，可以等待长时间
			m_client.poll_one();
			Sleep(100);
		}
 
	}
	return 0;
}
 
// This message handler will be invoked once for each incoming message. It
// prints the message and then sends a copy of the message back to the server.
void WebSocketClientOperate::on_message(client* c, websocketpp::connection_hdl hdl, message_ptr msg) {
	std::cout << "on_message called with hdl: " << hdl.lock().get()
		<< " and message: " << msg->get_payload()
		<< std::endl;
 
	//不同业务，接收不同数据内容，解析相应的数据并处理
	std::string strRecvice = Utf8toAscii(msg->get_payload());
	Json::Reader reader;
	Json::Value root;
	bool bRet = reader.parse(strRecvice, root);
	if (bRet)
	{
		int nRetErrorCode = root["ret"].asInt();;
		int nOperateType = root["operatetype"].asInt();
		if (nOperateType == 2)									//登录应答
		{
			m_strLoginSessionId = root["sessionid"].asString();
			if (nRetErrorCode == 0)
			{
				//登录成功
				m_nLoginStatus = LoginStatus_LoginSuccessed;
			}
		}
		else
		{
 
		}
	}
}
 
void WebSocketClientOperate::on_open(client* c, websocketpp::connection_hdl hdl) {
	std::cout << "open handler" << std::endl;
	client::connection_ptr con = c->get_con_from_hdl(hdl);
	//websocketpp::config::core_client::request_type requestClient = con->get_request();
	if (con->get_ec().value() != 0)
	{
		m_bIsConnectedServer = false;
	}
	else
	{
		m_bIsConnectedServer = true;
	}
}
 
void WebSocketClientOperate::on_close(client* c, websocketpp::connection_hdl hdl)
{
	closeConnect();
	m_bSubscribeDataSuccess = false;
	m_bIsConnectedServer = false;
	m_nLoginStatus = LoginStatus_NotLogin;
}
 
void WebSocketClientOperate::on_termination_handler(client* c, websocketpp::connection_hdl hdl)
{
	closeConnect();
	m_nLoginStatus = LoginStatus_LoginFail;
	m_bIsConnectedServer = false;
}
 
int WebSocketClientOperate::closeConnect()
{
	int nRet = 0;
	try {
		if (m_connection != NULL && m_connection->get_state() == websocketpp::session::state::value::open)
		{
			websocketpp::close::status::value cvValue = 0;
			std::string strReason = "";
			m_connection->close(cvValue, strReason);
		}
	}
	catch (websocketpp::exception const & e)
	{
		std::cout << e.what() << std::endl;
		nRet = -1;
	}
	return nRet;
}
 
int  WebSocketClientOperate::sendTextData(char *pText)
{
	int nRet = 0;
	try {
		websocketpp::lib::error_code ec;
		ec = m_connection->send(pText);
		if (ec) {
			std::cout << "Echo failed because: " << ec.message() << std::endl;
			nRet = -1;
		}
	}
	catch (websocketpp::exception const & e) {
		std::cout << e.what() << std::endl;
		return -2;
	}
 
	return nRet;
}
 
int  WebSocketClientOperate::sendBinaryData(unsigned char *pData, int nSize)
{
	int nRet = 0;
	try {
		websocketpp::lib::error_code ec;
		ec = m_connection->send(pData, nSize, websocketpp::frame::opcode::binary);
		if (ec) {
			std::cout << "Echo failed because: " << ec.message() << std::endl;
			nRet = -1;
		}
	}
	catch (websocketpp::exception const & e) {
		std::cout << e.what() << std::endl;
		return -2;
	}
 
	return nRet;
}
 
//main.cpp
#include "stdafx.h"
 
#include "WebSocketClientOperate.h"
 
int main(int argc, char* argv[]) {
	WebSocketClientOperate clientWebSocketClientOperate;
	clientWebSocketClientOperate.Init("127.0.0.1", 9100,"ws", "admin", "admin");
	clientWebSocketClientOperate.StartWork();
 
	while (true)
	{
		Sleep(200);
	}
	return 0;
}
