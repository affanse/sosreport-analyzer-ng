/*
 *  cfg.c - configuration functions 
 *  This file contains the contents of sosreport-analyzer.
 *
 *  Copyright (C) 2018-2019 Shintaro Fujiwara
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License as published by the Free Software Foundation; either
 *  version 2.1 of the License, or (at your option) any later version.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA
 *  02110-1301 USA
 */

#include <unistd.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h> /* for is_space */
#include <dirent.h>
#include <stddef.h>
#include <errno.h>
#include "common.h"
#include "cfg.h"

/* should be set to NULL, this will be checked in cfg_init() */
struct sosreport_analyzer_config *sosreport_analyzer_cfg = NULL;

const char msg_cfg_read [ 36 ] = "cfg_read was called by SIG_VALUE !\n";
const char msg_cfg_read_ok [ 40 ] = "file value was reloaded by SIG_VALUE !\n";

void cfg_defaults ( struct sosreport_analyzer_config *cfg )
{
    memset ( cfg, 0, sizeof ( struct sosreport_analyzer_config ) );
    if ( cfg == NULL )
    {
        printf("cfg.c: cfg_defaults() failed to memset");
        exit(EXIT_FAILURE);
    }
}

void check_argumentcount ( const char *file_name, int lnr,
                               const char *keyword, int condition )
{
    if ( !condition )
        printf("%s:%d: %s: wrong number of arguments",
                file_name, lnr, keyword); 
}

char *get_token ( char **line, char *buf, size_t buflen )
{
    size_t len;

    if ( ( line == NULL ) || ( *line == NULL ) || ( **line == '\0' ) || ( buf == NULL ) )
        return NULL;

    /* find the beginning and length of the token */
    *line += strspn ( *line, TOKEN_DELIM );
    len = strcspn ( *line, TOKEN_DELIM );

    /* check if there is a token */
    if ( len == 0 )
    {
        *line = NULL ;
        return NULL;
    }

    /* limit the token length */
    if ( len >= buflen )
        len = buflen - 1;

    /* copy the token */
    strncpy ( buf, *line, len );
    buf [ len ] = '\0';

    /* skip to the next token */
    *line += len;
    *line += strspn ( *line, TOKEN_DELIM );

    /* return the token */
    return buf;
}

