/*****************************************************************************

proc_info.h

Process Information Interface

*****************************************************************************/

#ifndef _PROC_INFO_H
#define _PROC_INFO_H

/*----------------------------------------------------------------------------
Includes
----------------------------------------------------------------------------*/

#include <windows.h>
#include <tchar.h>
#include <winternl.h>

#include "error_types.h"

/*----------------------------------------------------------------------------
Macros
----------------------------------------------------------------------------*/

/*----------------------------------------------------------------------------
Types and Structures
----------------------------------------------------------------------------*/

typedef NTSTATUS ( NTAPI *proc_NtQueryInformationProcess_fun ) (
                                    /* pointer to NT query API function     */
    IN  HANDLE          ProcessHandle,
                                    /* target process handle                */
    IN  PROCESSINFOCLASS
                        ProcessInformationClass,
                                    /* information to query                 */
    OUT PVOID           ProcessInformation,
                                    /* returned information                 */
    IN  ULONG           ProcessInformationLength,
                                    /* information buffer length            */
    OUT PULONG          ReturnLength                OPTIONAL
                                    /* length of returned information       */
);                                  /* returns status of query              */

typedef struct proc_command_s {     /* process command type                 */
    LPTSTR              image;      /* path to process' running image       */
    DWORD               count;      /* number of command strings            */
    LPTSTR              values[];   /* list of command strings              */
} proc_command_type;

typedef struct proc_instance_s {    /* process interface instance type      */
    proc_NtQueryInformationProcess_fun
                        nqip;       /* pointer to NT query API function     */
} proc_instance_type;

typedef struct proc_info_s {        /* process information type             */
    proc_instance_type* instance;   /* owner interface object               */
    DWORD               id;         /* process ID                           */
    HANDLE              handle;     /* process handle                       */
    HANDLE              token;      /* process token handle                 */
    PTOKEN_USER         user;       /* user information process token       */
    proc_command_type*  command;    /* process start command                */
} proc_info_type;

/*----------------------------------------------------------------------------
Memory Constants
----------------------------------------------------------------------------*/

/*----------------------------------------------------------------------------
Interface Prototypes
----------------------------------------------------------------------------*/

void proc_close(                    /* close query interface to process     */
    proc_info_type*     info        /* process information object           */
);

error_type proc_get_command(        /* get the command used to start proc.  */
    proc_info_type*     info,       /* process information object           */
    proc_command_type*  command     /* user's command object memory         */
);                                  /* returns error code                   */

error_type proc_get_user_sid(       /* get the process' user SID            */
    proc_info_type*     info,       /* process information object           */
    PSID                sid         /* pointer to destination SID variable  */
);                                  /* returns error code                   */

error_type proc_init(               /* initialize and interface instance    */
    proc_instance_type* instance    /* process interface instance object    */
);                                  /* returns error code                   */

error_type proc_open(               /* open query interface to process      */
    proc_instance_type* instance,   /* process information instance         */
    proc_info_type*     info,       /* process information object           */
    DWORD               id          /* process ID to open                   */
);                                  /* returns error code                   */

#endif  /* _PROC_INFO_H */

