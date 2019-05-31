#include <websocketpp/config/asio_no_tls.hpp>

#include <websocketpp/server.hpp>

#include <iostream>
#include <string>

using namespace std;

typedef websocketpp::server<websocketpp::config::asio> server;

using websocketpp::lib::placeholders::_1;
using websocketpp::lib::placeholders::_2;
using websocketpp::lib::bind;

// pull out the type of messages sent by our config
typedef server::message_ptr message_ptr;

// Define a callback to handle incoming messages
void on_message(server* s, websocketpp::connection_hdl hdl, message_ptr msg) {
    std::cout << "on_message called with hdl: " << hdl.lock().get()
              << " and message: " << msg->get_payload()
			  << "msgtype:"<<msg->get_opcode()
              << std::endl;
	

    if (msg->get_payload() == "stop-listening") {
        s->stop_listening();
        return;
    }
	/*
    try {
        s->send(hdl, msg->get_payload(), msg->get_opcode());
    } catch (websocketpp::exception const & e) {
        std::cout << "Echo failed because: "
                  << "(" << e.what() << ")" << std::endl;
    }
	*/

	string strMsg = msg->get_payload();
	cout<<"msg length:"<< strMsg.length(); 

}

void OnOpen(server* s,websocketpp::connection_hdl hdl)
{
	server::connection_ptr con = s->get_con_from_hdl(hdl);
	std::cout << "have client connected" << con <<std::endl;
}

void OnClose(server* s, websocketpp::connection_hdl hdl)
{
	std::cout << "have client disconnected" << std::endl;
}

int main() {
    // Create a server endpoint
    server echo_server;

    try {
        // Set logging settings
        echo_server.set_access_channels(websocketpp::log::alevel::all);
        echo_server.clear_access_channels(websocketpp::log::alevel::frame_payload);

        // Initialize Asio
        echo_server.init_asio();
		echo_server.set_reuse_addr(true);

		// Register our open handler
		echo_server.set_open_handler(bind(&OnOpen, &echo_server, ::_1));

		// Register our close handler
		echo_server.set_close_handler(bind(&OnClose, &echo_server, ::_1));
        echo_server.set_message_handler(bind(&on_message,&echo_server,::_1,::_2));

        // Listen on port 9002
        echo_server.listen(10086);

        // Start the server accept loop
        echo_server.start_accept();

        // Start the ASIO io_service run loop
        echo_server.run();
    } catch (websocketpp::exception const & e) {
        std::cout << e.what() << std::endl;
    } catch (...) {
        std::cout << "other exception" << std::endl;
    }
}