int set_member_to_struct ( const char *keyword, char *line, struct sosreport_analyzer_config *cfg )
{
    if ( strstr ( line, "skip" ) != NULL )
            printf("skip file '%s'.\n",keyword);
    else
    {
        if ( strcmp ( keyword, "date" ) == 0 )
            strncpy ( cfg->date, line, MAX_LINE_LENGTH - 1 );
        else if ( strcmp ( keyword, "lsb-release" ) == 0 )
            strncpy ( cfg->lsb_release, line, MAX_LINE_LENGTH - 1 );
        else if ( strcmp ( keyword, "uname" ) == 0 )
            strncpy ( cfg->uname, line, MAX_LINE_LENGTH - 1 );
        else if ( strcmp ( keyword, "hostname" ) == 0 )
            strncpy ( cfg->hostname, line, MAX_LINE_LENGTH - 1 );
        else if ( strcmp ( keyword, "uptime" ) == 0 )
            strncpy ( cfg->uptime, line, MAX_LINE_LENGTH - 1 );
        else if ( strcmp ( keyword, "root/anaconda-ks.cfg" ) == 0 )
            strncpy ( cfg->root_anaconda_ks_cfg, line, MAX_LINE_LENGTH - 1 );
        else if ( strcmp ( keyword, "dmidecode" ) == 0 )
            strncpy ( cfg->dmidecode, line, MAX_LINE_LENGTH - 1 );
        else if ( strcmp ( keyword, "lsmod" ) == 0 )
            strncpy ( cfg->lsmod, line, MAX_LINE_LENGTH - 1 );
        else if ( strcmp ( keyword, "lspci" ) == 0 )
            strncpy ( cfg->lspci, line, MAX_LINE_LENGTH - 1 );
        else if ( strcmp ( keyword, "sos_commands/scsi/lsscsi" ) == 0 )
            strncpy ( cfg->sos_commands_scsi_lsscsi, line, MAX_LINE_LENGTH - 1 );
        else if ( strcmp ( keyword, "installed-rpms" ) == 0 )
            strncpy ( cfg->installed_rpms, line, MAX_LINE_LENGTH - 1 );
        else if ( strcmp ( keyword, "df" ) == 0 )
            strncpy ( cfg->df, line, MAX_LINE_LENGTH - 1 );
        else if ( strcmp ( keyword, "vgdisplay" ) == 0 )
            strncpy ( cfg->vgdisplay, line, MAX_LINE_LENGTH - 1 );
        else if ( strcmp ( keyword, "free" ) == 0 )
            strncpy ( cfg->free, line, MAX_LINE_LENGTH - 1 );
        else if ( strcmp ( keyword, "ip_addr" ) == 0 )
            strncpy ( cfg->ip_addr, line, MAX_LINE_LENGTH - 1 );
        else if ( strcmp ( keyword, "route" ) == 0 )
            strncpy ( cfg->route, line, MAX_LINE_LENGTH - 1 );
        else if ( strcmp ( keyword, "last" ) == 0 )
            strncpy ( cfg->last, line, MAX_LINE_LENGTH - 1 );
        else if ( strcmp ( keyword, "ps" ) == 0 )
            strncpy ( cfg->ps, line, MAX_LINE_LENGTH - 1 );
        else if ( strcmp ( keyword, "lsof" ) == 0 )
            strncpy ( cfg->lsof, line, MAX_LINE_LENGTH - 1 );
        else if ( strcmp ( keyword, "netstat" ) == 0 )
            strncpy ( cfg->netstat, line, MAX_LINE_LENGTH - 1 );
        else if ( strcmp ( keyword, "etc/kdump.conf" ) == 0 )
            strncpy ( cfg->etc_kdump_conf, line, MAX_LINE_LENGTH - 1 );
        else if ( strcmp ( keyword, "etc/sysctl.conf" ) == 0 )
            strncpy ( cfg->etc_sysctl_conf, line, MAX_LINE_LENGTH - 1 );
        else if ( strcmp ( keyword, "proc/meminfo" ) == 0 )
            strncpy ( cfg->proc_meminfo, line, MAX_LINE_LENGTH - 1 );
        else if ( strcmp ( keyword, "proc/interrupts" ) == 0 )
            strncpy ( cfg->proc_interrupts, line, MAX_LINE_LENGTH - 1 );
        else if ( strcmp ( keyword, "proc/net/dev" ) == 0 )
            strncpy ( cfg->proc_net_dev, line, MAX_LINE_LENGTH - 1 );
        else if ( strcmp ( keyword, "proc/net/sockstat" ) == 0 )
            strncpy ( cfg->proc_net_sockstat, line, MAX_LINE_LENGTH - 1 );
        else if ( strcmp ( keyword, "etc/logrotate.conf" ) == 0 )
            strncpy ( cfg->etc_logrotate_conf, line, MAX_LINE_LENGTH - 1 );
        else if ( strcmp ( keyword, "etc/cron.d/" ) == 0 )
            strncpy ( cfg->etc_cron_d_, line, MAX_LINE_LENGTH - 1 );
        else if ( strcmp ( keyword, "var/log/messages" ) == 0 )
            strncpy ( cfg->var_log_messages, line, MAX_LINE_LENGTH - 1 );
        else if ( strcmp ( keyword, "sos_commands/kernel/sysctl_-a" ) == 0 )
            strncpy ( cfg->sos_commands_kernel_sysctl__a, line, MAX_LINE_LENGTH - 1 );
        else if ( strcmp ( keyword, "sos_commands/logs/journalctl_--no-pager" ) == 0 )
            strncpy ( cfg->sos_commands_logs_journalctl___no_pager, line, MAX_LINE_LENGTH - 1 );
        else if ( strcmp ( keyword, "sos_commands/networking/ethtool_-S" ) == 0 )
            strncpy ( cfg->sos_commands_networking_ethtool__S, line, MAX_LINE_LENGTH - 1 );
        else
            return ( 1 );
    }
    return ( 0 );
}

