#include "process.h"

#include <algorithm>
#include <cstdint>
#include <limits>
#include <stdexcept>
#include <thread>

#ifdef _WIN32
#include <windows.h>
#else
#include <cerrno>
#include <csignal>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
#endif

namespace scs::paper {

#ifdef _WIN32
	namespace {
		std::string QuoteWindowsArgument(const std::string& argument) {
			if (argument.find_first_of(" \t\"") == std::string::npos) {
				return argument;
			}
			std::string quoted = "\"";
			size_t backslashes = 0;
			for (const char c : argument) {
				if (c == '\\') {
					++backslashes;
				} else if (c == '"') {
					quoted.append(backslashes * 2 + 1, '\\');
					quoted.push_back('"');
					backslashes = 0;
				} else {
					quoted.append(backslashes, '\\');
					backslashes = 0;
					quoted.push_back(c);
				}
			}
			quoted.append(backslashes * 2, '\\');
			quoted.push_back('"');
			return quoted;
		}
	}

	ChildResult RunChild(const std::filesystem::path& executable,
		const std::vector<std::string>& arguments, std::chrono::milliseconds timeout,
		std::chrono::milliseconds grace) {
		const auto started = std::chrono::steady_clock::now();
		std::string command = QuoteWindowsArgument(executable.string());
		for (const auto& argument : arguments) {
			command += " " + QuoteWindowsArgument(argument);
		}
		std::vector<char> mutable_command(command.begin(), command.end());
		mutable_command.push_back('\0');

		STARTUPINFOA startup{};
		startup.cb = sizeof(startup);
		PROCESS_INFORMATION process{};
		if (!CreateProcessA(nullptr, mutable_command.data(), nullptr, nullptr, FALSE, 0,
			nullptr, nullptr, &startup, &process)) {
			throw std::runtime_error("Unable to start paper A* worker");
		}

		ChildResult result;
		const auto wait_ms = static_cast<DWORD>(std::min<int64_t>(
			(timeout + grace).count(), std::numeric_limits<DWORD>::max()));
		const DWORD wait_result = WaitForSingleObject(process.hProcess, wait_ms);
		if (wait_result == WAIT_TIMEOUT) {
			result.timed_out = true;
			result.killed = true;
			TerminateProcess(process.hProcess, 124);
			WaitForSingleObject(process.hProcess, 5000);
		}
		DWORD exit_code = 1;
		GetExitCodeProcess(process.hProcess, &exit_code);
		result.exit_code = static_cast<int>(exit_code);
		CloseHandle(process.hThread);
		CloseHandle(process.hProcess);
		result.wall_seconds = std::chrono::duration<double>(
			std::chrono::steady_clock::now() - started).count();
		return result;
	}
#else
	ChildResult RunChild(const std::filesystem::path& executable,
		const std::vector<std::string>& arguments, std::chrono::milliseconds timeout,
		std::chrono::milliseconds grace) {
		const pid_t pid = fork();
		if (pid < 0) {
			throw std::runtime_error("Unable to fork paper A* worker");
		}
		if (pid == 0) {
			std::vector<std::string> storage;
			storage.reserve(arguments.size() + 1);
			storage.emplace_back(executable.string());
			storage.insert(storage.end(), arguments.begin(), arguments.end());
			std::vector<char*> argv;
			argv.reserve(storage.size() + 1);
			for (auto& value : storage) {
				argv.push_back(value.data());
			}
			argv.push_back(nullptr);
			execv(executable.c_str(), argv.data());
			_exit(127);
		}

		ChildResult result;
		const auto started = std::chrono::steady_clock::now();
		int status = 0;
		while (true) {
			const pid_t waited = waitpid(pid, &status, WNOHANG);
			if (waited == pid) {
				break;
			}
			if (waited < 0 && errno != EINTR) {
				kill(pid, SIGKILL);
				waitpid(pid, &status, 0);
				throw std::runtime_error("Unable to wait for paper A* worker");
			}
			if (std::chrono::steady_clock::now() - started >= timeout + grace) {
				result.timed_out = true;
				kill(pid, SIGTERM);
				const auto terminate_started = std::chrono::steady_clock::now();
				while (std::chrono::steady_clock::now() - terminate_started < grace) {
					if (waitpid(pid, &status, WNOHANG) == pid) {
						result.killed = true;
						goto child_finished;
					}
					std::this_thread::sleep_for(std::chrono::milliseconds(50));
				}
				kill(pid, SIGKILL);
				waitpid(pid, &status, 0);
				result.killed = true;
				break;
			}
			std::this_thread::sleep_for(std::chrono::milliseconds(100));
		}

	child_finished:
		if (WIFEXITED(status)) {
			result.exit_code = WEXITSTATUS(status);
		} else if (WIFSIGNALED(status)) {
			result.exit_code = 128 + WTERMSIG(status);
		}
		result.wall_seconds = std::chrono::duration<double>(
			std::chrono::steady_clock::now() - started).count();
		return result;
	}
#endif

}
