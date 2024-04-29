#pragma once

#include <filesystem>
#include <fstream>


namespace util
{

bool displayFileInfo( const std::filesystem::path &targetPath );
void displayDirTree( const std::filesystem::path &targetPath, int level );
size_t getFileSize( const std::filesystem::path &pathToCheck );
bool displayTextFile( const std::filesystem::path &targetPath );
bool displayBinaryFile( const std::filesystem::path &targetPath );
void writeFile( const std::filesystem::path &path, const std::string &data );
void deleteFile( const std::string &filename );
void searchFile( const std::filesystem::path &directory, const std::filesystem::path &filename );
bool createDirectory( const std::string &str );
void setFilePermissions( const std::string &path, std::filesystem::perms permsToSet, std::filesystem::perm_options permsAction );
std::string getFilename( const std::string &path );
std::string getFileExtension( const std::string &filename );
//	\function	isFileBinary	||	\date	2020/10/30 2:31
//	\brief		read 255 chars just to be sure
bool isFileBinary( const char *fname );
//	\function	printFile	||	\date	2020/10/30 2:30
//	\brief		count lines of a file
size_t countLinesOfFile( const char *fileName );
//	\function	printFile	||	\date	2020/10/30 2:30
//	\brief		calculate columns of a file (assuming that the file has equal ammount of columns - maximum ammount of columns in any given line)
size_t countColumnsOfFile( const char *fileName );
#if defined _DEBUG && !defined NDEBUG
bool printFile( const char *fname );
//	\function	printFile	||	\date	2020/10/30 2:30
//	\brief		counts digits, white space, others
void countLetterOccurences( const char *filename );
#endif


}// util