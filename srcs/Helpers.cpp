#include "Webserv.hpp"

Context get_default_conf(int i) {
	struct Context default_conf;

	(void)i;
	// default_conf.ports.push_back( // Update with Listen
		// 1024 + i);	// Ports up to 1024 are privileged and require super user!
	default_conf.serverName.push_back("localhost");
	default_conf.index.push_back("index.html");
	default_conf.autoIndex = false;
	default_conf.clientMaxBodySize = 200000;
	default_conf.uploadDir = "None";
	default_conf.allowedMethods.push_back(GET);
	default_conf.allowedMethods.push_back(POST);
	default_conf.allowedMethods.push_back(DELETE);

	return default_conf;
}

std::string boolToString(bool value) {
	if (value)
		return "True";
	else
		return "False";
}

std::string methodToString(Method method) {
	switch (method) {
		case GET:
			return "GET";
		case POST:
			return "POST";
		case DELETE:
			return "DELETE";
		default:
			return "UNKNOWN";
	}
}

std::string int_to_string(int value) {
	std::stringstream ss;
	ss << value;
	return ss.str();
}

std::ostream& operator<<(std::ostream& outstream,
						 const struct Context configuration) {
	outstream << "Ports: ";
	for (std::vector<Listen>::const_iterator it = configuration.ports.begin();
		 it != configuration.ports.end(); it++)
		// outstream << *it << " "; // Update with Listen
	outstream << std::endl;
	outstream << "Server Name: " << configuration.serverName[0] << std::endl;
	outstream << "Index: " << configuration.index[0] << std::endl;
	outstream << "AutoIndex: " << boolToString(configuration.autoIndex)
			  << std::endl;
	outstream << "Client Max Body Size: " << configuration.clientMaxBodySize
			  << std::endl;
	outstream << "Upload Directory: " << configuration.uploadDir << std::endl;
	outstream << "Allowed methods: ";
	for (std::vector<Method>::const_iterator it =
			 configuration.allowedMethods.begin();
		 it != configuration.allowedMethods.end(); it++)
		outstream << methodToString(*it) << " ";
	outstream << std::endl;

	return outstream;
}