void cfg_read ( const char *file_name, struct sosreport_analyzer_config *cfg )
{
    FILE *fp;
    int fd, rc, lnr = 0;
    struct stat st;
    char linebuf [ MAX_LINE_LENGTH ];
    char *line;
    char keyword [ 1024 ];
    int i;

    /* open the file */
    rc = open ( file_name, O_NOFOLLOW | O_RDONLY );
    if ( rc < 0 ) {
        if ( errno != ENOENT )
        {
            printf("Error opening %s (%s)",
                file_name,strerror(errno));
	    printf("Config file %s doesn't exist.\n",file_name);
            exit ( EXIT_FAILURE );
	}
    }
    fd = rc;

    /* check the file's permissions: owned by root, not world writable,
     * not symlink.
     */
    char str_tmp2 [ MAX_FILE_NAME_LENGTH ]; 
    memset ( str_tmp2, '\0', sizeof ( str_tmp2 ) ); 
    snprintf ( str_tmp2, MAX_LINE_LENGTH, "Config file %s opened for parsing",file_name );
    append_list ( &sos_header_obj, str_tmp2 );
    append_list ( &sos_header_obj, "--------" );

    if ( fstat ( fd, &st ) < 0) {
        printf("Error fstat'ing %s (%s)\n",file_name,strerror(errno)); 
        close ( fd );
        exit ( EXIT_FAILURE );
    }
    if ( st.st_uid != 0 ) {
        printf("Error - %s isn't owned by root\n",file_name); 
        close ( fd );
        exit ( EXIT_FAILURE );
    }
    if ( ( st.st_mode & S_IWOTH ) == S_IWOTH ) {
        printf("Error - %s is world writable",file_name );
        close ( fd );
        exit ( EXIT_FAILURE );
    }
    /* using macro to check file*/
    if ( !S_ISREG ( st.st_mode ) ) {
        printf("Error - %s is not a regular file",file_name );
        close ( fd );
        exit ( EXIT_FAILURE );
    }
    /* open config file */
    if ( ( fp = fopen ( file_name,"r" ) ) == NULL )
    {
        printf("can't open config file (%s): %s", file_name, strerror ( errno ) );
        close ( fd );
        exit ( EXIT_FAILURE );
    }
    /* read file and parse lines */
    while ( fgets ( linebuf, sizeof ( linebuf ), fp ) != NULL )
    {
        lnr++;
        line = linebuf;
        /* strip newline */
        i = ( int ) strlen ( line );
        if ( ( i <= 0 ) || ( line [ i - 1 ] != '\n' ) )
        {
            printf("%s:%d: line too long or last line missing newline\n", file_name, lnr);
            close ( fd );
            exit ( EXIT_FAILURE );
        }
        line [ i - 1 ] = '\0';
        /* ignore comment lines */
        if ( line [ 0 ] == '#' )
            continue;
        /* strip trailing spaces */
        for ( i--; ( i > 0 ) && isspace ( line [ i - 1 ] ); i-- )
            line [ i - 1 ] = '\0';

        /* get keyword from line and ignore empty lines */
        if ( get_token ( &line, keyword, sizeof ( keyword ) ) == NULL )
            continue;
        /* runtime options */
        if ( set_member_to_struct ( keyword, line, cfg ) != 0 )
        /* fallthrough */
        {
            printf("%s:%d: unknown keyword: '%s'\n", file_name, lnr, keyword );
            close ( fd );
            exit ( EXIT_FAILURE );
        }
    }
    /* we're done reading file, close */
    fclose ( fp );

    /* appending member strigs with their items which had been set by config file */
    append_sos_header_obj ( "date", cfg );
    append_sos_header_obj ( "lsb-release", cfg );
    append_sos_header_obj ( "uname", cfg );
    append_sos_header_obj ( "hostname", cfg );
    append_sos_header_obj ( "uptime", cfg );
    append_sos_header_obj ( "root/anaconda-ks.cfg", cfg );
    append_sos_header_obj ( "dmidecode", cfg );
    append_sos_header_obj ( "lsmod", cfg );
    append_sos_header_obj ( "lspci", cfg );
    append_sos_header_obj ( "sos_commands/scsi/lsscsi", cfg );
    append_sos_header_obj ( "installed-rpms", cfg );
    append_sos_header_obj ( "df", cfg );
    append_sos_header_obj ( "vgdisplay", cfg );
    append_sos_header_obj ( "free", cfg );
    append_sos_header_obj ( "ip_addr", cfg );
    append_sos_header_obj ( "route", cfg );
    append_sos_header_obj ( "last", cfg );
    append_sos_header_obj ( "ps", cfg );
    append_sos_header_obj ( "lsof", cfg );
    append_sos_header_obj ( "netstat", cfg );
    append_sos_header_obj ( "etc/kdump.conf", cfg );
    append_sos_header_obj ( "etc/sysctl.conf", cfg );
    append_sos_header_obj ( "proc/meminfo", cfg );
    append_sos_header_obj ( "proc/interrupts", cfg );
    append_sos_header_obj ( "proc/net/dev", cfg );
    append_sos_header_obj ( "proc/net/sockstat", cfg );
    append_sos_header_obj ( "etc/logrotate.conf", cfg );
    append_sos_header_obj ( "etc/cron.d/", cfg );
    append_sos_header_obj ( "var/log/messages", cfg );
    append_sos_header_obj ( "sos_commands/kernel/sysctl_-a", cfg );
    append_sos_header_obj ( "sos_commands/logs/journalctl_--no-pager", cfg );
    append_sos_header_obj ( "sos_commands/networking/ethtool_-S", cfg );
    append_list ( &sos_header_obj, "--------" );
}

