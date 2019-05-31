#include <websocketpp/config/asio_no_tls.hpp>

#include <websocketpp/server.hpp>

#include <iostream>
using namespace std;

typedef websocketpp::server<websocketpp::config::asio> server;
typedef server::message_ptr message_ptr;

using websocketpp::connection_hdl;
using websocketpp::lib::placeholders::_1;
using websocketpp::lib::placeholders::_2;
using websocketpp::lib::bind;

class Websocket
{
public:
	
	inline int getPort()
	{
		return port;
	}
	
	void setPort(int port)
	{
		assert((port>1024&&port<65535));
		this->port = port;
	}

	inline server& getWebsocketServer()
	{
		return m_server;
	}
	

	Websocket()
	{
		// Set logging settings
		m_server.set_access_channels(websocketpp::log::alevel::all);
		m_server.clear_access_channels(websocketpp::log::alevel::frame_payload);

		// Initialize ASIO
		m_server.init_asio();

		// Register our open handler
		m_server.set_open_handler(bind(&Websocket::OnOpen, this, ::_1));

		// Register our close handler
		m_server.set_close_handler(bind(&Websocket::OnClose, this, ::_1));

		// Register our message handler
		m_server.set_message_handler(bind(&Websocket::OnMessage, this, ::_1, ::_2));
	}



	void run(uint16_t port) 
	{
		setPort(port);
		// listen on specified port
		m_server.listen(port);

		// Start the server accept loop
		m_server.start_accept();

		// Start the ASIO io_service run loop
		try {
			m_server.run();
		} catch (const std::exception & e) {
			std::cout << e.what() << std::endl;
		}
	}
	
	void OnOpen(connection_hdl hdl)
	{

		server::connection_ptr con = this->m_server.get_con_from_hdl(hdl);
		cout << "have client connected" <<con<< endl;
	}

	void OnClose(connection_hdl hdl)
	{
		cout << "have client disconnected" << endl;
	}

	void OnMessage(connection_hdl hdl, message_ptr msg)
	{
		
		string strMsg = msg->get_payload();
		cout << strMsg << endl;

		string strRespon = "receive: ";
		strRespon.append(strMsg);
		m_server.send(hdl, strRespon, websocketpp::frame::opcode::text);
		
	}

private:
	int port;
	server    m_server;
};

int main()
{
	Websocket ws;
	ws.run(10086);

}