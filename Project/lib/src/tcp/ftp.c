#include "app.h"
#include <time.h>
#include <stdarg.h>
#include <ctype.h>
#include	"vfs.h"
#include	"lwip/tcp.h"

#define msg110 "110 MARK %s = %s."
/*
         110 Restart marker reply.
             In this case, the text is exact and not left to the
             particular implementation; it must read:
                  MARK yyyy = mmmm
             Where yyyy is User-process data stream marker, and mmmm
             server's equivalent marker (note the spaces between markers
             and "=").
*/
#define msg120 			"120 Service ready in nnn minutes."
#define msg125			"125 Data connection already open; transfer starting."
#define msg150			"150 File status okay; about to open data connection."
#define msg150recv	"150 Opening BINARY mode data connection for %s (%i bytes)."
#define msg150stor	"150 Opening BINARY mode data connection for %s."
#define msg200			"200 Command okay."
#define msg202			"202 Command not implemented, superfluous at this site."
#define msg211			"211 System status, or system help reply."
#define msg212			"212 Directory status."
#define msg213			"213 %d"
#define msg214			"214 %s."
/*
             214 Help message.
             On how to use the server or the meaning of a particular
             non-standard command.  This reply is useful only to the
             human user.
*/
#define msg214SYST "214 %s system type."
/*
         215 NAME system type.
             Where NAME is an official system name from the list in the
             Assigned Numbers document.
*/
#define msg220 "220 lwIP FTP Server ready."
/*
         220 Service ready for new user.
*/
#define msg221 "221 Goodbye."
/*
         221 Service closing control connection.
             Logged out if appropriate.
*/
#define msg225 "225 Data connection open; no transfer in progress."
#define msg226 "226 Closing data connection."
/*
             Requested file action successful (for example, file
             transfer or file abort).
*/
#define msg227 "227 Entering Passive Mode (%i,%i,%i,%i,%i,%i)."
#define xmsg227 "227 =%i,%i,%i,%i,%i,%i"
/*
         227 Entering Passive Mode (h1,h2,h3,h4,p1,p2).
*/
#define msg230 "230 User logged in, proceed."
#define msg250 "250 Requested file action okay, completed."
#define msg257PWD "257 \"%s\" is current directory."
#define msg257 "257 \"%s\" created."
/*
         257 "PATHNAME" created.
*/
#define msg331 "331 User name okay, need password."
#define msg332 "332 Need account for login."
#define msg350 "350 Requested file action pending further information."
#define msg421 "421 Service not available, closing control connection."
/*
             This may be a reply to any command if the service knows it
             must shut down.
*/
#define msg425 "425 Can't open data connection."
#define msg426 "426 Connection closed; transfer aborted."
#define msg450 "450 Requested file action not taken."
/*
             File unavailable (e.g., file busy).
*/
#define msg451 "451 Requested action aborted: local error in processing."
#define msg452 "452 Requested action not taken."
/*
             Insufficient storage space in system.
*/
#define msg500 "500 Syntax error, command unrecognized."
/*
             This may include errors such as command line too long.
*/
#define msg501 "501 Syntax error in parameters or arguments."
#define msg502 "502 Command not implemented."
#define msg503 "503 Bad sequence of commands."
#define msg504 "504 Command not implemented for that parameter."
#define msg530 "530 Not logged in."
#define msg532 "532 Need account for storing files."
#define msg550 "550 Requested action not taken."
/*
             File unavailable (e.g., file not found, no access).
*/
#define msg551 "551 Requested action aborted: page type unknown."
#define msg552 "552 Requested file action aborted."
/*
             Exceeded storage allocation (for current directory or
             dataset).
*/
#define msg553 "553 Requested action not taken."
/*
             File name not allowed.
*/

enum ftpd_state_e {
				FTPD_USER,
				FTPD_PASS,
				FTPD_IDLE,
				FTPD_NLST,
				FTPD_LIST,
				FTPD_RETR,
				FTPD_RNFR,
				FTPD_STOR,
				FTPD_QUIT
};

#define	_CLOSED 		0
#define	_CLOSING 		1
#define	_CONNECTED 	2

static 	const char *month_table[12] = {
				"Jan",
				"Feb",
				"Mar",
				"Apr",
				"May",
				"Jun",
				"Jul",
				"Aug",
				"Sep",
				"Oct",
				"Nov",
				"Dec"
};

time_t	time(time_t *t) {
				*t=__time__;
				return *t;
}

struct	ftpd_datastate {
				int state;
				vfs_dir_t *vfs_dir;
				vfs_dirent_t *vfs_dirent;
				vfs_file_t *vfs_file;
				struct pbuf	*p;
				struct tcp_pcb *msgpcb;
				struct ftpd_msgstate *msgfs;
};

struct	ftpd_msgstate {
				enum ftpd_state_e state;
				struct pbuf	*p;
				vfs_t *vfs;
				struct ip_addr dataip;
				u16_t dataport;
				struct tcp_pcb *datapcb;
				struct ftpd_datastate *datafs;
				int passive;
				char *renamefrom;
};

