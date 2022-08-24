/************************************************************
 *
 * (c) 2007 Olivier Castan castan.o@free.fr
 * Modified by Simson Garfinkel, to fit into the AFFLIB build system.
 * Modified by Pasquale J. Rinaldi, Jr., to fit into the AFF4 build system.
 *
 * License: LGPL or BSD-4
 *
 * KISS: based on fuse hello.c example
 *
 * TODO: - use xattr to display informations from segments
 *       - use AF_ACQUISITION_DATE for creation date
 *       - option between BADFLAG and NULLs
 *       - ...
 *
 * *********************************************************/


#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include </usr/local/include/aff4/aff4.h>
#include </usr/local/include/aff4/aff4-c.h>
#include <assert.h>

#define FUSE_USE_VERSION 31

#include <fuse.h>
#include <fcntl.h>
#include <libgen.h>


/*
static struct options {
    const char* filename;
    int show_help;
} options;
*/

//#define OPTION(t, p) { t, offsetof(struct options, p), 1}

/*
static const struct fuse_opt option_spec[] = {
    //OPTION("--name=%s", filename),
    OPTION("-h", show_help),
    OPTION("--help", show_help),
    FUSE_OPT_END
};
*/

#define XCALLOC(type, num) ((type *) xcalloc ((num), sizeof(type)))

static off64_t imgsize = 0;
static char* imgpath = NULL;
static char* mntpath = NULL;


static void* aff4fuse_init(struct fuse_conn_info* conn, struct fuse_config* cfg)
{
    (void) conn;
    cfg->kernel_cache = 1;
    return NULL;
}

static int
aff4fuse_getattr(const char *path, struct stat *stbuf, struct fuse_file_info *fi)
{
    (void) fi;
    int res = 0;

    memset(stbuf, 0, sizeof(struct stat));
    if(strcmp(path, "/") == 0) {
        stbuf->st_mode = S_IFDIR | 0755;
        stbuf->st_nlink = 2;
    }
    else if(strcmp(path, mntpath) == 0) {
        stbuf->st_mode = S_IFREG | 0444;
        stbuf->st_nlink = 1;
	stbuf->st_size = imgsize;
    }
    else
        res = -ENOENT;

    return res;
}

static int
aff4fuse_readdir(const char *path, void *buf, fuse_fill_dir_t filler,
               off_t offset, struct fuse_file_info *fi, enum fuse_readdir_flags flags)
{
    (void) offset;
    (void) fi;
    (void) flags;

    if(strcmp(path, "/") != 0)
        return -ENOENT;

    filler(buf, ".", NULL, 0, (fuse_fill_dir_flags)0);
    filler(buf, "..", NULL, 0, (fuse_fill_dir_flags)0);
    filler(buf, mntpath + 1, NULL, 0, (fuse_fill_dir_flags)0);

    return 0;
}

static int
aff4fuse_open(const char *path, struct fuse_file_info *fi)
{
    if(strcmp(path, mntpath) != 0)
        return -ENOENT;

    if((fi->flags & O_ACCMODE) != O_RDONLY)
        return -EACCES;

    return 0;
}

static int
aff4fuse_read(const char *path, char *buf, size_t size, off_t offset,
            struct fuse_file_info *fi)
{
    int res = 0;
    (void) fi;
    if(strcmp(path, mntpath) != 0){
        return -ENOENT;
    }

    errno = 0;
    AFF4_init();
    int aff4handle = AFF4_open(imgpath);
    res = AFF4_read(aff4handle, offset, buf, size);
    AFF4_close(aff4handle);
    if (res<0){
	if (errno==0) errno=-EIO;
	else res = -errno;
    }
    return res;
}

static const struct fuse_operations aff4fuse_oper = {
    .getattr    = aff4fuse_getattr,
    .open	= aff4fuse_open,
    .read	= aff4fuse_read,
    .readdir	= aff4fuse_readdir,
    .init	= aff4fuse_init,
};

/*
static void show_help(const char* progname)
{
    printf("ProgName: %s\n", progname);
}
*/

static void* xmalloc(size_t num)
{
    void* alloc = malloc(num);
    if(!alloc)
    {
	perror("Memory exhausted");
	exit(EXIT_FAILURE);
    }
    return alloc;
}

static void* xcalloc(size_t num, size_t size)
{
    void* alloc = xmalloc(num*size);
    memset(alloc, 0, num*size);
    return alloc;
}

int main(int argc, char **argv)
{
    int ret;
    char* affbasename = NULL;
    size_t pathlen = 0;
    //char* aff4path = NULL;
    const char* rawext = ".dd";

    char** fargv = NULL;
    int fargc = 0;
    fargv = (char**)malloc(argc);
    fargv[0] = argv[0];
    fargv[1] = argv[2];
    fargc = 2;


    imgpath = argv[1]; // img path
    //aff4path = argv[2]; //mnt path 
    
    //printf("Mount Path Arg: %s\n", aff4path);
    //printf("Raw Ext: %s\n", rawext);
    
    affbasename = basename(imgpath);
    //printf("AFF4 BaseName: %s\n", affbasename);
    //printf("base length: %ld\n", strlen(affbasename));
    //printf("rawext length: %ld\n", strlen(rawext));
    pathlen = 1 + strlen(affbasename) + strlen(rawext) + 1;
    //printf("path length: %ld\n", pathlen);
    mntpath = XCALLOC(char, pathlen);
    mntpath[0] = '/';
    strcat(mntpath, affbasename);
    strcat(mntpath, rawext);
    mntpath[pathlen - 1] = 0;

    //printf("Mnt Path: %s\n", mntpath);

    AFF4_init();
    int aff4handle = AFF4_open(imgpath);
    imgsize = AFF4_object_size(aff4handle);
    AFF4_close(aff4handle);
    
    //printf("Img Size: %ld\n", imgsize);
    //mntpt = argv[2];
    //printf("Args: %s, %s\n", argv[1], argv[2]);
    //printf("Img path: %s\n", imgpath);
    
    //struct fuse_args args = FUSE_ARGS_INIT(argc, argv);
    //options.filename = strdup("aff4fuse");
    /*
    if(fuse_opt_parse(&args, &options, option_spec, NULL) == -1)
	return 1;
    if(options.show_help)
    {
	show_help(argv[0]);
	assert(fuse_opt_add_arg(&args, "--help") == 0);
	args.argv[0][0] = '\0';
    }
    */

    ret = fuse_main(fargc, fargv, &aff4fuse_oper, NULL);

    //ret = fuse_main(args.argc, args.argv, &aff4fuse_oper, NULL);
    //fuse_opt_free_args(&args);

    return ret;
}
