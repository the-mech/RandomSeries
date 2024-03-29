
#define WIN32_LEAN_AND_MEAN
#include <Windows.h>
#include <iostream>
#include <string>
#include <vector>
#include <random>

//given a base dir, this function extracts all filenames with absolute path from the base_dir and all subdirectories
void extractFilenames(std::vector<std::wstring>& filename_container, std::wstring& base_dir) {

	// wchar arr for interop with winapi
	wchar_t full_filename[512];
	//wstrings are easy to handle
	std::wstring ff; 
	std::wstring curr_dir;
	//save current directory
	GetCurrentDirectory(512, full_filename);
	curr_dir = full_filename;

	//get first file in base_directory
	WIN32_FIND_DATA dat;
	HANDLE first_file = FindFirstFile(base_dir.c_str(), &dat);

	//something went wrong
	if (first_file == INVALID_HANDLE_VALUE)
	{
		return;
	}

	do
	{
		// skip . and .. dirs
		if (0 == wcscmp(dat.cFileName, L".") || 0 == wcscmp(dat.cFileName, L"..")) {
			continue;
		}


		//GetFullPathName returns GetCurrentDirectory+dat.cfilename
		GetFullPathName(dat.cFileName, 512, full_filename, NULL);

		// subdir
		if (dat.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) {

			ff = std::wstring(full_filename); //ff contains full directory 
			SetCurrentDirectory(ff.c_str());	// set dir manually to subdir
			ff += L"\\*";

			extractFilenames(filename_container, ff);
			SetCurrentDirectory(curr_dir.c_str()); //reset dir
		}
		else
		{
			//push full name
			filename_container.push_back(full_filename);
		}


	} while (FindNextFile(first_file, &dat) != 0); //as long as we have files

	FindClose(first_file); // close handle
}

int main()
{
	std::vector<std::wstring> files;
	std::wstring base_dir = L"*";
	//std::cout << dat.cFileName << std::endl;

	//get all filepaths
	extractFilenames(files, base_dir);


	//process filepaths
	//if a space is found in a path token, wrap token with quote signs
	//like this
	// \Visual Studio 2017\ -> \"Visual Studio 2017"\

	for (auto& it : files) {
		for (int i = 0; i < it.size(); i++) {
			if (it[i] == L' ') {
				//loop to left backslash
				int j = i;
				do {
					j--;
				} while (it[j] != L'\\');
				//insert "
				it.insert(++j, L"\"");

				//loop to right backslash *or end*
				j = i;
				do {
					j++;
				} while (it[j] != L'\\' && j != it.size());
				//insert "
				it.insert(j, L"\"");
				i = j;
			}
		}
	}

	// now we got all files clean
	// choose a random file to open

	std::random_device rd;
	std::mt19937 rng_engine(rd());

	std::uniform_int_distribution<> dist(0, files.size() - 1);

	unsigned rng_number = dist(rng_engine);

	std::wstring exec = files[rng_number];

	std::cout << "Your lucky number is " << rng_number << std::endl;
	std::wcout << "Executing " << exec << std::endl;

	//execute chosen one

	STARTUPINFOW si = { sizeof(si) };
	PROCESS_INFORMATION pi;
	//_wsystem(files[rng_number].c_str());


	//actually executing cmd.exe with args /C filepath
	// /C executes following command and exits

	std::wstring command(L" /C ");
	command += exec;

	CreateProcessW(L"C:\\Windows\\System32\\cmd.exe",&command[0], NULL, NULL, FALSE, 0, NULL, NULL, &si, &pi);
	std::cout << "Err:" << GetLastError() << std::endl; // <- just in case
	return 0;
}