static	err_t		ftpd_dataconnected(void *, struct tcp_pcb *, err_t );
static	err_t		ftpd_datasent(void *, struct tcp_pcb *, u16_t);
static	void		send_file( struct ftpd_datastate *, struct tcp_pcb * );
static	void		send_data(struct tcp_pcb *, struct ftpd_datastate *);
static	void		send_msg(struct tcp_pcb *pcb, struct ftpd_msgstate *fsm, char *msg, ...);
static	void		ftpd_dataclose(struct tcp_pcb *, struct ftpd_datastate *);
static	void		ftpd_dataerr(void *, err_t);
static	void		send_next_directory(struct ftpd_datastate *, struct tcp_pcb *, int);

static err_t 		ftpd_datarecv(void *arg, struct tcp_pcb *pcb, struct pbuf *p, err_t err) {
	struct ftpd_datastate *fsd = (struct ftpd_datastate*)arg;
	if (err == ERR_OK && p != NULL) {
		struct pbuf *q;
		u16_t tot_len = 0;
		for (q = p; q != NULL; q = q->next) {
			int len = vfs_write(q->payload, 1, q->len, fsd->vfs_file);
			tot_len += len;
			if (len != q->len)
				break;
		}
		pbuf_free(p);
		tcp_recved(pcb, tot_len);					/* Inform TCP that we have taken the data. */
	}
	if (err == ERR_OK && p == NULL) {
		if(fsd->p) {
			fsd->state=_CLOSING;
			tcp_sent(pcb, ftpd_datasent);
			tcp_err(pcb, ftpd_dataerr);
			send_data(pcb,fsd);
		}	else {
			struct ftpd_msgstate *fsm= fsd->msgfs;
			struct tcp_pcb *msgpcb= fsd->msgpcb;
			f_close((FIL *)fsd->vfs_file);
			fsd->vfs_file = NULL;
			ftpd_dataclose(pcb, fsd);
			fsm->datapcb = NULL;
			fsm->datafs = NULL;
			fsm->state = FTPD_IDLE;
			send_msg(msgpcb, fsm, msg226);
		}
	}
	return ERR_OK;
}

static err_t ftpd_dataaccept(void *arg, struct tcp_pcb *pcb, err_t err)
{
    struct ftpd_datastate *fsd = (struct ftpd_datastate*)arg;
//    dbg_printf("\r\ndatapcb=%08X, dataarg=%08X\r\n", pcb,arg);

    fsd->msgfs->datapcb = pcb;
    fsd->state=_CONNECTED;
    /* Tell TCP that we wish to be informed of incoming data by a call
       to the http_recv() function. */
    tcp_recv(pcb, ftpd_datarecv);
    /* Tell TCP that we wish be to informed of data that has been
       successfully sent by a call to the ftpd_sent() function. */
    switch (fsd->msgfs->state) {
    case FTPD_LIST:
        send_next_directory(fsd, pcb, 0);
        break;
    case FTPD_NLST:
        send_next_directory(fsd, pcb, 1);
        break;
    case FTPD_RETR:
        send_file(fsd, pcb);
        break;
    default:
        break;
    }
    return ERR_OK;
}

static int open_dataconnection(struct tcp_pcb *pcb, struct ftpd_msgstate *fsm)
{
    if (fsm->passive)
        return 0;

    /* Allocate memory for the structure that holds the state of the connection. */
    fsm->datafs = (struct ftpd_datastate*)mem_malloc(sizeof(struct ftpd_datastate));

    if (fsm->datafs == NULL) {
        send_msg(pcb, fsm, msg451);
        return 1;
    }
    memset(fsm->datafs, 0, sizeof(struct ftpd_datastate));
    fsm->datafs->msgfs = fsm;
    fsm->datafs->msgpcb = pcb;
		fsm->datapcb = tcp_new();
    tcp_bind(fsm->datapcb, &pcb->local_ip, 20);
    tcp_arg(fsm->datapcb, fsm->datafs);
    tcp_connect(fsm->datapcb, &fsm->dataip, fsm->dataport, ftpd_dataconnected);
    return 0;
}

static err_t ftpd_dataconnected(void *arg, struct tcp_pcb *pcb, err_t err)
{
    struct ftpd_datastate *fsd = (struct ftpd_datastate*)arg;

    fsd->msgfs->datapcb = pcb;
    fsd->state = _CONNECTED;

    /* Tell TCP that we wish to be informed of incoming data by a call
       to the http_recv() function. */
    tcp_recv(pcb, ftpd_datarecv);

    /* Tell TCP that we wish be to informed of data that has been
       successfully sent by a call to the ftpd_sent() function. */
    switch (fsd->msgfs->state) {
    case FTPD_LIST:
        send_next_directory(fsd, pcb, 0);
        break;
    case FTPD_NLST:
        send_next_directory(fsd, pcb, 1);
        break;
    case FTPD_RETR:
        send_file(fsd, pcb);
        break;
    default:
        break;
    }

    return ERR_OK;
}

static void ftpd_dataclose(struct tcp_pcb *pcb, struct ftpd_datastate *fsd)
{
    tcp_arg(pcb, NULL);
    tcp_sent(pcb, NULL);
    tcp_recv(pcb, NULL);
    tcp_arg(pcb, NULL);
    tcp_close(pcb);
    mem_free(fsd);
}

