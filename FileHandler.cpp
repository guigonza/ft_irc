#include "FileHandler.hpp"

FileHandler::FileHandler() {}
FileHandler::~FileHandler() {}

void FileHandler::saveFile(const std::string& target, const std::string& name, 
						   const std::string& content, const std::string& sender) {
	FileInfo fi;
	fi.filename = name;
	fi.content = content;
	fi.senderNick = sender;
	_inboxes[target].push_back(fi);
}

std::vector<FileInfo> FileHandler::getPendingList(const std::string& target) {
	if (_inboxes.count(target))
		return _inboxes[target];
	return std::vector<FileInfo>();
}

FileInfo* FileHandler::getFile(const std::string& target, const std::string& filename) {
	if (_inboxes.count(target)) {
		std::vector<FileInfo>& box = _inboxes[target];
		for (size_t i = 0; i < box.size(); ++i) {
			if (box[i].filename == filename)
				return &box[i];
		}
	}
	return NULL;
}

void FileHandler::removeFile(const std::string& target, const std::string& filename) {
	if (_inboxes.count(target)) {
		std::vector<FileInfo>& box = _inboxes[target];
		for (std::vector<FileInfo>::iterator it = box.begin(); it != box.end(); ++it) {
			if (it->filename == filename) {
				box.erase(it);
				break;
			}
		}
	}
}

void FileHandler::clearInbox(const std::string& nick) {
	_inboxes.erase(nick);
}