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

enum StarClass {
	O,
	B,
	A,
	F,
	G,
	K,
	L,
	M,
	T,
	Y,
	D,
	GENERIC
};

struct Coordinate {
	std::string name;
	StarClass starClass;
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

				

				if (event == "StartJump")
				{
					rapidjson::Value& t = doc["JumpType"];
					string jType(t.GetString());

					if (jType == "Hyperspace")
					{
						string sSystem = doc["StarSystem"].GetString();

						std::vector<Coordinate>::iterator i = std::find_if(mVisitedCoordinates.begin(), mVisitedCoordinates.end(), [&](const auto& val) { return val.name == sSystem; });

						if (i == mVisitedCoordinates.end())
						{
							Coordinate c;

							c.name = sSystem;
							c.starClass = EvaluateStarClass(doc["StarClass"].GetString());

							mVisitedCoordinates.push_back(c);
						}
					}
				}
				else if (event == "FSDJump")
				{
					rapidjson::Value& posArr = doc["StarPos"];
					string sSystem = doc["StarSystem"].GetString();


					std::vector<Coordinate>::iterator i = std::find_if(mVisitedCoordinates.begin(), mVisitedCoordinates.end(), [&](const auto& val) { return val.name == sSystem; });

					if (i != mVisitedCoordinates.end())
					{
						Coordinate& c = *i;

						c.coords.x = posArr[0].GetFloat() / 10;
						c.coords.y = posArr[1].GetFloat() / 10;
						c.coords.z = posArr[2].GetFloat() / 10;
						cout << "System: " << c.name << ", StarClass: " << c.starClass << ", x: " << c.coords.x << ", y: " << c.coords.y << ", z: " << c.coords.z << endl;
					}
					else
					{
						Coordinate c;
						c.name = sSystem;
						c.coords.x = posArr[0].GetFloat() / 10;
						c.coords.y = posArr[1].GetFloat() / 10;
						c.coords.z = posArr[2].GetFloat() / 10;
						mVisitedCoordinates.push_back(c);
						cout << "System: " << c.name << ", StarClass: Unknown, " << ", x: " << c.coords.x << ", y: " << c.coords.y << ", z: " << c.coords.z << endl;
					}
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

	StarClass EvaluateStarClass(std::string classString)
	{
		if (classString == "O")
		{
			return StarClass::O;
		}
		else if (classString == "B")
		{
			return StarClass::B;
		}
		else if (classString == "A")
		{
			return StarClass::A;
		}
		else if (classString == "F")
		{
			return StarClass::F;
		}
		else if (classString == "G")
		{
			return StarClass::G;
		}
		else if (classString == "K")
		{
			return StarClass::K;
		}
		else if (classString == "L")
		{
			return StarClass::L;
		}
		else if (classString == "M")
		{
			return StarClass::M;
		}
		else if (classString == "T")
		{
			return StarClass::T;
		}
		else if (classString == "TTS")
		{
			return StarClass::GENERIC;
		}
		else if (classString == "Y")
		{
			return StarClass::Y;
		}
		else if (classString == "D" || classString == "DA" || classString == "DAZ" || classString == "DC" || classString == "DQ" || classString == "DAB")
		{
			return StarClass::D;
		}
		else if (classString == "N")
		{
			return StarClass::GENERIC;
		}
		else if (classString == "SupermassiveBlackHole")
		{
			return StarClass::GENERIC;
		}
		else if (classString == "H")
		{
			return StarClass::GENERIC;
		}
		else
		{
			return StarClass::GENERIC;
		}
	}
};