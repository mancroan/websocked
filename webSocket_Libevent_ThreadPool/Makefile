#编译器选择
#CC=arm-linux-gnueabihf-gcc	
CC=g++
#应用
app:  ev_httpd_server_process ev_client_process #HTTP-Websocket服务器（线程池Libevent库）与Websocket客户端（Libevent库）
#app:  ev_thread_server_process ev_client_process #WebSocket服务器（线程池Libevent库）与WebSocket客户端（Libevent库）
#app:  ev_server_process ev_client_process #WebSocket服务器（Libevent库）与WebSocket客户端（Libevent库）
#app: client_process server_process #WebSocket服务器（epoll）与WebSocket客户端
ev_httpd_server_process:ev_websocket.o ev_httpd.o evbase_threadpool.o Threadpool.o ev_httpd_test.o config.o
	$(CC)  -Wall -o $@ $^ -lpthread -lcrypto -lssl -levent
ev_thread_server_process:websocket_common.o ev_thread_server_main.o evbase_threadpool.o Threadpool.o config.o
	$(CC)  -Wall -o $@ $^ -lpthread -lcrypto -lssl -levent
ev_client_process:websocket_common.o ev_client_main.o config.o
	$(CC)  -Wall -o $@ $^ -lpthread -lcrypto -lssl -levent 
ev_server_process:websocket_common.o ev_server_main.o config.o
	$(CC)  -Wall -o $@ $^ -lpthread -lcrypto -lssl -levent
client_process:websocket_common.o client_main.o config.o
	$(CC) -Wall -o $@ $^ -lcrypto -lssl -levent -lpthread
server_process:websocket_common.o server_main.o config.o
	$(CC) -Wall -o $@ $^ -lcrypto -lssl -levent -lpthread


%.o:%.c include/config.h
	$(CC) $^ -c -Wall 
%.o:%.cpp include/cnfig.h
	$(CC) $^ -c -o $@ -Wall 

client_process_d:websocket_common_d.o client_main_d.o config_d.o
	$(CC) -Wall -o $@ $^ -g -lcrypto -lssl -levent -lpthread
server_process_d:websocket_common_d.o server_main_d.o config_d.o
	$(CC) -Wall -o $@ $^ -g -lcrypto -lssl -levent -lpthread
ev_client_process_d:websocket_common_d.o ev_client_main_d.o config_d.o
	$(CC) -Wall -o $@ $^ -lpthread -lcrypto -lssl -levent -g 
ev_server_process_d:websocket_common_d.o ev_server_main_d.o config_d.o
	$(CC) -Wall -o $@ $^ -lpthread -lcrypto -lssl -levent -g
ev_thread_server_process_d:websocket_common_d.o ev_thread_server_main_d.o evbase_threadpool_d.o Threadpool_d.o config_d.o 
	$(CC)  -Wall -o $@ $^ -lpthread -lcrypto -lssl -levent -g
ev_httpd_server_process_d:ev_websocket_d.o ev_httpd_d.o evbase_threadpool_d.o Threadpool_d.o ev_httpd_test_d.o config_d.o
	$(CC)  -Wall -o $@ $^ -lpthread -lcrypto -lssl -levent -g


%_d.o:%.c 
	$(CC) $^ -c -g -o $@ -Wall
%_d.o:%.cpp 
	$(CC) $^ -c -g -o $@ -Wall

.PHONY:clean
clean:
	@rm -rf include/config.h.gch client_process* server_process* ev_client_process* ev_server_process* ev_httpd_server_process* ev_thread_server_process* *.o 
#调试模式下编译
debug: ev_httpd_server_process_d   ev_client_process_d 	# HTTP-Websocket服务器（线程池Libevent库）与Websocket客户端（Libevent库）
#debug: ev_thread_server_process_d   ev_client_process_d #WebSocket服务器（线程池Libevent库）与WebSocket客户端（Libevent库）
#debug: ev_server_process_d   ev_client_process_d 	    #WebSocket服务器（Libevent库）与WebSocket客户端（Libevent库）
#debug: server_process  client_process_d 			#WebSocket服务器（epoll）与WebSocket客户端（epoll）