static void send_data(struct tcp_pcb *tpcb, struct ftpd_datastate *es)
{
		struct pbuf *ptr;
		err_t wr_err = ERR_OK;	 
		if(tpcb->snd_queuelen > TCP_SND_QUEUELEN-1)	// !!!! pred vpisom preveri, ce ni queue ze poln  
			tcp_output(tpcb);			
		while ((wr_err == ERR_OK) &&
					 (es->p != NULL) && 
					 (es->p->len <= tcp_sndbuf(tpcb)))
		{
			ptr=es->p;																/* get pointer on pbuf from es structure & enqueue data for transmission */
			wr_err=tcp_write(tpcb,ptr->payload,ptr->len, 1);
			if (wr_err == ERR_OK) {
				es->p = ptr->next;
				if (es->p != NULL) {
					pbuf_ref(es->p); 											/* increment reference count for es->p */
				}
				pbuf_free(ptr);													/* free pbuf: will free pbufs up to es->p (because es->p has a reference count > 0) */
			} else if(wr_err == ERR_MEM) { 						/* we are low on memory, try later / harder, defer to poll */
				es->p = ptr;
		 } else { }																	/* other problem ?? */
		}
}

static err_t ftpd_datasent(void *arg, struct tcp_pcb *pcb, u16_t len) {
struct 
	ftpd_datastate *fsd = (struct ftpd_datastate*)arg;		
	if(fsd->state ==_CLOSING)	{
		struct ftpd_msgstate *fsm= fsd->msgfs;
		struct tcp_pcb *msgpcb= fsd->msgpcb;
		vfs_close(fsd->vfs_file);
		fsd->vfs_file = NULL;
		ftpd_dataclose(pcb, fsd);
		fsm->datapcb = NULL;
		fsm->datafs = NULL;
		fsm->state = FTPD_IDLE;
		send_msg(msgpcb, fsm, msg226);
		} else {
			switch (fsd->msgfs->state) {
				case FTPD_LIST:
					send_next_directory(fsd, pcb, 0);
					break;
				case FTPD_NLST:
					send_next_directory(fsd, pcb, 1);
					break;
				case FTPD_RETR:
					send_file(fsd, pcb);
					break;
				default:
				break;
			}
		}
		return ERR_OK;
}

static void ftpd_dataerr(void *arg, err_t err) {
struct 
	ftpd_datastate *fsd = (struct ftpd_datastate*)arg;
	dbg_printf("ftpd_dataerr: %s (%i)\n", lwip_strerr(err), err);
	if (fsd == NULL)
		return;
	fsd->msgfs->datafs = NULL;
	fsd->msgfs->state = FTPD_IDLE;
	mem_free(fsd);
}

static void send_file( struct ftpd_datastate *fsd, struct tcp_pcb *pcb ) {
	if (fsd->state != _CONNECTED)
		return;
	if (fsd->vfs_file  && !vfs_eof(fsd->vfs_file)) {
		struct	pbuf *ptr;
		int len=f_size((FIL *)fsd->vfs_file)-f_tell((FIL *)fsd->vfs_file);
		char buffer[256];
		
		if(len>sizeof(buffer))
			len=sizeof(buffer);
		if(len>tcp_sndbuf(pcb))
			len=tcp_sndbuf(pcb);
		ptr = pbuf_alloc(PBUF_TRANSPORT, len , PBUF_POOL);
		if(!ptr) {
			tcp_sent(pcb, ftpd_datasent);
			tcp_err(pcb, ftpd_dataerr);
			send_data(pcb, fsd);
			return;
		}
		len = vfs_read( buffer, 1, sizeof(buffer), fsd->vfs_file );
		if(fsd->p)
			pbuf_chain(fsd->p,ptr);
		else
			fsd->p=ptr;
		pbuf_take(fsd->p,buffer,len);
		tcp_sent(pcb, ftpd_datasent);
		tcp_err(pcb, ftpd_dataerr);
		send_data( pcb, fsd );
	} else {
		struct ftpd_msgstate *fsm;
		struct tcp_pcb *msgpcb;
		if (fsd->p) {
			tcp_sent(pcb, ftpd_datasent);
			tcp_err(pcb, ftpd_dataerr);
			send_data(pcb, fsd);
			return;
		}
		fsm = fsd->msgfs;
		msgpcb = fsd->msgpcb;
		vfs_close( fsd->vfs_file );
		fsd->vfs_file = NULL;
		ftpd_dataclose( pcb, fsd );
		fsm->datapcb = NULL;
		fsm->datafs = NULL;
		fsm->state = FTPD_IDLE;
		send_msg( msgpcb, fsm, msg226 );
		return;
	}
}

