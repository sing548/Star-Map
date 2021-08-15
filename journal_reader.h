#include <vector>
#include <string>
#include <iostream>
#include <filesystem>
#include <fstream>

#include <stdio.h>

#include "External Libraries/rapidjson/document.h"
#include "External Libraries/rapidjson/writer.h"
#include "External Libraries/rapidjson/stringbuffer.h"

namespace fs = std::filesystem;

struct Coordinate {
	std::string name;
	glm::vec3 coords;
};

class JournalReader {
public:

	JournalReader() { }
	std::vector<Coordinate> mVisitedCoordinates;
	void readAllJounals(std::string path) 
	{
		for (const auto& entry : std::filesystem::directory_iterator(path))
		{
			std::string fileName = entry.path().string();

			fileName = ReplaceAll(fileName, path, "");
			fileName = ReplaceAll(fileName, "\\", "");

			if (fileName._Starts_with("Journal.") && hasEnding(fileName, ".log"))
				processFile(entry.path().string(), mVisitedCoordinates);
		}
	}
private:
	void processFile(std::string path, std::vector<Coordinate> visitedCoordinates)
	{
		fstream newFile;

		newFile.open(path, ios::in);

		if (newFile.is_open())
		{
			string line;

			while (getline(newFile, line))
			{
				const char* lineChars = line.c_str();

				rapidjson::Document doc;
				doc.Parse(lineChars);


				rapidjson::Value& v = doc["event"];

				string event(v.GetString());

				if (event == "FSDJump")
				{
					rapidjson::Value& posArr = doc["StarPos"];

					Coordinate c;

					c.name = doc["StarSystem"].GetString();
					c.coords.x = posArr[0].GetFloat() / 10;
					c.coords.y = posArr[1].GetFloat() / 10;
					c.coords.z = posArr[2].GetFloat() / 10;

					mVisitedCoordinates.push_back(c);

					cout << "System: " << c.name << ", x: " << c.coords.x << ", y: " << c.coords.y << ", z: " << c.coords.z << endl;
				}
			}
		}
	}
	std::string ReplaceAll(std::string str, const std::string& from, const std::string& to) {
		size_t start_pos = 0;
		while ((start_pos = str.find(from, start_pos)) != std::string::npos) {
			str.replace(start_pos, from.length(), to);
			start_pos += to.length(); // Handles case where 'to' is a substring of 'from'
		}
		return str;
	}
	bool hasEnding(std::string const& fullString, std::string const& ending)
	{
		if (fullString.length() >= ending.length())
		{
			return (0 == fullString.compare(fullString.length() - ending.length(), ending.length(), ending));
		}
		else
			return false;
	}
};

