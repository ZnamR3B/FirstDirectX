#pragma once
#include "ErrorException.h"
#include <sstream>

ErrorException::ErrorException(int line, const char* file) noexcept
	:
	line(line),
	file(file)
{}

const char* ErrorException::what() const noexcept{
	std::ostringstream output;
	output << GetLine() << std::endl << GetOriginString();
	whatBuffer = output.str();
	return whatBuffer.c_str();
}

const char* ErrorException::GetType() const noexcept{
	return "Error Exception";
}

int ErrorException::GetLine() const noexcept{
	return line;
}

const std::string& ErrorException::GetFile() const noexcept {
	return file;
}

std::string ErrorException::GetOriginString() const noexcept {
	std::ostringstream output;
	output << "[File] " << file << std::endl
		<< "[Line]" << line;
	return output.str();
}