static void send_next_directory(struct ftpd_datastate *fsd, struct tcp_pcb *pcb, int shortlist)
{
    char 		buffer[256];
		struct	pbuf *ptr;
		int			len;
    if (fsd->state != _CONNECTED)
        return;
		fsd->vfs_dirent = vfs_readdir(fsd->vfs_dir);
		if (fsd->vfs_dirent ) {
				if (shortlist)
					len = sprintf(buffer, "%s\r\n", fsd->vfs_dirent->name);
				else {
					vfs_stat_t st;
					time_t current_time;
					int current_year;
					struct tm *s_time;
					time(&current_time);
					s_time = gmtime(&current_time);
					current_year = s_time->tm_year;
					vfs_stat(fsd->msgfs->vfs, fsd->vfs_dirent->name, &st);
					s_time = gmtime(&st.st_mtime);
					if (s_time->tm_year == current_year)
						len = sprintf(buffer, "-rwxrwxrwx   1 user     ftp  %11d %s %02i %02i:%02i %s\r\n", st.st_size, month_table[s_time->tm_mon], s_time->tm_mday, s_time->tm_hour, s_time->tm_min, fsd->vfs_dirent->name);
					else
						len = sprintf(buffer, "-rwxrwxrwx   1 user     ftp  %11d %s %02i %5i %s\r\n", st.st_size, month_table[s_time->tm_mon], s_time->tm_mday, s_time->tm_year + 1900, fsd->vfs_dirent->name);
					if (VFS_ISDIR(st.st_mode))
						buffer[0] = 'd';
					}
				ptr = pbuf_alloc(PBUF_TRANSPORT, len , PBUF_POOL);
				if(!ptr) {
					tcp_sent(pcb, ftpd_datasent);
					tcp_err(pcb, ftpd_dataerr);
          send_data(pcb, fsd);
          return;
				}	
				if(fsd->p)
					pbuf_chain(fsd->p,ptr);
				else
					fsd->p=ptr;
				pbuf_take(fsd->p,buffer,len);
				tcp_sent(pcb, ftpd_datasent);
				tcp_err(pcb, ftpd_dataerr);
				send_data( pcb, fsd );
			} else {
        struct ftpd_msgstate *fsm;
        struct tcp_pcb *msgpcb;

        if (fsd->p) {
					tcp_sent(pcb, ftpd_datasent);
					tcp_err(pcb, ftpd_dataerr);
          send_data(pcb, fsd);
          return;
        }
        fsm = fsd->msgfs;
        msgpcb = fsd->msgpcb;

				vfs_closedir(fsd->vfs_dir);
        fsd->vfs_dir = NULL;
        ftpd_dataclose( pcb, fsd );
        fsm->datapcb = NULL;
        fsm->datafs = NULL;
        fsm->state = FTPD_IDLE;
        send_msg( msgpcb, fsm, msg226 );
        return;
    }
}

static void cmd_user(const char *arg, struct tcp_pcb *pcb, struct ftpd_msgstate *fsm)
{
    send_msg(pcb, fsm, msg331);
    fsm->state = FTPD_PASS;
//		send_msg(pcb, fs, msgLoginFailed);
//		fs->state = FTPD_QUIT;
}

static void cmd_pass(const char *arg, struct tcp_pcb *pcb, struct ftpd_msgstate *fsm)
{
    send_msg(pcb, fsm, msg230);
    fsm->state = FTPD_IDLE;
//		send_msg(pcb, fs, msgLoginFailed);
//		fs->state = FTPD_QUIT;
}

static void cmd_port(const char *arg, struct tcp_pcb *pcb, struct ftpd_msgstate *fsm)
{
    int nr;
    unsigned pHi, pLo;
    unsigned ip[4];

    nr = sscanf(arg, "%u,%u,%u,%u,%u,%u", &(ip[0]), &(ip[1]), &(ip[2]), &(ip[3]), &pHi, &pLo);
    if (nr != 6) {
        send_msg(pcb, fsm, msg501);
    } else {
        IP4_ADDR(&fsm->dataip, (u8_t) ip[0], (u8_t) ip[1], (u8_t) ip[2], (u8_t) ip[3]);
        fsm->dataport = ((u16_t) pHi << 8) | (u16_t) pLo;
        send_msg(pcb, fsm, msg200);
    }
}

static void cmd_quit(const char *arg, struct tcp_pcb *pcb, struct ftpd_msgstate *fsm)
{
    send_msg(pcb, fsm, msg221);
    fsm->state = FTPD_QUIT;
}
static void cmd_cwd(const char *arg, struct tcp_pcb *pcb, struct ftpd_msgstate *fsm)
{
	char *c=(char *)arg;
	int i=strlen(c);
	if(i>1 && c[i-1]=='/')
		c[i-1]='\0';
	if(strchr(c,':') && c[0]=='/')
		++c;																		//~~~
	
	if (!vfs_chdir(fsm->vfs, c)) {
		send_msg(pcb, fsm, msg250);
	} else {
		send_msg(pcb, fsm, msg550);
	}
}

static void cmd_pwd(const char *arg, struct tcp_pcb *pcb, struct ftpd_msgstate *fsm)
{
    char path[256];
    if( vfs_getcwd( fsm->vfs, path, sizeof(path) ) )
			send_msg(pcb, fsm, msg257PWD, path);	//~~~
}

static void cmd_size(const char *arg, struct tcp_pcb *pcb, struct ftpd_msgstate *fsm)
{
	vfs_stat_t st;
	if(vfs_stat(fsm->vfs, arg, &st) != 0)
		send_msg(pcb, fsm, msg550);
	else
		send_msg(pcb, fsm, msg213, st.st_size);
}

