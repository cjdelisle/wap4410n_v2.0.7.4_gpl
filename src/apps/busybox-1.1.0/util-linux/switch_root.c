/* vi:set ts=4:*/

#include <dirent.h>
#include <fcntl.h>
#include <stdio.h>
#include <sys/mount.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/vfs.h>
#include <unistd.h>

#include "busybox.h"

// Make up for header deficiencies.

#ifndef RAMFS_MAGIC
#define RAMFS_MAGIC		0x858458f6
#endif

#ifndef TMPFS_MAGIC
#define TMPFS_MAGIC		0x01021994
#endif

#ifndef MS_MOVE
#define MS_MOVE			8192
#endif

dev_t rootdev;

// Recursively delete contents of rootfs.

static void delete_contents(char *directory)
{
	DIR *dir;
	struct dirent *d;
	struct stat st;

	// Don't descend into other filesystems
	if (stat(directory,&st) || st.st_dev != rootdev) return;

	// Recursively delete the contents of directories.
	if (S_ISDIR(st.st_mode)) {
		if((dir = opendir(directory))) {
			while ((d = readdir(dir))) {
				char *newdir=d->d_name;

				// Skip . and ..
				if(*newdir=='.' && (!newdir[1] || (newdir[1]=='.' && !newdir[2])))
					continue;
				
				// Recurse to delete contents
				newdir = alloca(strlen(directory) + strlen(d->d_name) + 2);
				sprintf(newdir, "%s/%s", directory, d->d_name);
				delete_contents(newdir);
			}
			closedir(dir);
			
			// Directory should now be empty.  Zap it.
			rmdir(directory);
		}
		
	// It wasn't a directory.  Zap it.
		
	} else unlink(directory);
}

int switch_root_main(int argc, char *argv[])
{
	char *newroot, *console=NULL;
	struct stat st1, st2;
	struct statfs stfs;

	// Parse args (-c console)

	bb_opt_complementally="-2";
	bb_getopt_ulflags(argc,argv,"c:",&console);
	
	// Change to new root directory and verify it's a different fs.

	newroot=argv[optind++];
	
	if (chdir(newroot) || stat(".", &st1) || stat("/", &st2) ||
		st1.st_dev == st2.st_dev)
	{
		bb_error_msg_and_die("bad newroot %s",newroot);
	}
	rootdev=st2.st_dev;
	
	// Additional sanity checks: we're about to rm -rf /,  so be REALLY SURE
	// we mean it.  (I could make this a CONFIG option, but I would get email
	// from all the people who WILL eat their filesystemss.)

	if (stat("/init", &st1) || !S_ISREG(st1.st_mode) || statfs("/", &stfs) ||
		(stfs.f_type != RAMFS_MAGIC && stfs.f_type != TMPFS_MAGIC) ||
		getpid() != 1)
	{
		bb_error_msg_and_die("not rootfs");
	}

	// Zap everything out of rootdev

	delete_contents("/");
	
	// Overmount / with newdir and chroot into it.  The chdir is needed to
	// recalculate "." and ".." links.

	if (mount(".", "/", NULL, MS_MOVE, NULL) || chroot(".") || chdir("/"))
		bb_error_msg_and_die("moving root");
	
	// If a new console specified, redirect stdin/stdout/stderr to that.

	if (console) {
		close(0);
		if(open(console, O_RDWR) < 0)
			bb_error_msg_and_die("Bad console '%s'",console);
		dup2(0, 1);
		dup2(0, 2);
	}

	// Exec real init.  (This is why we must be pid 1.)
	execv(argv[optind],argv+optind+1);
	bb_error_msg_and_die("Bad init '%s'",argv[optind]);
}
