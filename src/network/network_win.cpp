#ifndef _MSC_VER
#include <unistd.h>
#endif
#define _WINSOCK_DEPRECATED_NO_WARNINGS 1
#ifndef WINSOCK2_IMPORTED
#define WINSOCK2_IMPORTED
#include <winsock2.h>
#include <ws2tcpip.h>
#endif
#include <windows.h>
#include <natupnp.h>
#include <iphlpapi.h>
#include <Mstcpip.h>
#include <ip2string.h>
#include "pcp.h"
#include "network.hpp"

namespace network {
	struct local_addresses {
		std::string address;
		bool ipv6 = false;
	};

	port_forwarder::port_forwarder() { }

	bool port_forwarder::forward_with_pcp(bool fl_ipv6, std::string fl_address) {
		bool mapped_ports_with_upnp = false;

		IUPnPNAT* nat_interface = nullptr;
		IStaticPortMappingCollection* port_mappings = nullptr;
		IStaticPortMapping* opened_port = nullptr;

		BSTR proto = SysAllocString(L"TCP");
		BSTR desc = SysAllocString(L"Katerina Engine Host");
		auto tmpwstr = text::utf8_to_native(fl_address);
		BSTR local_host = SysAllocString(tmpwstr.c_str());
		VARIANT_BOOL enabled = VARIANT_TRUE;
		if(SUCCEEDED(CoCreateInstance(__uuidof(UPnPNAT), NULL, CLSCTX_ALL, __uuidof(IUPnPNAT), (void**)&nat_interface)) && nat_interface) {
			if(SUCCEEDED(nat_interface->get_StaticPortMappingCollection(&port_mappings)) && port_mappings) {
				if(SUCCEEDED(port_mappings->Add(default_server_port, proto, default_server_port, local_host, enabled, desc, &opened_port)) && opened_port) {
					mapped_ports_with_upnp = true;
				}
			}
		}
		pcp_ctx_t* pcp_obj = nullptr;
		if(!mapped_ports_with_upnp) {
			pcp_obj = pcp_init(ENABLE_AUTODISCOVERY, NULL);
			sockaddr_storage source_ip;
			sockaddr_storage ext_ip;
			WORD wVersionRequested;
			WSADATA wsaData;
			wVersionRequested = MAKEWORD(2, 2);
			auto err = WSAStartup(wVersionRequested, &wsaData);
			memset(&source_ip, 0, sizeof(source_ip));
			memset(&ext_ip, 0, sizeof(ext_ip));
			if(fl_ipv6 == false) {
				((sockaddr_in*)(&source_ip))->sin_port = default_server_port;
				((sockaddr_in*)(&source_ip))->sin_addr.s_addr = inet_addr(fl_address.c_str());
				((sockaddr_in*)(&source_ip))->sin_family = AF_INET;
				((sockaddr_in*)(&ext_ip))->sin_port = default_server_port;
				((sockaddr_in*)(&ext_ip))->sin_family = AF_INET;
			} else {
				((sockaddr_in6*)(&source_ip))->sin6_port = default_server_port;
				PCSTR term = nullptr;
				RtlIpv6StringToAddressA(fl_address.c_str(), &term, &(((sockaddr_in6*)(&source_ip))->sin6_addr));
				((sockaddr_in6*)(&source_ip))->sin6_family = AF_INET6;
				((sockaddr_in6*)(&ext_ip))->sin6_port = default_server_port;
				((sockaddr_in6*)(&ext_ip))->sin6_family = AF_INET6;
			}
			auto flow = pcp_new_flow(pcp_obj, (sockaddr*)&source_ip, NULL, (sockaddr*)&ext_ip, IPPROTO_TCP, 3600, NULL);
			if(flow) {
				pcp_wait(flow, 10000, 0);
			}
			pcp_close_flow(flow);
		}
		// wait for destructor
		internal_wait.lock();
		if(pcp_obj) {
			pcp_terminate(pcp_obj, 1);
		}
		//cleanup forwarding
		if(port_mappings) {
			port_mappings->Remove(default_server_port, proto);
		}
		if(opened_port) {
			opened_port->Release();
		}
		if(port_mappings) {
			port_mappings->Release();
		}
		if(nat_interface) {
			nat_interface->Release();
		}
		SysFreeString(proto);
		SysFreeString(local_host);
		SysFreeString(desc);
		internal_wait.unlock();
		return mapped_ports_with_upnp;
	}