static void cmd_cdup(const char *arg, struct tcp_pcb *pcb, struct ftpd_msgstate *fsm)
{
    if (!vfs_chdir(fsm->vfs, "..")) {
        send_msg(pcb, fsm, msg250);
    } else {
        send_msg(pcb, fsm, msg550);
    }
}

static void cmd_list_common(const char *arg, struct tcp_pcb *pcb, struct ftpd_msgstate *fsm, int shortlist)
{
    vfs_dir_t *vfs_dir=NULL;
	
    char cwd[256];
	
		if(vfs_getcwd(fsm->vfs, cwd, sizeof(cwd)))
			vfs_dir = vfs_opendir(fsm->vfs, cwd);
    if (!vfs_dir) {
        send_msg(pcb, fsm, msg451);
        return;
    }
		
/* doesn't do anything in PASV !!! */
    if( open_dataconnection(pcb, fsm) != 0)
    {
        vfs_closedir(vfs_dir);
        return;
    }

    fsm->datafs->vfs_dir = vfs_dir;
    fsm->datafs->vfs_dirent = NULL;
    if (shortlist != 0)
    {
        fsm->state = FTPD_NLST;
    }
    else
    {
        fsm->state = FTPD_LIST;
    }
    send_msg(pcb, fsm, msg150);
}

static void cmd_nlst(const char *arg, struct tcp_pcb *pcb, struct ftpd_msgstate *fsm)
{
    cmd_list_common(arg, pcb, fsm, 1);
}

static void cmd_list(const char *arg, struct tcp_pcb *pcb, struct ftpd_msgstate *fsm)
{
    cmd_list_common(arg, pcb, fsm, 0);
}

static void cmd_retr(const char *arg, struct tcp_pcb *pcb, struct ftpd_msgstate *fsm)
{
    vfs_file_t *vfs_file;
    vfs_stat_t st;

    vfs_stat(fsm->vfs, arg, &st);
    //if (!VFS_ISREG(st.st_mode))
    //{
    //    send_msg(pcb, fsm, msg550);
    //    return;
    //}

    vfs_file = vfs_open(fsm->vfs, arg, "rb");
    if (!vfs_file) {
        send_msg(pcb, fsm, msg550);
        return;
    }

    send_msg(pcb, fsm, msg150recv, arg, st.st_size);

    if (open_dataconnection(pcb, fsm) != 0) {
        vfs_close(vfs_file);
        return;
    }

    fsm->datafs->vfs_file = vfs_file;
    fsm->state = FTPD_RETR;
}

static void cmd_stor(const char *arg, struct tcp_pcb *pcb, struct ftpd_msgstate *fsm)
{
    vfs_file_t *vfs_file;

    vfs_file = vfs_open(fsm->vfs, arg, "wb");
    if (!vfs_file) {
        send_msg(pcb, fsm, msg550);
        return;
    }

    send_msg(pcb, fsm, msg150stor, arg);

    if (open_dataconnection(pcb, fsm) != 0) {
        vfs_close(vfs_file);
        return;
    }

    fsm->datafs->vfs_file = vfs_file;
    fsm->state = FTPD_STOR;
}

static void cmd_noop(const char *arg, struct tcp_pcb *pcb, struct ftpd_msgstate *fsm)
{
    send_msg(pcb, fsm, msg200);
}

static void cmd_syst(const char *arg, struct tcp_pcb *pcb, struct ftpd_msgstate *fsm)
{
    send_msg(pcb, fsm, msg214SYST, "UNIX");
}

static void cmd_pasv(const char *arg, struct tcp_pcb *pcb, struct ftpd_msgstate *fsm)
{
    static u16_t port = 4096;
    static u16_t start_port = 4096;
    struct tcp_pcb *temppcb;

    /* Allocate memory for the structure that holds the state of the        connection. */
    fsm->datafs = (struct ftpd_datastate*)mem_malloc(sizeof(struct ftpd_datastate));

    if (fsm->datafs == NULL) {
        send_msg(pcb, fsm, msg451);
        return;
    }
    memset(fsm->datafs, 0, sizeof(struct ftpd_datastate));

    fsm->datapcb = tcp_new();
    if (!fsm->datapcb) {
        mem_free(fsm->datafs);
        send_msg(pcb, fsm, msg451);
        return;
    }
		start_port = port;
    while (1) {
        err_t err;
			
        if(++port > 0x7fff)
            port = 4096;
    
        fsm->dataport = port;
        err = tcp_bind(fsm->datapcb, &pcb->local_ip, fsm->dataport);
        if (err == ERR_OK)
            break;
        if (start_port == port)
            err = ERR_CLSD;
        if (err == ERR_USE)
            continue;
        if (err != ERR_OK) {
            ftpd_dataclose(fsm->datapcb, fsm->datafs);
            fsm->datapcb = NULL;
            fsm->datafs = NULL;
            return;
        }
    }

    temppcb = tcp_listen(fsm->datapcb);
    if (!temppcb) {
        ftpd_dataclose(fsm->datapcb, fsm->datafs);
        fsm->datapcb = NULL;
        fsm->datafs = NULL;
        return;
    }
    fsm->datapcb = temppcb;

    fsm->passive = 1;
    fsm->datafs->state = _CLOSED;
    fsm->datafs->msgfs = fsm;
    fsm->datafs->msgpcb = pcb;

    /* Tell TCP that this is the structure we wish to be passed for our
       callbacks. */
    tcp_arg(fsm->datapcb, fsm->datafs);
    tcp_accept(fsm->datapcb, ftpd_dataaccept);
    send_msg(pcb, fsm, msg227, ip4_addr1(&pcb->local_ip), ip4_addr2(&pcb->local_ip), ip4_addr3(&pcb->local_ip), ip4_addr4(&pcb->local_ip), (fsm->dataport >> 8) & 0xff, (fsm->dataport) & 0xff);
 //   dbg_printf("\r\ndatapcb=%08X, dataarg=%08X\r\n", fsm->datapcb,fsm->datafs);

}

