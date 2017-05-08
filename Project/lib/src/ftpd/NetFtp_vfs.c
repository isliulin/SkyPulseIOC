//
//#include "mbed.h"                  // DRB
//#include "SDFileSystem.h"          // DRB

#define printit printf

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <time.h>
#include "NetFtp_vfs.h"
#include "ff.h"

//}

// extern void printit( char *p_buf );

int  *vfs_openfs( void )
{
	//UINT	n;
	FATFS	*fatfs=(FATFS *)malloc(sizeof(FATFS));
	FIL		*f=(FIL *)malloc(sizeof(FIL));

	f_mount(0,fatfs);
	//f_mkfs(0,1,4096);

	//f_open(f,L"test.c",FA_WRITE + FA_CREATE_NEW + FA_CREATE_ALWAYS);
	//f_write(f,"1 2 3",16,&n);
	//f_close(f);
	//f_open(f,L"test.h",FA_WRITE + FA_CREATE_NEW + FA_CREATE_ALWAYS);
	//f_write(f,"a b c",16,&n);
	//f_close(f);
	//f_open(f,L"data.h",FA_WRITE + FA_CREATE_NEW + FA_CREATE_ALWAYS);
	//f_write(f,"b z ž",16,&n);
	//f_close(f);

    return (int *)fatfs;
}

vfs_file_t *vfs_open( int *fd, const char *fname, const char *mode )
{
	FIL *f=(FIL *)malloc(sizeof(FIL));
	char	m=0;
	if(*mode=='r')
		m |= FA_READ;
	if(*mode=='w')
		m |= FA_WRITE + FA_CREATE_NEW;
	
	if(f_open(f,(TCHAR *)fname,m)==FR_OK)
		return ((vfs_file_t *)f);
	free(f); 
	return(NULL);
}

ssize_t vfs_read( void *buf,int what, int BufferSize, vfs_file_t *fp )
{
	UINT n;
    if(f_read((FIL *)fp,buf,BufferSize,&n)==FR_OK)
		return(n);
	else
		return(0);
}

int vfs_write(  void *buf,int what, int BufferSize, vfs_file_t *fp  )
{
	UINT n;
    if(f_write((FIL *)fp,buf,BufferSize,&n)==FR_OK)
		return(1);
	else
		return(0);
}

int vfs_eof( vfs_file_t *fp )
{
	return f_eof( (FIL*)fp );
}

int vfs_close( vfs_file_t *fp  )
{
	if( NULL != fp )
    {
        f_close( (FIL*)fp );
    }
    return 0;
}

int vfs_stat( int *fd, const char *fname, vfs_stat_t *st )
{
	int n;
	FILINFO fno;
	TCHAR	lfn[_MAX_LFN+1];
	fno.lfname=lfn;
	fno.lfsize=sizeof(lfn);

	n=f_stat((TCHAR *)fname,&fno)-FR_OK;
	if(!n)
	{
		st->st_mode=VFS_IRWXG;
		if(fno.fattrib & AM_DIR)
			st->st_mode|=VFS_IRWXU | VFS_IRWXG | VFS_IRWXO;
		
		st->st_size = fno.fsize;
		st->st_mtime=fno.ftime;
	}
	return(n);

}

char *vfs_getcwd( int *fd, void *x, int y )
{
	char *cwd=(char *)malloc(128);
	if(f_getcwd((TCHAR *)cwd,128)==FR_OK)
		return(cwd);
	free(cwd);
	return(NULL);
}

vfs_dir_t  *vfs_opendir( int *fd, char *cwd )
{
	DIR	*dir=(DIR *)malloc(sizeof(DIR));
	if(f_opendir(dir,(TCHAR *)cwd)==FR_OK)
	{
		f_readdir(dir,NULL);
		return((vfs_dir_t *)dir);
	}
	else
		return(NULL);
}

vfs_dirent_t *vfs_readdir( vfs_dir_t *fd )
{
	FILINFO fno;
	TCHAR	lfn[_MAX_LFN+1];
	fno.lfname=lfn;
	fno.lfsize=sizeof(lfn);
    if(f_readdir((DIR *)fd,&fno)==FR_OK)
	{
		if(*fno.fname != '\0')
		{
			vfs_dirent_t * ent=(vfs_dirent_t *)malloc(sizeof (vfs_dirent_t));
			ent->name=(char *)malloc(sizeof fno.fname);
			memcpy(ent->name, fno.fname, sizeof(fno.fname));
			return ent;
		}
	}
	return(NULL);
}

int vfs_closedir( vfs_dir_t *fd )
{
	return 0;
}
int vfs_mkdir( int *fd, const char *arg, int mode )
{
    return(f_mkdir((const TCHAR *)arg )-FR_OK);
}
int vfs_rmdir( int *fd, const char *arg )
{
	return(f_unlink((TCHAR *)arg)-FR_OK);
}
int vfs_rename( int *fd, char *oldname, const char *newname )
{
    return(f_rename( (const TCHAR *)oldname, (const TCHAR *)newname )-FR_OK);
}
int vfs_remove( int *fd, const char *arg )
{
    return(f_unlink( (const TCHAR *)arg )-FR_OK);
}
int vfs_chdir( int *fd, const char *arg )
{
    return(f_chdir((const TCHAR *)arg )-FR_OK);
}
