#pragma once

#include "FileDescriptor.hpp"
#include "format.hpp"
#include <memory>
#include <netinet/in.h>
#include <string>
#include <thread>

class CSession {
  public:
	CSession();
	~CSession();

	struct SRecvData {
		char	data[1024] = {0};
		ssize_t dataSize   = sizeof(data);
		bool	good	   = true;
	};

	std::string getName();
	void		setSelf(std::pair<std::jthread, std::shared_ptr<CSession>> *self);

	SRecvData read();
	SRecvData read(const std::string &msg);
	bool	  write(const std::string &msg, eFormatType type);
	template <typename... Args>
	bool write(eFormatType type, std::format_string<Args...> fmt, Args &&...args);

	void run();

	bool isValid();

  private:
	enum eEventType : std::uint8_t {
		READ,
		WRITE
	};

	std::pair<std::jthread, std::shared_ptr<CSession>> *self;
	Hyprutils::OS::CFileDescriptor						m_sockfd;
	sockaddr_in											m_addr;
	socklen_t											m_addrLen = sizeof(m_addr);
	std::string											m_name;
	char												m_ip[INET_ADDRSTRLEN];
	int													m_port;

  bool m_isReading = false;

	void recvLoop();
	bool registerSession();

	void onConnect();
	void onDisconnect();
	void onErrno(eEventType);
	void onRecv(const SRecvData &data);
	void onSend(const std::string &msg);
	void onShutdown();

	friend class CSessionManager;
};
