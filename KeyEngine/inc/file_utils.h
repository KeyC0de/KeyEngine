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
const std::string getFileExtension( const std::string &filename );
//===================================================
//	\function	isFileBinary
//	\brief  read 255 chars just to be sure
//	\date	2020/10/30 2:31
bool isFileBinary( const char *fname );
//===================================================
//	\function	printFile
//	\brief  for text files
//	\date	2020/10/30 2:30
// count lines of a file
size_t countLinesOfFile( char *fileName );
// calculate columns of a file (assuming that the file has equal ammount of columns - maximum ammount of columns in any given line)
size_t countColumnsOfFile( char *fileName );
#if defined _DEBUG && !defined NDEBUG
bool printFile( const char *fname );
// counts digits, white space, others
void countLetterOccurences( char *filename );
#endif


}// util