	bool port_forwarder::is_external_ip(std::string str) {
		// Detect and skip non-external addresses
		bool is_link_local = false;
		bool is_special_use = false;
		if(str.find("fe") == 0) {
			const char c = str[2];
			if(c == '8' || c == '9' || c == 'a' || c == 'b') {
				is_link_local = true;
			}
		} else if(str.find("2001:0:") == 0) {
			is_special_use = true;
		}
		return !(is_special_use || is_link_local);
	}

	void port_forwarder::start_forwarding() {
		if(started)
			return;

		internal_wait.lock();
		started = true;
		do_forwarding = std::thread([&]() {
			//setup forwarding
			if(!SUCCEEDED(CoInitializeEx(NULL, COINIT_APARTMENTTHREADED)))
				return;

			std::vector<local_addresses> found_locals;
			// try to figure out what the computer's local address is
			IP_ADAPTER_ADDRESSES* adapter_addresses = NULL;
			IP_ADAPTER_ADDRESSES* adapter = NULL;
			// Start with a 16 KB buffer and resize if needed -
			// multiple attempts in case interfaces change while
			// we are in the middle of querying them.
			DWORD adapter_addresses_buffer_size = 16 * 1024;
			for(uint32_t attempts = 0; attempts != 3; ++attempts) {
				adapter_addresses = (IP_ADAPTER_ADDRESSES*)malloc(adapter_addresses_buffer_size);
				assert(adapter_addresses);
				DWORD error = GetAdaptersAddresses(
					AF_UNSPEC,
					GAA_FLAG_SKIP_ANYCAST | GAA_FLAG_SKIP_MULTICAST | GAA_FLAG_SKIP_DNS_SERVER | GAA_FLAG_SKIP_FRIENDLY_NAME,
					NULL,
					adapter_addresses,
					&adapter_addresses_buffer_size);
				if(ERROR_SUCCESS == error) {
					// We're done here, people!
					break;
				} else if(ERROR_BUFFER_OVERFLOW == error) {
					// Try again with the new size
					free(adapter_addresses);
					adapter_addresses = NULL;
					continue;
				} else {
					// Unexpected error code - log and throw
					free(adapter_addresses);
					adapter_addresses = NULL;
					// @todo
				}
			}

			// Iterate through all of the adapters
			for(adapter = adapter_addresses; NULL != adapter; adapter = adapter->Next) {
				// Skip loopback adapters
				if(adapter->IfType == IF_TYPE_SOFTWARE_LOOPBACK) {
					continue;
				}

				// Parse all IPv4 and IPv6 addresses
				for(IP_ADAPTER_UNICAST_ADDRESS* address = adapter->FirstUnicastAddress; address != NULL; address = address->Next) {
					auto family = address->Address.lpSockaddr->sa_family;
					if(address->DadState != NldsPreferred && address->DadState != IpDadStatePreferred)
						continue;

					if(AF_INET == family) {
						// IPv4
						SOCKADDR_IN* ipv4 = reinterpret_cast<SOCKADDR_IN*>(address->Address.lpSockaddr);
						char str_buffer[INET_ADDRSTRLEN] = { 0 };
						inet_ntop(AF_INET, &(ipv4->sin_addr), str_buffer, INET_ADDRSTRLEN);
						std::string ip_str(str_buffer);
						reports::write_debug("Local IPv4 address: " + ip_str + "\n");
						found_locals.push_back(local_addresses{ ip_str,  false });
					} else if(AF_INET6 == family) {
						// IPv6
						SOCKADDR_IN6* ipv6 = reinterpret_cast<SOCKADDR_IN6*>(address->Address.lpSockaddr);
						char str_buffer[INET6_ADDRSTRLEN] = { 0 };
						inet_ntop(AF_INET6, &(ipv6->sin6_addr), str_buffer, INET6_ADDRSTRLEN);
						std::string ip_str(str_buffer);
						// Detect and skip non-external addresses
						if(is_external_ip(ip_str)) {
							reports::write_debug("Local IPv6 address: " + ip_str + "\n");
							found_locals.push_back(local_addresses{ ip_str, true });
						}
					} else {
						// Skip all other types of addresses
					}
				}
			}
			// Cleanup
			free(adapter_addresses);
			adapter_addresses = NULL;

			// try to add port mapping
			bool mapped_ports_with_upnp = false;
			if(found_locals.size() != 0) {
				mapped_ports_with_upnp = forward_with_pcp(found_locals[0].ipv6, found_locals[0].address);
			}
			CoUninitialize();
		});
	}