static void cmd_abrt(const char *arg, struct tcp_pcb *pcb, struct ftpd_msgstate *fsm)
{
    if (fsm->datafs != NULL) {
        tcp_arg(fsm->datapcb, NULL);
        tcp_sent(fsm->datapcb, NULL);
        tcp_recv(fsm->datapcb, NULL);
        tcp_arg(fsm->datapcb, NULL);
        tcp_abort(pcb);
				if(fsm->datafs->p)
					pbuf_free(fsm->datafs->p);
        mem_free(fsm->datafs);
        fsm->datafs = NULL;
    }
    fsm->state = FTPD_IDLE;
}

static void cmd_type(const char *arg, struct tcp_pcb *pcb, struct ftpd_msgstate *fsm)
{
    dbg_printf("Got TYPE -%s-\n", arg);
    if( *arg == 'A' )
    {
        send_msg(pcb, fsm, "200 Type set to A." );
		return;
    }
    if( *arg == 'I' )
    {
        send_msg(pcb, fsm, "200 Type set to I." );
		return;
    }
	
	send_msg(pcb, fsm, msg502);
}

static void cmd_mode(const char *arg, struct tcp_pcb *pcb, struct ftpd_msgstate *fsm)
{
    dbg_printf("Got MODE -%s-\n", arg);
    send_msg(pcb, fsm, msg502);
}

static void cmd_rnfr(const char *arg, struct tcp_pcb *pcb, struct ftpd_msgstate *fsm)
{
    if (arg == NULL) {
        send_msg(pcb, fsm, msg501);
        return;
    }
    if (*arg == '\0') {
        send_msg(pcb, fsm, msg501);
        return;
    }
    if (fsm->renamefrom)
        mem_free(fsm->renamefrom);
    fsm->renamefrom = (char*)mem_malloc(strlen(arg) + 1);
    if (fsm->renamefrom == NULL) {
        send_msg(pcb, fsm, msg451);
        return;
    }
    strcpy(fsm->renamefrom, arg);
    fsm->state = FTPD_RNFR;
    send_msg(pcb, fsm, msg350);
}

static void cmd_rnto(const char *arg, struct tcp_pcb *pcb, struct ftpd_msgstate *fsm)
{
    if (fsm->state != FTPD_RNFR) {
        send_msg(pcb, fsm, msg503);
        return;
    }
    fsm->state = FTPD_IDLE;
    if (arg == NULL) {
        send_msg(pcb, fsm, msg501);
        return;
    }
    if (*arg == '\0') {
        send_msg(pcb, fsm, msg501);
        return;
    }
    if (vfs_rename(fsm->vfs, fsm->renamefrom, arg)) {
        send_msg(pcb, fsm, msg450);
    } else {
        send_msg(pcb, fsm, msg250);
    }
}

static void cmd_mkd(const char *arg, struct tcp_pcb *pcb, struct ftpd_msgstate *fsm)
{
    if (arg == NULL) {
        send_msg(pcb, fsm, msg501);
        return;
    }
    if (*arg == '\0') {
        send_msg(pcb, fsm, msg501);
        return;
    }
    if (vfs_mkdir(fsm->vfs, arg, VFS_IRWXU | VFS_IRWXG | VFS_IRWXO) != 0) {
        send_msg(pcb, fsm, msg550);
    } else {
        send_msg(pcb, fsm, msg257, arg);
    }
}

static void cmd_rmd(const char *arg, struct tcp_pcb *pcb, struct ftpd_msgstate *fsm)
{
    vfs_stat_t st;

    if (arg == NULL) {
        send_msg(pcb, fsm, msg501);
        return;
    }
    if (*arg == '\0') {
        send_msg(pcb, fsm, msg501);
        return;
    }
    if (vfs_stat(fsm->vfs, arg, &st) != 0) {
        send_msg(pcb, fsm, msg550);
        return;
    }
    if (!VFS_ISDIR(st.st_mode)) {
        send_msg(pcb, fsm, msg550);
        return;
    }
    if (vfs_rmdir(fsm->vfs, arg) != 0) {
        send_msg(pcb, fsm, msg550);
    } else {
        send_msg(pcb, fsm, msg250);
    }
}

