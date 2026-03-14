#include "FolderMethods.h"
#include <string>

#ifndef WIN32

#include <unistd.h>
#include <errno.h>
#include <sys/stat.h>

int _mkdir( const char* name ) {
	return mkdir(name, 0755);
}

int _access( const char* name, int mode ) {
	return access(name, mode);
}

int _errno() {
	return errno;
}

#else
	#include <io.h>
	#include <direct.h>
	#include <windows.h>
#endif

bool exists(const char* name) {
    return _access(name, 0) == 0;
}

bool createFolderIfNotExists( const char* name ) {
	if (exists(name))
        return true;

    int errorCode = 0;
	if ((errorCode = _mkdir(name)) != 0) {
#ifdef ANDROID
        LOGI("FAILED to create folder %s, error: %d! Checking for Android 29+ scoped storage issues...", name, _errno());
        std::string pathStr(name);
        size_t docsPos = pathStr.find("/Documents");
        if (docsPos != std::string::npos) {
            std::string parentPath = pathStr.substr(0, docsPos);
            LOGI("Android 29+ scoped storage detected, trying parent path: %s", parentPath.c_str());
            if (!exists(parentPath.c_str())) {
                if ((errorCode = _mkdir(parentPath.c_str())) != 0) {
                    LOGE("FAILED to create parent folder %s, error: %d", parentPath.c_str(), _errno());
                    return false;
                }
                LOGI("Created parent folder %s", parentPath.c_str());
            }
            if ((errorCode = _mkdir(name)) == 0) {
                LOGI("Successfully created folder %s after parent creation", name);
                return true;
            } else {
                LOGE("Still FAILED to create folder %s even after parent creation, error: %d", name, _errno());
                return false;
            }
        } else {
            LOGE("FAILED to create folder %s, error: %d (not a Documents path issue)", name, _errno());
            return false;
        }
#else
        LOGI("FAILED to create folder %s, %d! Escape plan?\n", name, _errno());
        return false;
#endif // ANDROID
    }

    LOGI("Created folder %s\n", name);
    return true;
}

int getRemainingFileSize( FILE* fp ) {
	if (!fp) return 0;
	int current = ftell(fp);
	fseek(fp, 0, SEEK_END);
	int end = ftell(fp);
	fseek(fp, current, SEEK_SET);
	return end - current;
}

int getFileSize( const char* filename ) {
	FILE* fp = fopen(filename, "rb");
	if (!fp)
		return -1;

	fseek(fp, 0, SEEK_END);
	int size = ftell(fp);
	fclose(fp);
	return size;
}

bool createTree( const char* base, const char* tree[], int treeLength ) {
	if (!createFolderIfNotExists(base))
		return false;

	std::string p(base);
	for (int i = 0; i < treeLength; ++i && tree[i]) {
		p += tree[i];
		if (!createFolderIfNotExists(p.c_str()))
			return false;
	}
	return true;
}
