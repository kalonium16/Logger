#pragma once
#include <queue>
#include <string>
#include <fstream>
#include <mutex>
#include <thread>
#include <iostream>
#include <filesystem>



class Logger
{
public:
	Logger(std::string path = "log.txt") : m_logFilePath(path)
	{
		m_thread = std::thread{ &Logger::processEntries,this };
	};

	~Logger() {
		{
			std::scoped_lock lock{ m_mutex };
			m_exist = true;
		}
		m_condVar.notify_all();
		m_thread.join();

	};

	Logger(const Logger&) = delete;
	Logger& operator=(const Logger&) = delete;

	void log(std::string entry) {
		std::scoped_lock lock{ m_mutex };
		m_queue.push(std::move(entry));
		m_condVar.notify_all();
	};

private:

	void processEntries()
	{
		std::filesystem::path filePath{ m_logFilePath };
		std::ofstream logFile(filePath.c_str(), std::ios::app);
		if (logFile.fail())
		{
			std::cerr << "Failed to open logfile." << std::endl;
			return;
		}
		std::unique_lock lock{ m_mutex,std::defer_lock };
		while (true)
		{
			lock.lock();
			if (!m_exist) {
				m_condVar.wait(lock);
			}
			else {
				processEntriesHelper(m_queue, logFile);
				break;
			}
			std::queue<std::string> localQueue;
			localQueue.swap(m_queue);
			lock.unlock();
			processEntriesHelper(localQueue, logFile);
		}
		lock.unlock();
	};

	void processEntriesHelper(std::queue<std::string>& queue, std::ofstream& ofs) const
	{
		while (!queue.empty())
		{
			ofs << queue.front() << std::endl;
			queue.pop();
		}
	};


	std::mutex m_mutex;
	std::condition_variable m_condVar;
	std::queue<std::string> m_queue;

	std::thread m_thread;
	bool m_exist{false};
	std::string m_logFilePath{ "log.txt" };

};
