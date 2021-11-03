#include "file_utils.h"
#include <iostream>
#include "console.h"


namespace util
{

namespace fs = std::filesystem;


bool displayFileInfo( const fs::path& targetPath )
{
	if ( fs::exists( targetPath ) )
	{
		std::cout << "root_name: " << targetPath.root_name() << '\n'
			<< "root_path: " << targetPath.root_path() << '\n'
			<< "relative_path: " << targetPath.relative_path() << '\n'
			<< "parent_path: " << targetPath.parent_path() << '\n'
			<< "filename: " << targetPath.filename() << '\n'
			<< "stem: " << targetPath.stem() << '\n'
			<< "extension: " << targetPath.extension() << '\n';
		return true;
	}
	return false;
}

void displayDirTree( const fs::path& targetPath,
	int level )
{
	if ( fs::exists( targetPath ) && fs::is_directory( targetPath ) )
	{
		auto lead = std::string( level * 3, ' ' );
		for ( const auto& entry : fs::directory_iterator( targetPath ) )
		{
			auto filename = entry.path().filename();
			if ( fs::is_directory( entry.status() ) )
			{
				std::cout << "==============\n";
				std::cout << lead
					<< "[+] "
					<< filename
					<< "\n";
				displayDirTree( entry,
					level + 1 );
				std::cout << "\n";
			}
			else if ( fs::is_regular_file( entry.status() ) )
			{
				std::cout << '\n';
				displayFileInfo( filename );
			}
			else
			{
				std::cout << lead
					<< " [?]"
					<< filename
					<< "\n";
			}
		}
	}
}

size_t getFileSize( const fs::path& pathToCheck )
{
	if ( fs::exists( pathToCheck ) &&
		fs::is_regular_file( pathToCheck ) )
	{
		auto err = std::error_code{};
		auto filesize = fs::file_size( pathToCheck,
			err );
		if ( filesize != static_cast<size_t>( -1 ) )
		{
			return filesize;
		}
	}

	return static_cast<size_t>( -1 );
}

bool displayTextFile( const fs::path& targetPath )
{
	std::ifstream f{targetPath.string(), std::ios::beg};
	std::string s;
	if ( f.is_open() )
	{
		while( std::getline( f, s ) )
		{
			std::cout << s << '\n';
		}
		return true;
	}
	return false;
}

bool displayBinaryFile( const fs::path& targetPath )
{
	std::ifstream f{targetPath.string(), std::ios::binary};
	std::string s;
	if ( f.is_open() )
	{
		while( std::getline( f, s ) )
		{
			std::cout << s << '\n';
		}
		return true;
	}
	return false;
}

void writeFile( const fs::path& path,
	const std::string& data )
{
	std::ofstream out{path, std::ios::out};
	if ( out )
	{
		out << data;
		out.close();
	}
}

void deleteFile( const std::string& filename )
{
	try
	{
		if ( !std::filesystem::remove( filename ) )
		{
#if defined _DEBUG && !defined NDEBUG
			std::cout << "file "
				<< filename
				<< " not found.\n";
#endif
		}
	}
	catch ( const std::filesystem::filesystem_error& ex )
	{
		std::cout << "filesystem error: "
			<< ex.what()
			<< '\n';
	}
}

void searchFile( const fs::path& directory,
	const fs::path& filename )
{
	auto d = fs::directory_iterator( directory );

	auto found = std::find_if( d,
		end( d ),
		[&filename]( const auto& dir )
		{
			return dir.path().filename() == filename;
		} );

	if ( found != end( d ) )
	{
		// we have found what we were looking for
	}
}

bool createDirectory( const std::string& str )
{
	return fs::create_directories( str );
}

void setFilePermissions( const std::string& path,
	fs::perms permsToSet,
	fs::perm_options permsAction ) 
{
	fs::permissions( path,
		permsToSet,
		permsAction );
}

const std::string getFileExtension( const std::string& filename )
{
	const std::filesystem::path path{filename};
	return path.extension().string();
}

bool isFileBinary( const char* fname )
{
	char c;
	std::ifstream ifs{fname, std::ios::binary};
	unsigned charsRead = 0;
	while ( ( c = ifs.get() ) != EOF && charsRead < 255 )
	{
		if ( c == '\0' )
		{
			return true;
		}
		++charsRead;
	}
	return false;
}

#if defined _DEBUG && !defined NDEBUG
bool printFile( const char* fname )
{
	std::ifstream ifs{fname};
	KeyConsole& console = KeyConsole::getInstance();
	if ( !ifs.is_open() )
	{
		console.log( "can't open " + std::string{*fname} + "!\n" );
		return false;
	}
	char c;
	while ( !ifs.eof() )
	{
		c = ifs.get();
		console.print( std::string{c} );
	}
	return true;
}
#endif


}// util