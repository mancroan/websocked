//WebSocketServerOpreate.h
 
#pragma once
 
#include <boost/algorithm/string.hpp>
#include <string>
#include <vector>
#include <iostream>
#include <boost/thread.hpp>
#include <websocketpp/config/asio_no_tls.hpp>
#include <websocketpp/server.hpp>
 
#define			Base_Uri_Length			128
 
//名称与值数据对
struct NameAndValue
{
	std::string strName;
	std::string strValue;
};
 
typedef websocketpp::server<websocketpp::config::asio> server;
 
using websocketpp::lib::placeholders::_1;
using websocketpp::lib::placeholders::_2;
using websocketpp::lib::bind;
 
// pull out the type of messages sent by our config
typedef server::message_ptr message_ptr;
 
 
class WebSocketServerOpreate
{
public:
	WebSocketServerOpreate();
	~WebSocketServerOpreate();
 
	int Init(unsigned short usPort,char *pBaseUri="/ws");
	int Uninit();
	int StartWork();
	int StopWork();
 
protected:
	int ThreadProccess();
	void InsertClientConnection(websocketpp::connection_hdl hdl);
	void DeleteClientConnection(websocketpp::connection_hdl hdl);
 
public:
	bool validate(server *s, websocketpp::connection_hdl hdl);
	void on_http(server* s, websocketpp::connection_hdl hdl);
	void on_fail(server* s, websocketpp::connection_hdl hdl);
	//连接打开回调函数
	void on_open(server* s, websocketpp::connection_hdl hdl);
	//连接关闭回调函数
	void on_close(server *s, websocketpp::connection_hdl hdl);
	//接收websocket数据回调函数
	void on_message(server* s, websocketpp::connection_hdl hdl, message_ptr msg);
 
public:
	// 字符串分割
	static int StringSplit(std::vector<std::string>& dst, const std::string& src, const std::string& separator);
	//去前后空格
	static std::string& StringTrim(std::string &str);
	//获取请求命令与参数
	static bool GetReqeustCommandAndParmeter(std::string strUri, std::string & strRequestOperateCommand, std::vector<NameAndValue> & listRequestOperateParameter);
 
protected:
	unsigned short m_usPort;
	char m_szBaseUri[Base_Uri_Length];
	server m_server;
	boost::thread * m_threadMain;
	bool m_bThreadExit;
	std::list<websocketpp::connection_hdl> m_listClientConnection;
};
 
//WebSocketServerOpreate.cpp
 
#include "stdafx.h"
#include "WebSocketServerOpreate.h"
 
#include "jsoncpp/include/json/json.h"
 
#ifdef _DEBUG
#pragma comment(lib,"jsoncpp/lib/debug/lib_json.lib")
#else
#pragma comment(lib,"jsoncpp/lib/release/lib_json.lib")
#endif
 
WebSocketServerOpreate::WebSocketServerOpreate()
{
	m_usPort = 9100;
	memset(m_szBaseUri, 0, sizeof(m_szBaseUri));
	m_bThreadExit = true;
	m_threadMain = NULL;
	m_listClientConnection.clear();
}
 
WebSocketServerOpreate::~WebSocketServerOpreate()
{
	if (m_threadMain != NULL)
	{
		delete m_threadMain;
		m_threadMain = NULL;
	}
}
 
bool WebSocketServerOpreate::validate(server * s, websocketpp::connection_hdl hdl) {
	//sleep(6);
	return true;
}
 
void WebSocketServerOpreate::on_http(server* s, websocketpp::connection_hdl hdl) {
	server::connection_ptr con = s->get_con_from_hdl(hdl);
 
	std::string res = con->get_request_body();
 
	std::stringstream ss;
	ss << "got HTTP request with " << res.size() << " bytes of body data.";
 
	con->set_body(ss.str());
	con->set_status(websocketpp::http::status_code::ok);
}
 
void WebSocketServerOpreate::on_fail(server* s, websocketpp::connection_hdl hdl) {
	server::connection_ptr con = s->get_con_from_hdl(hdl);
 
	std::cout << "Fail handler: " << con->get_ec() << " " << con->get_ec().message() << std::endl;
}
 
