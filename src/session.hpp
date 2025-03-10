#pragma once

#include "FileDescriptor.hpp"
#include "format.hpp"
#include <memory>
#include <string>
#include <thread>

class CSession {
  public:
	CSession(Hyprutils::OS::CFileDescriptor m_sockfd);
	~CSession();

	struct SRecvData {
		char	data[1024] = {0};
		ssize_t dataSize   = sizeof(data);
		bool	good	   = true;
		bool	charValid  = true;
	};

	std::string getName();
	bool		isValid();
	SRecvData	read();
	SRecvData	read(const std::string &msg);
	bool		write(const std::string &msg, eFormatType type);
	template <typename... Args>
	bool write(eFormatType type, std::format_string<Args...> fmt, Args &&...args);
	void run();
	void setSelf(std::pair<std::jthread, std::shared_ptr<CSession>> *self);

  private:
	std::pair<std::jthread, std::shared_ptr<CSession>> *self;
	enum eEventType {
		READ,
		WRITE
	};
	Hyprutils::OS::CFileDescriptor m_sockfd;
	std::string					   m_name;
	void						   recvLoop();
	void						   registerSession();
	void						   onConnect();
	void						   onDisconnect();
	void						   onErrno(eEventType);
	void						   onRecv(const SRecvData &data);
	void						   onSend(const std::string &msg);
	friend class CSessionManager;
};
