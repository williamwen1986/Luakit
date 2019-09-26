// Patch [LARPOUX]
// Functions that appear to be missing when targeting the Xcode iOS Simulators
// These are used by the luakit SDK OpenSSL plugin
#include <dirent.h>
#include <fnmatch.h>
extern "C" DIR * opendir$INODE64( char * dirName );
DIR * opendir$INODE64( char * dirName )
{
    return opendir( dirName );
}
 
extern "C" struct dirent * readdir$INODE64( DIR * dir );
struct dirent * readdir$INODE64( DIR * dir )
{
    return readdir( dir );
}
