#pragma once

#include "FileDescriptor.hpp"
#include <string>

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
	SRecvData	read(std::string &msg);
	bool		write(const std::string &msg) const;
	void		run();

  private:
	Hyprutils::OS::CFileDescriptor m_sockfd;
	std::string					   m_name;
	void						   recvManager();
	bool						   registerSession();
	void						   onConnect() const;
	void						   onDisconnect() const;
	void						   onErrno() const;
	void						   onRecv(SRecvData &data) const;
	friend class CSessionManager;
};
