#include		<stdlib.h>
#include		<stdio.h>
#include		<string.h>
#include		<time.h>
#include		"lwip/debug.h"
#include		"lwip/stats.h"
#include		"lwip/tcp.h"
#include		"vfs.h"
#include		"ff.h"
//___________________________________________________________________________________________________
int*				vfs_openfs( void ) {
FATFS*			fatfs=(FATFS *)mem_malloc(sizeof(FATFS));
						f_mount(fatfs,"0:",1);
						f_chdrive("0:");
						return (int *)fatfs;
}
//___________________________________________________________________________________________________
vfs_file_t*	vfs_open( int *fd, const char *fname, const char *mode ) {
FIL*				f=(FIL *)mem_malloc(sizeof(FIL));
char				m=0;
						if(*mode=='r')
							m |= FA_READ;
						if(*mode=='w')
							m |= FA_WRITE | FA_CREATE_ALWAYS;
						if(f_open(f,(TCHAR *)fname,m)==FR_OK)
							return ((vfs_file_t *)f);
						mem_free(f); 
						return(NULL);
}
//___________________________________________________________________________________________________
ssize_t			vfs_read( void *buf,int what, int BufferSize, vfs_file_t *fp ) {
UINT				n;
						if(f_read((FIL *)fp,buf,BufferSize,&n)==FR_OK)
							return(n);
						else
							return(0);
}
//___________________________________________________________________________________________________
int					vfs_write(  void *buf,int what, int BufferSize, vfs_file_t *fp  ) {
UINT				n;
						if(f_write((FIL *)fp,buf,BufferSize,&n)==FR_OK)
							return(n);
						else
							return(0);
}
//___________________________________________________________________________________________________
int					vfs_eof( vfs_file_t *fp ) {
						return f_eof( (FIL*)fp );
}
//___________________________________________________________________________________________________
int					vfs_close(vfs_file_t *fp) {
						if(fp) {
							mem_free(fp);
						f_close( (FIL*)fp );
						}
						return 0;
}
//___________________________________________________________________________________________________
int					vfs_stat( int *fd, const char *fname, vfs_stat_t *st ) {
int					n;
TCHAR				lfn[_MAX_LFN + 1];				// long filename support
FILINFO			fno;
						fno.lfname = lfn;
						fno.lfsize = sizeof lfn;
						n=f_stat(fname,&fno)-FR_OK;
						if(!n) {
							st->st_mode=VFS_IRWXG;
							if(fno.fattrib & AM_DIR)
							st->st_mode|=VFS_IRWXU | VFS_IRWXG | VFS_IRWXO;
							st->st_size = fno.fsize;
							st->st_mtime=fno.ftime;
						}
						return(n);	
}
//___________________________________________________________________________________________________
char				*vfs_getcwd( int *fd, void *x, int y ) {
						if(f_getcwd((TCHAR *)x,y)==FR_OK)
							return(x);
						return(NULL);
}
//___________________________________________________________________________________________________
vfs_dir_t		*vfs_opendir( int *fd, char *cwd ) {
DIR					*dir=mem_malloc(sizeof(DIR));
						if(f_opendir(dir,(TCHAR *)cwd)==FR_OK) {
							f_readdir(dir,NULL);
							dbg_printf("send_data: directory %s opened\r\n",cwd);
							return((vfs_dir_t *)dir);
						}
						dbg_printf("send_data: dir. not opened\r\n");
						return(NULL);
}
//___________________________________________________________________________________________________
vfs_dirent_t	*vfs_readdir( vfs_dir_t *fd ) {
DIR					*dir=(DIR *)fd;
FILINFO			fno;
TCHAR				lfn[_MAX_LFN+1];
						fno.lfname=lfn;
						fno.lfsize=sizeof(lfn);

						if(f_readdir(dir,&fno)==FR_OK && dir->sect) {
							vfs_dirent_t *ent=(vfs_dirent_t *)mem_malloc(sizeof (vfs_dirent_t));
							if(ent) {
								ent->name=(char *)mem_malloc(sizeof(lfn));
								if(ent->name) {
									if(dir->lfn_idx != (WORD)-1)
										sprintf(ent->name,"%s",fno.lfname);
									else
										sprintf(ent->name,"%s",fno.fname);
									dbg_printf("send_data: dir. entry found, %s\r\n",ent->name);
									return ent;
								}
							}
							dbg_printf("readdir: memory error\r\n");
						} else
							dbg_printf("send_data: no entry found\r\n");
						return(NULL);
}
//___________________________________________________________________________________________________
int					vfs_chdir( int *fd, const char *arg ) {
						f_chdrive((const TCHAR *)arg );
						return(f_chdir((const TCHAR *)arg )-FR_OK);
}
//___________________________________________________________________________________________________
int					vfs_closedir( vfs_dir_t *fd ) {
						if(fd) {
							mem_free(fd);
						}
						return 0;
}
//___________________________________________________________________________________________________
int					vfs_mkdir( int *fd, const char *arg, int mode ) {
						return(f_mkdir((const TCHAR *)arg )-FR_OK);
}
//___________________________________________________________________________________________________
int					vfs_rmdir( int *fd, const char *arg ) {
						return(f_unlink((TCHAR *)arg)-FR_OK);
}
//___________________________________________________________________________________________________
int					vfs_rename( int *fd, char *oldname, const char *newname ) {
						return(f_rename( (const TCHAR *)oldname, (const TCHAR *)newname )-FR_OK);
}
//___________________________________________________________________________________________________
int					vfs_remove( int *fd, const char *arg ) {
						return(f_unlink( (const TCHAR *)arg )-FR_OK);
}

