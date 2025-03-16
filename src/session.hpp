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
		std::string	 data;
		const size_t size = 1024;
		bool		 good = true;

		bool isEmpty() const;
		void sanitize();
	};

	const std::string &getName() const;
	void			   setSelf(std::pair<std::jthread, std::shared_ptr<CSession>> *self);

	std::unique_ptr<SRecvData> read();
	std::unique_ptr<SRecvData> read(const std::string &msg);
	bool					   write(const std::string &msg);
	template <typename... Args>
	bool write(std::format_string<Args...> fmt, Args &&...args);
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

#ifdef DEBUG
	void onConnect();
	void onDisconnect();
#endif
	void onErrno(eEventType);
#ifdef DEBUG
	void onRecv(const SRecvData &data);
	void onSend(const std::string &msg);
#endif
	void onKick(const std::string &reason = "", const bool &kill = false);

	friend class CSessionManager;
};
