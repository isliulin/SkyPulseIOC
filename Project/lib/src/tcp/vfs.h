/**
 * @file ap_vfs.h
 * @brief Virtual File System interface
 *
 * @defgroup APACHE_CORE_VFS Virtual File System interface
 * @ingroup  APACHE_CORE
 * @{
 */

#ifndef APACHE_VFS_H
#define APACHE_VFS_H

#ifdef __cplusplus
extern "C" {
#endif
	
#define FTPD_DEBUG	
int 		dbg_lwip(const char *, ...);

#ifdef FTPD_DEBUG
#define dbg_printf dbg_lwip
#else
#ifdef _MSC_VER
#define dbg_printf  printf/* x */
#else
#define dbg_printf
#endif
#endif
//..#include "apr.h"
//..#include "apr_file_io.h"

//..#include "httpd.h"

#define AP_USE_VFS 1
#if !AP_USE_VFS

/* VFS Interface disabled */

#define vfs_file_t apr_file_t
#define vfs_dir_t apr_dir_t

#define ap_vfs_stat(r, finfo, fname, wanted, pool) \
    apr_stat(finfo, fname, wanted, pool)
#define ap_vfs_file_open(r, new, fname, flag, perm, pool) \
    apr_file_open(new, fname, flag, perm, pool)
#define ap_vfs_file_read(thefile, buf, nbytes) \
    apr_file_read(thefile, buf, nbytes)
#define ap_vfs_file_write(thefile, buf, nbytes) \
    apr_file_write(thefile, buf, nbytes)
#define ap_vfs_file_seek(thefile, where, offset) \
    apr_file_seek(thefile, where, offset)
#define ap_vfs_file_eof(thefile) \
    apr_file_eof(thefile)
#define ap_vfs_file_close(thefile) \
    apr_file_close(thefile)
#define ap_vfs_file_remove(r, path, pool) \
    apr_file_remove(path, pool)
#define ap_vfs_file_rename(r, from_path, to_path, p) \
    apr_file_rename(from_path, to_path, p)
#define ap_vfs_file_perms_set(r, fname, perms) \
    apr_file_perms_set(fname, perms)
#define ap_vfs_dir_open(r, new, dirname, pool) \
    apr_dir_open(new, dirname, pool)
#define ap_vfs_dir_read(finfo, wanted, thedir) \
    apr_dir_read(finfo, wanted, thedir)
#define ap_vfs_dir_close(thedir) \
    apr_dir_close(thedir)
#define ap_vfs_dir_make(r, path, perm, pool) \
    apr_dir_make(path, perm, pool)
#define ap_vfs_dir_remove(r, path, pool) \
    apr_dir_remove(path, pool)

#else

/* VFS Public Interface */

#define AP_DECLARE(x) x
typedef int apr_status_t; // DRB

//typedef struct vfs_file_t vfs_file_t;
//typedef struct vfs_dir_t vfs_dir_t;
//typedef struct vfs_mount vfs_mount;

typedef struct FIL vfs_file_t;
typedef struct DIR vfs_dir_t;
typedef struct vfs_mount vfs_mount;

#if 0 // DRB
AP_DECLARE(apr_status_t) ap_vfs_stat(const request_rec *r,
                                     apr_finfo_t *finfo, 
                                     const char *fname, 
                                     apr_int32_t wanted,
                                     apr_pool_t *pool);

AP_DECLARE(apr_status_t) ap_vfs_file_open(const request_rec *r,
                                          vfs_file_t **new, 
                                          const char *fname, 
                                          apr_int32_t flag, 
                                          apr_fileperms_t perm, 
                                          apr_pool_t *pool);

AP_DECLARE(apr_status_t) ap_vfs_file_read(vfs_file_t *thefile,
					  void *buf,
					  apr_size_t *nbytes);

AP_DECLARE(apr_status_t) ap_vfs_file_write(vfs_file_t *thefile,
					   const void *buf,
					   apr_size_t *nbytes);

AP_DECLARE(apr_status_t) ap_vfs_file_seek(vfs_file_t *thefile, 
					  apr_seek_where_t where,
					  apr_off_t *offset);

AP_DECLARE(apr_status_t) ap_vfs_file_eof(vfs_file_t *file);

AP_DECLARE(apr_status_t) ap_vfs_file_close(vfs_file_t *file);

AP_DECLARE(apr_status_t) ap_vfs_file_remove(const request_rec *r,
                                            const char *path,
                                            apr_pool_t *pool);

AP_DECLARE(apr_status_t) ap_vfs_file_rename(const request_rec *r,
                                            const char *from_path, 
                                            const char *to_path,
                                            apr_pool_t *p);

AP_DECLARE(apr_status_t) ap_vfs_file_perms_set(const request_rec *r,
                                               const char *fname, 
                                               apr_fileperms_t perms);

AP_DECLARE(apr_status_t) ap_vfs_dir_open(const request_rec *r,
                                         vfs_dir_t **new,
                                         const char *dirname, 
                                         apr_pool_t *pool);

AP_DECLARE(apr_status_t) ap_vfs_dir_read(apr_finfo_t *finfo,
                                         apr_int32_t wanted,
                                         vfs_dir_t *thedir);

AP_DECLARE(apr_status_t) ap_vfs_dir_close(vfs_dir_t *thedir);

AP_DECLARE(apr_status_t) ap_vfs_dir_make(const request_rec *r,
                                         const char *path,
                                         apr_fileperms_t perm, 
                                         apr_pool_t *pool);

AP_DECLARE(apr_status_t) ap_vfs_dir_remove(const request_rec *r,
                                           const char *path,
                                           apr_pool_t *pool);

/* VFS Provider interface */

struct vfs_mount {
    vfs_mount     *next;
    apr_uint32_t  flags;
    const char    *mountpoint;
    void          *userdata;

    /* VFS implementation hooks */
    apr_status_t (*stat)          (const vfs_mount *mnt, const request_rec *r,
				   apr_finfo_t *finfo, const char *fname, 
				   apr_int32_t wanted, apr_pool_t *pool);
    apr_status_t (*file_open)     (const vfs_mount *mnt, const request_rec *r,
				   vfs_file_t **new,  const char *fname, 
				   apr_int32_t flag,  apr_fileperms_t perm, 
				   apr_pool_t *pool);
    apr_status_t (*file_read)     (const vfs_mount *mnt, vfs_file_t *thefile,
				   void *buf, apr_size_t *nbytes);
    apr_status_t (*file_write)    (const vfs_mount *mnt, vfs_file_t *thefile,
				   const void *buf, apr_size_t *nbytes);
    apr_status_t (*file_seek)     (const vfs_mount *mnt, vfs_file_t *thefile, 
				   apr_seek_where_t where, apr_off_t *offset);
    apr_status_t (*file_eof)      (const vfs_mount *mnt, vfs_file_t *file);
    apr_status_t (*file_close)    (const vfs_mount *mnt, vfs_file_t *file);
    apr_status_t (*file_remove)   (const vfs_mount *mnt, const request_rec *r,
				   const char *path, apr_pool_t *pool);
    apr_status_t (*file_rename)   (const vfs_mount *mnt, const request_rec *r,
				   const char *from_path, const char *to_path,
				   apr_pool_t *p);
    apr_status_t (*file_perms_set)(const vfs_mount *mnt, const request_rec *r,
				   const char *fname,  apr_fileperms_t perms);
    apr_status_t (*dir_open)      (const vfs_mount *mnt, const request_rec *r,
				   vfs_dir_t **new, const char *dirname, 
				   apr_pool_t *pool);
    apr_status_t (*dir_read)      (const vfs_mount *mnt, apr_finfo_t *finfo,
				   apr_int32_t wanted, vfs_dir_t *thedir);
    apr_status_t (*dir_close)     (const vfs_mount *mnt, vfs_dir_t *thedir);
    apr_status_t (*dir_make)      (const vfs_mount *mnt, const request_rec *r,
				   const char *path, apr_fileperms_t perm, 
				   apr_pool_t *pool);
    apr_status_t (*dir_remove)    (const vfs_mount *mnt, const request_rec *r,
				   const char *path, apr_pool_t *pool);
};

/* VFS file type */

struct vfs_file_t {
    vfs_mount    *mnt;
    void         *userdata;
};

/* VFS directory type */

struct vfs_dir_t {
    vfs_mount    *mnt;
    void         *userdata;
};

/* linked list of vfs_mount structures */

extern vfs_mount *ap_vfs_mount_head;
#else

#define VFS_IRWXU 1
#define VFS_IRWXG 2
#define VFS_IRWXO 4
#define ssize_t int
#define loff_t  int
#define filldir_t int
#define VFS_ISDIR(x)   ( x & 1) // ((x) && (x) type == VFS_DIR) //! Macro to know if a given entry represents a directory 
#define VFS_ISREG( x ) ( x & 2)

#define vfs_t int                
typedef struct 
{
        int st_size;
        time_t st_mtime;
        int st_mode;
} vfs_stat_t;

typedef struct 
{
        char *name;
} vfs_dirent_t;


/* VFS file type */

struct vfs_file_t {
    vfs_mount    *mnt;
    void         *userdata;
};

/* VFS directory type */

struct vfs_dir_t {
    vfs_mount    *mnt;
    void         *userdata;
};

int *vfs_openfs( void );
vfs_file_t *vfs_open( int *fd, const char *arg, const char *mode );
ssize_t vfs_read( void *buf,int what, int BufferSize, vfs_file_t *fp );
int vfs_write(  void *buf,int what, int BufferSize, vfs_file_t *fp  );
int vfs_eof( vfs_file_t *fp );
int vfs_close( vfs_file_t *fp  );
int vfs_stat( int *fd, const char *fname, vfs_stat_t *st );
char *vfs_getcwd( int *fd, void *x, int y );
vfs_dir_t    *vfs_opendir( int *fd, char *cwd );
vfs_dirent_t *vfs_readdir( vfs_dir_t *fd );
int vfs_closedir( vfs_dir_t *fd );
int vfs_mkdir( int *fd, const char *arg, int mode );
int vfs_rmdir( int *fd, const char *arg );
int vfs_rename( int *fd, char *was, const char *arg );
int vfs_remove( int *fd, const char *arg );
int vfs_chdir( int *fd, const char *arg );


//#define vfs_eof( a )
//#define vfs_close( a )         fclose( a )
//#define vfs_readdir( a )       readdir( a )
//#define vfs_stat( a, b, c )    fstat( a, b )
//#define vfs_closedir( a )      closedir( a )

#endif // DRB

#endif /* AP_USE_VFS */

#endif /* APACHE_VFS_H */