static void cmd_dele(const char *arg, struct tcp_pcb *pcb, struct ftpd_msgstate *fsm)
{
    vfs_stat_t st;

    if (arg == NULL) {
        send_msg(pcb, fsm, msg501);
        return;
    }
    if (*arg == '\0') {
        send_msg(pcb, fsm, msg501);
        return;
    }
	
	if (vfs_stat(fsm->vfs, arg, &st) != 0)
    {
        send_msg(pcb, fsm, msg550);
        return;
    }
    //if (!VFS_ISREG(st.st_mode))
    //{
    //    send_msg(pcb, fsm, msg550);
    //    return;
    //}

    if (vfs_remove(fsm->vfs, arg) != 0)
    {
        send_msg(pcb, fsm, msg550);
    }
    else
    {
        send_msg(pcb, fsm, msg250);
    }
}


struct 			ftpd_command {
    char *cmd;
    void (*func) (const char *arg, struct tcp_pcb * pcb, struct ftpd_msgstate * fsm);
};

static struct ftpd_command ftpd_commands[] = {
    "USER", cmd_user,
    "PASS", cmd_pass,
    "PORT", cmd_port,
    "QUIT", cmd_quit,
    "CWD",  cmd_cwd,
    "CDUP", cmd_cdup,
    "PWD",  cmd_pwd,
    "XPWD", cmd_pwd,
    "NLST", cmd_nlst,
    "LIST", cmd_list,
    "RETR", cmd_retr,
    "STOR", cmd_stor,
    "NOOP", cmd_noop,
    "SYST", cmd_syst,
    "ABOR", cmd_abrt,
    "TYPE", cmd_type,
    "MODE", cmd_mode,
    "RNFR", cmd_rnfr,
    "RNTO", cmd_rnto,
    "MKD",  cmd_mkd,
    "XMKD", cmd_mkd,
    "RMD",  cmd_rmd,
    "XRMD", cmd_rmd,
    "DELE", cmd_dele,
		"SIZE", cmd_size,
    "PASV", cmd_pasv,
    NULL
};
static void send_msgdata(struct tcp_pcb *tpcb, struct ftpd_msgstate *es)
{
		struct pbuf *ptr;
		err_t wr_err = ERR_OK;	 
		if(tpcb->snd_queuelen > TCP_SND_QUEUELEN-1)	// !!!! pred vpisom preveri, ce ni queue ze poln  
			tcp_output(tpcb);			
		while ((wr_err == ERR_OK) &&
					 (es->p != NULL) && 
					 (es->p->len <= tcp_sndbuf(tpcb)))
		{
			ptr=es->p;																/* get pointer on pbuf from es structure & enqueue data for transmission */
			wr_err=tcp_write(tpcb,ptr->payload,ptr->len, 1);
			if (wr_err == ERR_OK) {
				es->p = ptr->next;
				if (es->p != NULL) {
					pbuf_ref(es->p); 											/* increment reference count for es->p */
				}
				pbuf_free(ptr);													/* free pbuf: will free pbufs up to es->p (because es->p has a reference count > 0) */
			} else if(wr_err == ERR_MEM) { 						/* we are low on memory, try later / harder, defer to poll */
				es->p = ptr;
		 } else { }																	/* other problem ?? */
		}
}


static void send_msg(struct tcp_pcb *pcb, struct ftpd_msgstate *fsm, char *msg, ...)
{
    va_list arg;
    char buffer[256];
		struct pbuf *ptr;
    int len;

    va_start(arg, msg);
    vsprintf(buffer, msg, arg);
    va_end(arg);
    strcat(buffer, "\r\n");
    len = strlen(buffer);
		ptr = pbuf_alloc(PBUF_TRANSPORT, len , PBUF_POOL);
		if(!ptr) {
			send_msgdata(pcb, fsm);
			return;
		}
		if(fsm->p)
			pbuf_chain(fsm->p,ptr);
		else
			fsm->p=ptr;
		pbuf_take(fsm->p,buffer,len);
    dbg_printf("response: %s", buffer);
    send_msgdata(pcb, fsm);
}

static void ftpd_msgerr(void *arg, err_t err)
{
    struct ftpd_msgstate *fsm = (struct ftpd_msgstate*)arg;
	
		dbg_printf("ftpd_msgerr: %s (%i)\n", lwip_strerr(err), err);
    if (fsm == NULL)
        return;
    if (fsm->datafs)
        ftpd_dataclose(fsm->datapcb, fsm->datafs);
    mem_free(fsm->p);
    if (fsm->renamefrom)
        mem_free(fsm->renamefrom);
//    vfs_close( (vfs_file_t*)fsm->vfs);
    mem_free(fsm);
}

static void ftpd_msgclose(struct tcp_pcb *pcb, struct ftpd_msgstate *fsm)
{
    tcp_arg(pcb, NULL);
    tcp_sent(pcb, NULL);
    tcp_recv(pcb, NULL);
    if (fsm->datafs)
			ftpd_dataclose(fsm->datapcb, fsm->datafs);
//    vfs_close( (vfs_file_t*)fsm->vfs);
    mem_free(fsm->p);
		if (fsm->renamefrom)
			mem_free(fsm->renamefrom);
    mem_free(fsm);
    tcp_arg(pcb, NULL);
    tcp_close(pcb);
}

