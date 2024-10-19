#include <netinet/in.h>
#include <sys/socket.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <stdio.h>
#include <unistd.h>
#include "network.hpp"

namespace network {
	port_forwarder::port_forwarder() { }

	bool port_forwarder::forward_with_pcp(bool fl_ipv6, std::string fl_address) {
		return false;
	}

	bool port_forwarder::is_external_ip(std::string str) {
		return false;
	}

	void port_forwarder::start_forwarding() {

	}

	port_forwarder::~port_forwarder() {

	}

	std::string get_last_error_msg() {
		return std::string("Dummy");
	}

	void socket_shutdown(socket_t socket_fd) {
		if(socket_fd > 0) {
			shutdown(socket_fd, SHUT_RDWR);
			close(socket_fd);
		}
	}

	int internal_socket_recv(socket_t socket_fd, void* data, size_t n) {
		return recv(socket_fd, data, n, MSG_DONTWAIT);
	}

	int internal_socket_send(socket_t socket_fd, const void* data, size_t n) {
		return send(socket_fd, data, n, MSG_NOSIGNAL);
	}

	template<typename F>
	int socket_recv(socket_t socket_fd, void* data, size_t len, size_t* m, F&& func) {
		while(*m < len) {
			int r = internal_socket_recv(socket_fd, reinterpret_cast<uint8_t*>(data) + *m, len - *m);
			if(r > 0) {
				*m += static_cast<size_t>(r);
			} else if(r < 0) { // error
				return r;
			} else if(r == 0) {
				break;
			}
		}
		// Did we receive a command?
		if(*m >= len) {
			assert(*m <= len);
			*m = 0; // reset
			func();
		}
		return 0;
	}

	int socket_send(socket_t socket_fd, std::vector<char>& buffer) {
		while(!buffer.empty()) {
			int r = internal_socket_send(socket_fd, buffer.data(), buffer.size());
			if(r > 0) {
				buffer.erase(buffer.begin(), buffer.begin() + static_cast<size_t>(r));
			} else if(r < 0) {
				return r;
			} else if(r == 0) {
				break;
			}
		}
		return 0;
	}

	socket_t socket_init_server(bool as_v6, struct sockaddr_storage& server_address) {
		socket_t socket_fd = static_cast<socket_t>(socket(as_v6 ? AF_INET6 : AF_INET, SOCK_STREAM, IPPROTO_TCP));
		if(socket_fd < 0) {
			std::abort();
		}
		struct timeval timeout;
		timeout.tv_sec = 60;
		timeout.tv_usec = 0;
		int opt = 1;
		if(setsockopt(socket_fd, SOL_SOCKET, SO_REUSEADDR, (char*)&opt, sizeof(opt))) {
			window::emit_error_message("Network setsockopt [reuseaddr] error: " + get_last_error_msg(), true);
		}
		if(setsockopt(socket_fd, SOL_SOCKET, SO_RCVTIMEO, (char*)&timeout, sizeof(timeout)) < 0) {
			window::emit_error_message("Network setsockopt [rcvtimeo] error: " + get_last_error_msg(), true);
		}
		if(setsockopt(socket_fd, SOL_SOCKET, SO_SNDTIMEO, (char*)&timeout, sizeof(timeout)) < 0) {
			window::emit_error_message("Network setsockopt [sndtimeo] error: " + get_last_error_msg(), true);
		}
		if(as_v6) {
			struct sockaddr_in6 v6_server_address;
			v6_server_address.sin6_addr = IN6ADDR_ANY_INIT;
			v6_server_address.sin6_family = AF_INET6;
			v6_server_address.sin6_port = htons(default_server_port);
			std::memcpy(&server_address, &v6_server_address, sizeof(v6_server_address));
		} else {
			struct sockaddr_in v4_server_address;
			v4_server_address.sin_addr.s_addr = INADDR_ANY;
			v4_server_address.sin_family = AF_INET;
			v4_server_address.sin_port = htons(default_server_port);
			std::memcpy(&server_address, &v4_server_address, sizeof(v4_server_address));
		}
		if(bind(socket_fd, (struct sockaddr*)&server_address, as_v6 ? sizeof(sockaddr_in6) : sizeof(sockaddr_in)) < 0) {
			window::emit_error_message("Network bind error: " + get_last_error_msg(), true);
		}
		if(listen(socket_fd, SOMAXCONN) < 0) {
			window::emit_error_message("Network listen error: " + get_last_error_msg(), true);
		}
		return socket_fd;
	}

	socket_t socket_init_client(bool& as_v6, struct sockaddr_storage& client_address, const char* ip_address) {
		struct addrinfo hints;
		std::memset(&hints, 0, sizeof(hints));
		hints.ai_family = AF_UNSPEC;
		hints.ai_socktype = SOCK_STREAM;
		hints.ai_protocol = IPPROTO_TCP;
		struct addrinfo* result = NULL;
		if(getaddrinfo(ip_address, "1984", &hints, &result) != 0) {
			window::emit_error_message("Network getaddrinfo error: " + get_last_error_msg(), true);
		}
		as_v6 = false;
		bool found = false;
		for(struct addrinfo* ptr = result; ptr != NULL; ptr = ptr->ai_next) {
			if(ptr->ai_socktype == SOCK_STREAM && ptr->ai_protocol == IPPROTO_TCP) {
				if(ptr->ai_family == AF_INET || ptr->ai_family == AF_INET6) {
					as_v6 = ptr->ai_family == AF_INET6;
					std::memcpy(&client_address, ptr->ai_addr, sizeof(sockaddr));
					found = true;
				}
			}
		}
		freeaddrinfo(result);
		if(!found) {
			window::emit_error_message("No suitable host found", true);
		}
		socket_t socket_fd = static_cast<socket_t>(socket(as_v6 ? AF_INET6 : AF_INET, SOCK_STREAM, IPPROTO_TCP));
		if(socket_fd < 0) {
			window::emit_error_message("Network socket error: " + get_last_error_msg(), true);
		}
		if(connect(socket_fd, (const struct sockaddr*)&client_address, as_v6 ? sizeof(sockaddr_in6) : sizeof(sockaddr_in)) < 0) {
			window::emit_error_message("Network connect error: " + get_last_error_msg(), true);
		}
		return socket_fd;
	}
}