void append_sos_header_obj ( const char *member, struct sosreport_analyzer_config *cfg )
{
    char str_tmp [ MAX_FILE_NAME_LENGTH ]; 
    memset ( str_tmp, '\0', MAX_FILE_NAME_LENGTH ); 
    snprintf (str_tmp, strlen ( member ) + 2, "%s=", member );
    if ( strcmp ( member, "date" ) == 0 )
        strcat ( str_tmp, cfg->date );
    else if ( strcmp ( member, "lsb-release" ) == 0 )
        strcat ( str_tmp, cfg->lsb_release );
    else if ( strcmp ( member, "uname" ) == 0 )
        strcat ( str_tmp, cfg->uname );
    else if ( strcmp ( member, "hostname" ) == 0 )
        strcat ( str_tmp, cfg->hostname );
    else if ( strcmp ( member, "uptime" ) == 0 )
        strcat ( str_tmp, cfg->uptime );
    else if ( strcmp ( member, "root/anaconda-ks.cfg" ) == 0 )
        strcat ( str_tmp, cfg->root_anaconda_ks_cfg );
    else if ( strcmp ( member, "dmidecode" ) == 0 )
        strcat ( str_tmp, cfg->dmidecode );
    else if ( strcmp ( member, "lsmod" ) == 0 )
        strcat ( str_tmp, cfg->lsmod );
    else if ( strcmp ( member, "lspci" ) == 0 )
        strcat ( str_tmp, cfg->lspci );
    else if ( strcmp ( member, "sos_commands/scsi/lsscsi" ) == 0 )
        strcat ( str_tmp, cfg->sos_commands_scsi_lsscsi );
    else if ( strcmp ( member, "installed-rpms" ) == 0 )
        strcat ( str_tmp, cfg->installed_rpms );
    else if ( strcmp ( member, "df" ) == 0 )
        strcat ( str_tmp, cfg->df );
    else if ( strcmp ( member, "vgdisplay" ) == 0 )
        strcat ( str_tmp, cfg->vgdisplay );
    else if ( strcmp ( member, "free" ) == 0 )
        strcat ( str_tmp, cfg->free );
    else if ( strcmp ( member, "ip_addr" ) == 0 )
        strcat ( str_tmp, cfg->ip_addr );
    else if ( strcmp ( member, "route" ) == 0 )
        strcat ( str_tmp, cfg->route );
    else if ( strcmp ( member, "last" ) == 0 )
        strcat ( str_tmp, cfg->last );
    else if ( strcmp ( member, "ps" ) == 0 )
        strcat ( str_tmp, cfg->ps );
    else if ( strcmp ( member, "lsof" ) == 0 )
        strcat ( str_tmp, cfg->lsof );
    else if ( strcmp ( member, "netstat" ) == 0 )
        strcat ( str_tmp, cfg->netstat );
    else if ( strcmp ( member, "etc/kdump.conf" ) == 0 )
        strcat ( str_tmp, cfg->etc_kdump_conf );
    else if ( strcmp ( member, "etc/sysctl.conf" ) == 0 )
        strcat ( str_tmp, cfg->etc_sysctl_conf );
    else if ( strcmp ( member, "proc/meminfo" ) == 0 )
        strcat ( str_tmp, cfg->proc_meminfo );
    else if ( strcmp ( member, "proc/interrupts" ) == 0 )
        strcat ( str_tmp, cfg->proc_interrupts );
    else if ( strcmp ( member, "proc/net/dev" ) == 0 )
        strcat ( str_tmp, cfg->proc_net_dev );
    else if ( strcmp ( member, "proc/net/sockstat" ) == 0 )
        strcat ( str_tmp, cfg->proc_net_sockstat );
    else if ( strcmp ( member, "etc/logrotate.conf" ) == 0 )
        strcat ( str_tmp, cfg->etc_logrotate_conf );
    else if ( strcmp ( member, "etc/cron.d/" ) == 0 )
        strcat ( str_tmp, cfg->etc_cron_d_ );
    else if ( strcmp ( member, "var/log/messages" ) == 0 )
        strcat ( str_tmp, cfg->var_log_messages );
    else if ( strcmp ( member, "sos_commands/kernel/sysctl_-a" ) == 0 )
        strcat ( str_tmp, cfg->sos_commands_kernel_sysctl__a );
    else if ( strcmp ( member, "sos_commands/logs/journalctl_--no-pager" ) == 0 )
        strcat ( str_tmp, cfg->sos_commands_logs_journalctl___no_pager );
    else if ( strcmp ( member, "sos_commands/networking/ethtool_-S" ) == 0 )
        strcat ( str_tmp, cfg->sos_commands_networking_ethtool__S );
    append_list ( &sos_header_obj, str_tmp );
}

void cfg_init ( const char *file_name )
{
    /* check if we were called before */
    if ( sosreport_analyzer_cfg != NULL )
        printf("cfg_init() may only be called once.\n");
    /* allocate the memory (this memory is not freed anywhere, it's ok, outside this socpe, it's NULL) */
    sosreport_analyzer_cfg = (struct sosreport_analyzer_config * ) malloc ( sizeof ( struct sosreport_analyzer_config ) );
    if ( sosreport_analyzer_cfg == NULL )
        printf("malloc() failed to allocate memory.\n");
    /* clear configuration */
    cfg_defaults ( sosreport_analyzer_cfg );
    /* read configfile */
    cfg_read ( file_name, sosreport_analyzer_cfg );
}

void cfg_clear ( )
{
    /* printf("freeing the config pointer, because we have already read stuff -- sosreport_analyzer_cfg:%p\n",sosreport_analyzer_cfg); */
    free ( sosreport_analyzer_cfg );
    sosreport_analyzer_cfg = NULL;
}
