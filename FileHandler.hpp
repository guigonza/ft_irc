#ifndef FILEHANDLER_HPP
# define FILEHANDLER_HPP

# include <string>

//CAMBIOS - Clase de utilidad estatica para parsear mensajes DCC (CTCP)
// No instanciable - solo metodos estaticos
class FileHandler {
	public:
		//CAMBIOS - Comprueba si un trailing de PRIVMSG es CTCP con DCC
		static bool isDCCMessage(const std::string& trailing);

		//CAMBIOS - Parsea DCC SEND extrayendo filename, ip entera, puerto y tamanio
		static bool parseDCCSend(const std::string& trailing, std::string& outFilename,
								 unsigned long& outIp, unsigned short& outPort,
								 unsigned long& outSize);

	private:
		FileHandler(); // No instanciable
};

#endif