void WebSocketServerOpreate::on_open(server* s, websocketpp::connection_hdl hdl) {
	//申请websocket upgrade成功之后，调用open_handler函数，回调on_open。
	//在这里，可以获取http请求的地址、参数信息。
	std::cout << "open handler" << std::endl;
	InsertClientConnection(hdl);
	server::connection_ptr con = s->get_con_from_hdl(hdl);
	websocketpp::config::core::request_type requestClient = con->get_request();
	std::string strMethod = requestClient.get_method();		//请求方法
	std::string strUri = requestClient.get_uri();			//请求uri地址，可以解析参数
	std::string strRequestOperateCommand = "";				//操作类型
	std::vector<NameAndValue> listRequestOperateParameter;	//操作参数列表	
	GetReqeustCommandAndParmeter(strUri, strRequestOperateCommand, listRequestOperateParameter);
	std::cout << "command:" << strRequestOperateCommand << std::endl;
	if (strcmp(strRequestOperateCommand.c_str(), m_szBaseUri) == 0)
	{
		if (listRequestOperateParameter.size() >= 2)
		{
			//验证用户和密码，返回登录结果
			if (strcmp(listRequestOperateParameter[0].strValue.c_str(), "admin") == 0 && strcmp(listRequestOperateParameter[1].strValue.c_str(), "admin") == 0)
			{
				Json::Value root;
				root["operatetype"] = 2;					//操作类型，比如登录、登录应答、注销等
				root["ret"] = 0;
				root["sessionid"] = "1110-1111-9999-3333";
				std::string strLoginResponse = root.toStyledString();
				s->send(hdl, strLoginResponse.c_str(), websocketpp::frame::opcode::TEXT);
			}
			else
			{
				Json::Value root;
				root["operatetype"] = 2;					//操作类型，比如登录、登录应答、注销等
				root["ret"] = 1;
				root["sessionid"] = "";
				std::string strLoginResponse = root.toStyledString();
				s->send(hdl, strLoginResponse.c_str(), websocketpp::frame::opcode::TEXT);
			}
		}
		else
		{
			Json::Value root;
			root["operatetype"] = 2;					//操作类型，比如登录、登录应答、注销等
			root["ret"] = 2;
			root["sessionid"] = "";
			std::string strLoginResponse = root.toStyledString();
			s->send(hdl, strLoginResponse.c_str(), websocketpp::frame::opcode::TEXT);
		}
	}
	else
	{
		//error request
	}
}
 
void WebSocketServerOpreate::on_close(server *s, websocketpp::connection_hdl hdl) {
	std::cout << "Close handler" << std::endl;
	DeleteClientConnection(hdl);
}
 
// Define a callback to handle incoming messages
void WebSocketServerOpreate::on_message(server* s, websocketpp::connection_hdl hdl, message_ptr msg) {
	/*
	hdl.lock().get() 获得连接标识
	msg->get_payload() 是收到的消息内容
	msg->get_opcode() 是收到消息的类型 ，包含：文本TEXT,二进制BINARY等等
	*/
	std::cout << "on_message called with hdl: " << hdl.lock().get()
		<< " and message: " << msg->get_payload()
		<< std::endl;
 
	try {
		/*
		发送消息
		s->send(
		hdl, //连接
		msg->get_payload(), //消息
		msg->get_opcode());//消息类型
		*/
		s->send(hdl, msg->get_payload(), msg->get_opcode());
	}
	catch (websocketpp::exception const & e) {
		std::cout << "Echo failed because: "
			<< "(" << e.what() << ")" << std::endl;
	}
}
 
