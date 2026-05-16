#ifndef FILEHANDLER_HPP
# define FILEHANDLER_HPP

# include <string>
# include <vector>
# include <map>

struct FileInfo {
	std::string filename;
	std::string content;
	std::string senderNick;
};

class FileHandler {
	public:
		FileHandler();
		~FileHandler();

		// Operaciones principales
		void                saveFile(const std::string& target, const std::string& name, 
									 const std::string& content, const std::string& sender);
		FileInfo*           getFile(const std::string& target, const std::string& filename);
		void                removeFile(const std::string& target, const std::string& filename);
		
		// Consultas
		std::vector<FileInfo> getPendingList(const std::string& target);
		void                clearInbox(const std::string& nick);

	private:
		// Mapa de: Nickname del receptor -> Lista de archivos pendientes
		std::map<std::string, std::vector<FileInfo> > _inboxes;
};

#endif