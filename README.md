# Luminance
Lumianance was compiled with OpenCV-3.3.1, Release-x64

For compiling from source: 

	Add OpenCV libraries and Include directories in Luminance.sln
	Build Code in x64 - Release
	Threading by including `thread` 
	Filesystem access by including `filesystem` (corresponding namespace -> std::experimental::filesystem)

Executable and other information: 

	The pre-built .exe file is in `x64\Release\` and called Luminance.exe
	Folder `x64\Release\ref` contains provided videos for initial testing
	Program can be run from the Command Line in Windows as: 
		
		Luminance <folder_path> <threads_count_for_execution>