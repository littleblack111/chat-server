#pragma once

#include "FileDescriptor.hpp"
#include "format.hpp"
#include <memory>
#include <string>
#include <thread>

class CSession {
  public:
	struct SRecvData {
		char	data[1024] = {0};
		ssize_t dataSize   = sizeof(data);
		bool	good	   = true;
	};
	CSession(Hyprutils::OS::CFileDescriptor m_sockfd);
	~CSession();
	std::string getName() const;
	bool		isValid() const;
	SRecvData	read();
	SRecvData	read(const std::string &msg);
	bool		write(const std::string &msg, eFormatType type) const;
	template <typename... Args>
	bool write(eFormatType type, std::format_string<Args...> fmt, Args &&...args) const;
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
	void						   recvManager();
	bool						   registerSession();
	void						   onConnect() const;
	void						   onDisconnect() const;
	void						   onErrno(eEventType) const;
	void						   onRecv(const SRecvData &data) const;
	void						   onSend(const std::string &msg) const;
	friend class CSessionManager;
};
