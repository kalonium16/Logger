#include <vector>
#include <thread>
#include <fstream>
#include <string>
#include <map>
#include <latch>

#include "logger.h"
#include <gtest/gtest.h>

TEST(LoggerTests, LoggerCreate) {
	{
		std::ofstream ofs;
		ofs.open("log.txt", std::ofstream::out | std::ofstream::trunc);
		ofs.close();
		std::string filename{ "log.txt" };
		Logger logger;
		logger.log("Hi!");
	}
	std::string out{};
	std::ifstream logFile("log.txt");
	logFile >> out;
  EXPECT_EQ(out, "Hi!");
}

TEST(LoggerTests, LoggerLogOneTime) {
	{
		std::ofstream ofs;
		ofs.open("log.txt", std::ofstream::out | std::ofstream::trunc);
		ofs.close();
		std::string filename{ "log.txt" };
		Logger logger;
		logger.log("Hi World!!!");
	}
	std::string out{};
	std::ifstream logFile("log.txt");
	std::getline(logFile, out);
	logFile >> out;
	EXPECT_EQ(out, "Hi World!!!");
}

TEST(LoggerTests, LoggerLogSeveralTimes) {
	{
		std::ofstream ofs;
		ofs.open("log.txt", std::ofstream::out | std::ofstream::trunc);
		ofs.close();
		std::string filename{ "log.txt" };
		Logger logger;
		logger.log("1Hi World!!!");
		logger.log("2Hi World!!!");
		logger.log("3Hi World!!!");
		logger.log("4Hi World!!!");
	}
	std::vector<std::string> vOut{};
	std::ifstream logFile("log.txt");
	while (logFile.peek() != EOF) {
		std::string out{};
		std::getline(logFile, out);
		vOut.push_back(std::move(out));
	}
	EXPECT_EQ(vOut.size(), 4);
}

TEST(LoggerTests, LoggerLogMultithreading) {
	{
		// using  latch to start writing to file in the same time
		std::latch start{ 10 };
		auto  logMessages = [&](int id, Logger& logger)
			{
				start.arrive_and_wait();
				for (int i{ 0 }; i < 10; ++i) {
					std::string in{ "Log entry " + std::to_string(i) + " from thread " + std::to_string(id) };
					logger.log(in);
				}
			};
		std::ofstream ofs;
		ofs.open("log.txt", std::ofstream::out | std::ofstream::trunc);
		ofs.close();
		std::string filename{ "log.txt" };
		Logger logger{};
		std::vector<std::thread> threads;
		for (int i{ 0 }; i < 10; ++i) {
			threads.emplace_back(logMessages,i,std::ref(logger));
		}

		for (auto& t : threads) {
			t.join();
		}
	}
	std::vector<std::string> vOut{};
	std::ifstream logFile("log.txt");
	while (logFile.peek() != EOF) {
		std::string out{};
		std::getline(logFile, out);
		vOut.push_back(std::move(out));
	}
	EXPECT_EQ(vOut.size(), 100);
}

TEST(LoggerTests, LoggerMultithreadingEntriesCheck) {
	{
		// using  latch to start writing to file in the same time
		std::latch start{ 10 };
		auto  logMessages = [&](int id, Logger& logger)
			{
				start.arrive_and_wait();
				std::string in{ "Log entry from thread " + std::to_string(id) };
				logger.log(in);
			};
		std::ofstream ofs;
		ofs.open("log.txt", std::ofstream::out | std::ofstream::trunc);
		ofs.close();
		std::string filename{ "log.txt" };
		Logger logger{};
		std::vector<std::thread> threads;
		for (int i{ 0 }; i < 10; ++i) {
			threads.emplace_back(logMessages, i, std::ref(logger));
		}

		for (auto& t : threads) {
			t.join();
		}
	}
	std::map<std::string,bool> vOut{};
	std::ifstream logFile("log.txt");
	while (logFile.peek() != EOF) {
		std::string out{};
		std::getline(logFile, out);
		vOut[std::move(out)] = true;;
	}
	for (int i{ 0 }; i < 10; ++i) {
		std::string in{ "Log entry from thread " + std::to_string(i) };
		EXPECT_EQ(vOut.contains(in), true);
	}
}