int WebSocketServerOpreate::Init(unsigned short usPort, char *pBaseUri)
{
	int nRet = 0;
	m_usPort = usPort;
	strcpy_s(m_szBaseUri, pBaseUri);
	try {
		// Set logging settings
		m_server.set_access_channels(websocketpp::log::alevel::all);
		m_server.set_error_channels(websocketpp::log::elevel::all);
		//m_server.clear_access_channels(websocketpp::log::alevel::frame_payload);
 
		// Register our message handler
		m_server.set_message_handler(bind(&WebSocketServerOpreate::on_message,this, &m_server, ::_1, ::_2));
		m_server.set_http_handler(bind(&WebSocketServerOpreate::on_http, this, &m_server, ::_1));
		m_server.set_fail_handler(bind(&WebSocketServerOpreate::on_fail, this, &m_server, ::_1));
		m_server.set_open_handler(bind(&WebSocketServerOpreate::on_open, this, &m_server, ::_1));
		m_server.set_close_handler(bind(&WebSocketServerOpreate::on_close, this, &m_server, ::_1));
		m_server.set_validate_handler(bind(&WebSocketServerOpreate::validate, this, &m_server, ::_1));
 
		// Initialize ASIO
		m_server.init_asio();
		m_server.set_reuse_addr(true);
 
		// Listen on port
		m_server.listen(m_usPort);
		// Start the server accept loop
		m_server.start_accept();
	}
	catch (websocketpp::exception const & e) {
		std::cout << e.what() << std::endl;
		nRet = -1;
	}
	catch (const std::exception & e) {
		std::cout << e.what() << std::endl;
		nRet = -2;
	}
	catch (...) {
		std::cout << "other exception" << std::endl;
		nRet = -3;
	}
	
	return nRet;
}
int WebSocketServerOpreate::Uninit()
{
	return 0;
}
int WebSocketServerOpreate::StartWork()
{
	m_bThreadExit = false;
	m_threadMain = new boost::thread(boost::bind(&WebSocketServerOpreate::ThreadProccess, this));
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
int WebSocketServerOpreate::StopWork()
{
	//stop
	m_bThreadExit = true;
	m_server.stop();
	return 0;
}
 
int WebSocketServerOpreate::ThreadProccess()
{
	while (true)
	{
		if (m_bThreadExit)
		{
			break;
		}
 
		m_server.poll_one();
		Sleep(100);
	}
	return 0;
}
 
void WebSocketServerOpreate::InsertClientConnection(websocketpp::connection_hdl hdl)
{
	m_listClientConnection.push_back(hdl);
}
void WebSocketServerOpreate::DeleteClientConnection(websocketpp::connection_hdl hdl)
{
	std::list<websocketpp::connection_hdl>::iterator iter, iterEnd;
	iter = m_listClientConnection.begin();
	iterEnd = m_listClientConnection.end();
	for (iter; iter != iterEnd; iter++)
	{
		server::connection_ptr conInput = m_server.get_con_from_hdl(hdl);
		server::connection_ptr conSrc = m_server.get_con_from_hdl(*iter);
		if (conInput == conInput)
		{
			m_listClientConnection.erase(iter);
			break;
		}
	}
}
 
// 字符串分割
int WebSocketServerOpreate::StringSplit(std::vector<std::string>& dst, const std::string& src, const std::string& separator)
{
	if (src.empty() || separator.empty())
		return 0;
 
	int nCount = 0;
	std::string temp;
	size_t pos = 0, offset = 0;
 
	// 分割第1~n-1个
	while ((pos = src.find_first_of(separator, offset)) != std::string::npos)
	{
		temp = src.substr(offset, pos - offset);
		if (temp.length() > 0) {
			dst.push_back(temp);
			nCount++;
		}
		offset = pos + 1;
	}
 
	// 分割第n个
	temp = src.substr(offset, src.length() - offset);
	if (temp.length() > 0) {
		dst.push_back(temp);
		nCount++;
	}
 
	return nCount;
}
//去前后空格
 std::string& WebSocketServerOpreate::StringTrim(std::string &str)
{
	 if (str.empty()) {
		 return str;
	 }
	 str.erase(0, str.find_first_not_of(" "));
	 str.erase(str.find_last_not_of(" ") + 1);
	 return str;
}
//获取请求命令与参数
bool WebSocketServerOpreate::GetReqeustCommandAndParmeter(std::string strUri, std::string & strRequestOperateCommand, std::vector<NameAndValue> & listRequestOperateParameter)
{
	bool bRet = false;
	std::vector<std::string> vecRequest;
	int nRetSplit = StringSplit(vecRequest, strUri, "?");
	if (nRetSplit > 0)
	{
		if (vecRequest.size() == 1)
		{
			strRequestOperateCommand = vecRequest[0];
		}
		else if (vecRequest.size() > 1)
		{
			strRequestOperateCommand = vecRequest[0];
			std::string strRequestParameter = vecRequest[1];
			std::vector<std::string> vecParams;
			nRetSplit = StringSplit(vecParams, strRequestParameter, "&");
			if (nRetSplit > 0)
			{
				std::vector<std::string>::iterator iter, iterEnd;
				iter = vecParams.begin();
				iterEnd = vecParams.end();
				for (iter; iter != iterEnd; iter++)
				{
					std::vector<std::string> vecNameOrValue;
					nRetSplit = StringSplit(vecNameOrValue, *iter, "=");
					if (nRetSplit > 0)
					{
						NameAndValue nvNameAndValue;
						nvNameAndValue.strName = vecNameOrValue[0];
						nvNameAndValue.strValue = "";
						if (vecNameOrValue.size() > 1)
						{
							nvNameAndValue.strValue = vecNameOrValue[1];
						}
						//insert
						listRequestOperateParameter.push_back(nvNameAndValue);
					}
				}
			}
		}
		else
		{
 
		}
	}
	return bRet;
}
 
//main.cpp
// WebSocketTest.cpp : 定义控制台应用程序的入口点。
//
 
#include "stdafx.h"
#include "WebSocketServerOpreate.h"
 
 
int main() {
	WebSocketServerOpreate serverWebSocketServerOpreate;
	serverWebSocketServerOpreate.Init(9100);
	serverWebSocketServerOpreate.StartWork();
 
	while (true)
	{
		Sleep(200);
	}
 
	serverWebSocketServerOpreate.StopWork();
	serverWebSocketServerOpreate.Uninit();
}
 
