#pragma once

#include "FileDescriptor.hpp"
#include "chatManager.hpp"
#include "format.hpp"
#include <atomic>
#include <memory>
#include <mutex>
#include <netinet/in.h>
#include <optional>
#include <string>
#include <thread>

class CSession {
  public:
	CSession();
	~CSession();

	struct SRecvData {
		std::string	 data;
		const size_t size = 1024;
		bool		 good = false;

		bool isEmpty() const;
		void sanitize();

		static constexpr char asciiEscape = 0x1B; // 
	};

	const std::string &getName() const;
	const std::string &getIp() const;
	bool			   isAdmin() const;
	void			   setSelf(std::pair<std::jthread, std::shared_ptr<CSession>> *self);

	std::unique_ptr<SRecvData> read();
	std::unique_ptr<SRecvData> read(const std::string &msg, bool bypassDeaf = false);
	bool					   write(const std::string &msg);
	template <typename... Args>
	bool write(std::format_string<Args...> fmt, Args &&...args);
	template <typename... Args>
	bool write(eFormatType type, std::format_string<Args...> fmt, Args &&...args);
	bool writeChat(const CChatManager::SMessage &msg);

	bool isMuted() const;
	void setMuted(bool mute);
	bool isDeaf() const;
	void setDeaf(bool deaf);

	void run();

	bool isValid();

  private:
	constexpr static char m_adminIps[1][INET_ADDRSTRLEN] = {"127.0.0.1"};

	enum eEventType : std::uint8_t {
		READ,
		WRITE
	};

	// no need atomic since they don't change after first set
	std::pair<std::jthread, std::shared_ptr<CSession>> *self;
	std::shared_ptr<CFileDescriptor>					m_sockfd;
	sockaddr_in											m_addr;
	socklen_t											m_addrLen = sizeof(m_addr);
	std::string											m_name; // need thread safety if we impl rename later
	std::string											m_ip;
	int													m_port;
	bool												m_isAdmin = false;

	std::optional<std::string> m_szReading = std::nullopt;
	std::atomic<bool>		   m_bMuted	   = false;
	std::atomic<bool>		   m_bDeaf	   = true; // init as true as we don't want anything during registerSession

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

	mutable std::mutex m_writeMutex;
	mutable std::mutex m_readMutex;
	mutable std::mutex m_readingMutex;
	mutable std::mutex m_mutex;

	friend class CSessionManager;
};
