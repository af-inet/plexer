#include "file.h"

//ntfw(".", &cb, 0, 0)
//static int cb(const char *fpath,const struct stat *sb,int tflag,struct FTW *ftwbuf);
//isdir = sb->st_mode & S_IFDIR;

static void (*g_callback)(const char *filename);

static int plxr_ntfw_callback(
	const char *fpath,
	const struct stat *sb,
	int tflag,
	struct FTW *ftwbuf)
{
	if ( sb->st_mode & S_IFREG ) {
		g_callback(fpath);
	}

	return 0;
}

void plxr_ntfw(void (*callback)(const char *filename)) {
	g_callback = callback;
	if ( nftw(".", &plxr_ntfw_callback, 0, FTW_PHYS) == -1 ) {
		perror("ntfw");
	}
}

// TEMP
void plxr_readdir() {	
	struct dirent *ent;
	DIR *dir = opendir(".");

	if ( dir == NULL ) {
		perror("opendir");
	}

	for(;;) {
		ent = readdir(dir);

		if (ent) {
			printf("%s\n", ent->d_name);
		} else {
			perror("readdir");
			break;
		}
	}
	
	closedir(dir);
}


char *plxr_alloc_file(char *filename, off_t *size){
	struct stat info;
	char *buffer;
	int fd;

	fd = open(filename, O_RDONLY);

	if( fd == -1 )
	{
		perror("open");
		return NULL;
	}

	if( fstat(fd, &info) != 0 )
	{
		perror("fstat");
		close(fd);
		return NULL;
	}

	buffer = malloc(info.st_size + 1);

	if( buffer == NULL )
	{
		perror("malloc");
		close(fd);
		return NULL;
	}

	if( read(fd, buffer, info.st_size) == -1 )
	{
		perror("read");
		close(fd);
		return NULL;
	}

	buffer[info.st_size] = '\0'; // null terminator
	close(fd);
	*size = info.st_size;
	return buffer;
}