static err_t ftpd_msgsent(void *arg, struct tcp_pcb *pcb, u16_t len)
{
    struct ftpd_msgstate *fsm = (struct ftpd_msgstate*)arg;
	
		if (pcb->state > ESTABLISHED)
        return ERR_OK;

    if (!fsm->p && (fsm->state == FTPD_QUIT))
        ftpd_msgclose(pcb, fsm);

    send_msgdata(pcb, fsm);

    return ERR_OK;
}


void bcopy (char *src, char *dest, int len)
{
  if (dest < src)
    while (len--)
      *dest++ = *src++;
  else
    {
      char *lasts = src + (len-1);
      char *lastd = dest + (len-1);
      while (len--)
        *(char *)lastd-- = *(char *)lasts--;
    }
}

static err_t ftpd_msgrecv(void *arg, struct tcp_pcb *pcb, struct pbuf *p, err_t err)
{
    char *text;
    struct ftpd_msgstate *fsm = (struct ftpd_msgstate*)arg;
			if (err == ERR_OK && p != NULL) {

				tcp_recved(pcb, p->tot_len);

        text = (char*)mem_malloc(p->tot_len + 1);
        if (text == NULL) {
					dbg_printf("ftpd_msgaccept: Out of memory\n");
 					pbuf_free(p);
					return ERR_MEM;
				} else {
            char cmd[5];
            struct pbuf *q;
            char *pt = text;
            struct ftpd_command *ftpd_cmd;

            for (q = p; q != NULL; q = q->next)
            {
                bcopy( (char*)q->payload, pt, q->len);
                pt += q->len;
            }
            *pt = '\0';

            pt = &text[strlen(text) - 1];
            while (((*pt == '\r') || (*pt == '\n')) && pt >= text)
                *pt-- = '\0';

            dbg_printf("query: %s\n", text);
            strncpy(cmd, text, 4);
            for (pt = cmd; pt < &cmd[4] && isalpha(*pt); pt++)
                *pt = toupper(*pt);
            *pt = '\0';

            for (ftpd_cmd = ftpd_commands; ftpd_cmd->cmd != NULL; ftpd_cmd++) {
                if (!strcmp(ftpd_cmd->cmd, cmd))
                    break;
            }

            if (strlen(text) < (strlen(cmd) + 1))
                pt = "";
            else
                pt = &text[strlen(cmd) + 1];

            if (ftpd_cmd->func)
                ftpd_cmd->func(pt, pcb, fsm);
            else
                send_msg(pcb, fsm, msg502);

            mem_free(text);
        } 
        pbuf_free(p);
    }

    return ERR_OK;
}

static err_t ftpd_msgpoll(void *arg, struct tcp_pcb *pcb)
{
    struct ftpd_msgstate *fsm = (struct ftpd_msgstate*)arg;

    if (fsm == NULL)
        return ERR_OK;

    if (fsm->datafs) {
        if (fsm->datafs->state==_CONNECTED) {
            switch (fsm->state) {
            case FTPD_LIST:
                send_next_directory(fsm->datafs, fsm->datapcb, 0);
                break;
            case FTPD_NLST:
                send_next_directory(fsm->datafs, fsm->datapcb, 1);
                break;
            case FTPD_RETR:
                send_file(fsm->datafs, fsm->datapcb);
                break;
            default:
                break;
            }
        }
    }
    return ERR_OK;
}

static err_t ftpd_msgaccept(void *arg, struct tcp_pcb *pcb, err_t err)
{
/* Allocate memory for the structure that holds the state of the connection. */
struct ftpd_msgstate *fsm = (struct ftpd_msgstate*)mem_malloc(sizeof(struct ftpd_msgstate));
		if (fsm == NULL) {
        dbg_printf("ftpd_msgaccept: Out of memory\n");
        return ERR_MEM;
    }
    memset(fsm, 0, sizeof(struct ftpd_msgstate));
    /* Initialize the structure. */
    fsm->state = FTPD_IDLE;
    fsm->vfs = vfs_openfs();

    /* Tell TCP that this is the structure we wish to be passed for our
       callbacks. */
    tcp_arg( pcb, fsm );

    /* Tell TCP that we wish to be informed of incoming data by a call
       to the http_recv() function. */
    tcp_recv( pcb, ftpd_msgrecv );

    /* Tell TCP that we wish be to informed of data that has been
       successfully sent by a call to the ftpd_sent() function. */
    tcp_sent( pcb, ftpd_msgsent );
    tcp_err( pcb, ftpd_msgerr );
    tcp_poll(pcb, ftpd_msgpoll, 1);
    send_msg(pcb, fsm, msg220);
    return ERR_OK;
}


void ftpd_init(void)
{
    struct tcp_pcb *pcb;
    pcb = tcp_new();
    tcp_bind( pcb, IP_ADDR_ANY, 21 );
    pcb = tcp_listen( pcb );
    tcp_accept( pcb, ftpd_msgaccept );
}



