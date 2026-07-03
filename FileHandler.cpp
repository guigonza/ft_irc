#include "FileHandler.hpp"
#include <sstream>
#include <cstdlib>
#include <iostream>

//CAMBIOS - Comprueba si un trailing de PRIVMSG es un mensaje CTCP con DCC
// Formato: \x01DCC <comando> ... \x01
bool FileHandler::isDCCMessage(const std::string& trailing) {
	return (trailing.size() >= 6
			&& static_cast<unsigned char>(trailing[0]) == 0x01
			&& trailing.compare(1, 4, "DCC ") == 0);
}

//CAMBIOS - Parsea un mensaje DCC SEND extrayendo los campos
// Formato: \x01DCC SEND <filename> <ip_int32> <port> <size>\x01
bool FileHandler::parseDCCSend(const std::string& trailing, std::string& outFilename,
							   unsigned long& outIp, unsigned short& outPort,
							   unsigned long& outSize) {
	if (!isDCCMessage(trailing))
		return false;
	if (trailing.size() < 11 || trailing.compare(5, 5, "SEND ") != 0)
		return false;

	// Datos despues de "DCC SEND "
	std::string data = trailing.substr(10);
	// Quitar \x01 final si existe
	if (!data.empty() && static_cast<unsigned char>(data[data.size() - 1]) == 0x01)
		data.erase(data.size() - 1);

	std::istringstream ss(data);
	std::string token;

	if (!(ss >> outFilename))
		return false;

	if (!(ss >> token))
		return false;
	outIp = std::strtoul(token.c_str(), NULL, 10);

	if (!(ss >> token))
		return false;
	outPort = static_cast<unsigned short>(std::strtoul(token.c_str(), NULL, 10));

	if (!(ss >> token))
		return false;
	outSize = std::strtoul(token.c_str(), NULL, 10);

	return true;
}