	port_forwarder::~port_forwarder() {
		if(started) {
			internal_wait.unlock();
			do_forwarding.join();
		}
	}

	std::string get_last_error_msg() {
		auto err = WSAGetLastError();
		LPTSTR err_buf = nullptr;
		FormatMessage(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM, nullptr, err, 0, (LPTSTR)&err_buf, 0, nullptr);
		native_string err_text = err_buf;
		LocalFree(err_buf);
		return text::native_to_utf8(err_text);
	}

	void socket_shutdown(socket_t socket_fd) {
		if(socket_fd > 0) {
			shutdown(socket_fd, SD_BOTH);
			closesocket(socket_fd);
		}
	}

	int internal_socket_recv(socket_t socket_fd, void* data, size_t n) {
		u_long has_pending = 0;
		auto r = ioctlsocket(socket_fd, FIONREAD, &has_pending);
		if(has_pending) {
			return int(recv(socket_fd, reinterpret_cast<char*>(data), static_cast<int>(n), 0));
		}
		return 0;
	}

	int internal_socket_send(socket_t socket_fd, const void* data, size_t n) {
		return int(send(socket_fd, reinterpret_cast<const char*>(data), static_cast<int>(n), 0));
	}

	template<typename F>
	int socket_recv(socket_t socket_fd, void* data, size_t len, size_t* m, F&& func) {
		while(*m < len) {
			int r = internal_socket_recv(socket_fd, reinterpret_cast<uint8_t*>(data) + *m, len - *m);
			if(r > 0) {
				*m += static_cast<size_t>(r);
			} else if(r < 0) { // error
				int err = WSAGetLastError();
				if(err == WSAENOBUFS || err == WSAEWOULDBLOCK) {
					return 0;
				}
				return err;
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
				int err = WSAGetLastError();
				if(err == WSAENOBUFS || err == WSAEWOULDBLOCK) {
					return 0;
				}
				return err;
			} else if(r == 0) {
				break;
			}
		}
		return 0;
	}

	socket_t socket_init_server(bool as_v6, struct sockaddr_storage& server_address) {
		WSADATA data;
		if(WSAStartup(MAKEWORD(2, 2), &data) != 0) {
			window::emit_error_message("WSA startup error: " + get_last_error_msg(), true);
		}
		socket_t socket_fd = static_cast<socket_t>(socket(as_v6 ? AF_INET6 : AF_INET, SOCK_STREAM, IPPROTO_TCP));
		if(socket_fd == static_cast<socket_t>(INVALID_SOCKET)) {
			window::emit_error_message("Network socket error: " + get_last_error_msg(), true);
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
			struct sockaddr_in6 v6_server_address{};
			v6_server_address.sin6_addr = IN6ADDR_ANY_INIT;
			v6_server_address.sin6_family = AF_INET6;
			v6_server_address.sin6_port = htons(default_server_port);
			std::memcpy(&server_address, &v6_server_address, sizeof(v6_server_address));
		} else {
			struct sockaddr_in v4_server_address{};
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
		u_long mode = 1; // 1 to enable non-blocking socket
		ioctlsocket(socket_fd, FIONBIO, &mode);
		return socket_fd;
	}

	socket_t socket_init_client(bool& as_v6, struct sockaddr_storage& client_address, const char* ip_address) {
		WSADATA data;
		if(WSAStartup(MAKEWORD(2, 2), &data) != 0) {
			window::emit_error_message("WSA startup error: " + get_last_error_msg(), true);
		}
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
		if(socket_fd == static_cast<socket_t>(INVALID_SOCKET)) {
			window::emit_error_message("Network socket error: " + get_last_error_msg(), true);
		}
		if(connect(socket_fd, (const struct sockaddr*)&client_address, as_v6 ? sizeof(sockaddr_in6) : sizeof(sockaddr_in)) < 0) {
			window::emit_error_message("Network connect error: " + get_last_error_msg(), true);
		}
		return socket_fd;
	}
}
