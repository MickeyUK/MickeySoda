#pragma once
#include <nlohmann/json.hpp>
#include "matoya.h"
#include "../Helpers/Countries.h"

using namespace std;

/// <summary>
/// Handles Soda Arcade stuff
/// </summary>
class Arcade {
private:
	bool _dev = false;		// Whether or not to use the dev server
	bool _staging = false;	// Whether or not to use the staging server
	int _postID = -1;		// The ID of the post associated with the room
	bool _secure = false;	// Whether or not to use HTTPS
	uint16_t _status = 0;	// The HTTP status code of the last request

	string _devDomain = "soda-arcade.test"; // The domain of the dev server
	string _stagingDomain = "staging.soda-arcade.com"; // The domain of the staging server
	string _prodDomain = "soda-arcade.com"; // The domain of the production server

	string _domain; // The domain of the server

	string createHeaders(size_t size);
public:
	class Artwork {
	public:
		int id = 0;
		string title = "";
	};

	static Arcade instance;
	vector<Artwork> artwork = vector<Artwork>();
	Countries countries = Countries();
	int artworkID = -1;	// The ID of the artwork associated with the room

	Arcade();
	
	bool login(string email, string password);
	bool checkToken(string token);

	bool createPost();
	bool deletePost();

	bool getArtwork();
};