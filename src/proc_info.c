/*****************************************************************************

proc_info.c

Process Information Query Interface

This module normalizes the many and varied ways of retrieving information from
Windows processes.

*****************************************************************************/

/*----------------------------------------------------------------------------
Includes
----------------------------------------------------------------------------*/

#include <windows.h>
#include <tchar.h>

#include "proc_info.h"

/*----------------------------------------------------------------------------
Macros
----------------------------------------------------------------------------*/

/*----------------------------------------------------------------------------
Types and Structures
----------------------------------------------------------------------------*/

/*----------------------------------------------------------------------------
Memory Constants
----------------------------------------------------------------------------*/

/*----------------------------------------------------------------------------
Module Variables
----------------------------------------------------------------------------*/

/*----------------------------------------------------------------------------
Module Prototypes
----------------------------------------------------------------------------*/

void enable_process_debugging(      /* attempt to enable debugging features */
    void
);

/*----------------------------------------------------------------------------
Implementation
----------------------------------------------------------------------------*/

/*==========================================================================*/
void proc_close(                    /* close query interface to process     */
    proc_info_type*     info        /* process information object           */
) {

    /*------------------------------------------------------------------
    Check interface usage.
    ------------------------------------------------------------------*/
    if( info == NULL ) {
        return;
    }

    /*------------------------------------------------------------------
    Check for allocated user information.
    ------------------------------------------------------------------*/
    if( info->user != NULL ) {
        HeapFree( GetProcessHeap(), 0, ( LPVOID ) info->user );
        info->user = NULL;
    }

    /*------------------------------------------------------------------
    Check for open process token.
    ------------------------------------------------------------------*/
    if( info->token != NULL ) {
        CloseHandle( info->token );
        info->token = NULL;
    }

    /*------------------------------------------------------------------
    Check for open process handle.
    ------------------------------------------------------------------*/
    if( info->handle != NULL ) {
        CloseHandle( info->handle );
        info->handle = NULL;
    }

    /*------------------------------------------------------------------
    Clear the instance pointer and the ID value.
    ------------------------------------------------------------------*/
    info->instance = NULL;
    info->id       = 0;

}


/*==========================================================================*/
error_type proc_get_command(        /* get the command used to start proc.  */
    proc_info_type*     info,       /* process information object           */
    proc_command_type*  command     /* user's command object memory         */
) {                                 /* returns error code                   */
    //// ZIH - implement me
    return ERR_OK;
}


/*==========================================================================*/
error_type proc_get_user_sid(       /* get the process' user SID            */
    proc_info_type*     info,       /* process information object           */
    PSID                sid         /* pointer to destination SID variable  */
) {                                 /* returns error code                   */

    /*------------------------------------------------------------------
    Local Variables
    ------------------------------------------------------------------*/
    DWORD               return_length;
                                    /* data length return variable          */
    BOOL                wresult;    /* result of Windows API calls          */

    /*------------------------------------------------------------------
    Check interface usage.
    ------------------------------------------------------------------*/
    if( ( info == NULL ) || ( sid == NULL ) ) {
        return ERR_USAGE;
    }

    /*------------------------------------------------------------------
    See if we need to lazy-load the user info.
    ------------------------------------------------------------------*/
    if( info->user == NULL ) {

        /*--------------------------------------------------------------
        Perform a dummy query to fetch the size of the user info data.
        --------------------------------------------------------------*/
        return_length = 0;
        wresult = GetTokenInformation(
            info->token,
            TokenUser,
            ( LPVOID ) info->user,
            0,
            &return_length
        );
        if( ( wresult == FALSE )
        && ( GetLastError() != ERROR_INSUFFICIENT_BUFFER ) ) {
            return ERR_WINAPI;
        }

        /*------------------------------------------------------------------
        Allocate heap space for the user info data.
        ------------------------------------------------------------------*/
        info->user = ( PTOKEN_USER ) HeapAlloc(
            GetProcessHeap(),
            HEAP_ZERO_MEMORY,
            return_length
        );
        if( info->user == NULL ) {
            return ERR_ALLOC;
        }

        /*--------------------------------------------------------------
        Perform the real query to retrieve the user information.
        --------------------------------------------------------------*/
        wresult = GetTokenInformation(
            info->token,
            TokenUser,
            ( LPVOID ) info->user,
            return_length,
            &return_length
        );
        if( wresult == FALSE ) {
            HeapFree( GetProcessHeap(), 0, ( LPVOID ) info->user );
            info->user = NULL;
            return ERR_WINAPI;
        }

    }

    /*------------------------------------------------------------------
    Load the process' SID into the user's memory.
    ------------------------------------------------------------------*/
    memcpy( sid, info->user->User.Sid, sizeof( SID ) );
    return ERR_OK;
}


/*==========================================================================*/
error_type proc_init(               /* initialize an interface instance     */
    proc_instance_type* instance    /* process interface instance object    */
) {                                 /* returns error code                   */

    /*------------------------------------------------------------------
    Local Variables
    ------------------------------------------------------------------*/
    HINSTANCE           ntdll;      /* link to NT dll library         */

    /*------------------------------------------------------------------
    Initialize local variables.
    ------------------------------------------------------------------*/
    ntdll = NULL;

    /*------------------------------------------------------------------
    Test for proper usage.
    ------------------------------------------------------------------*/
    if( instance == NULL ) {
        return ERR_USAGE;
    }

    /*------------------------------------------------------------------
    Dynamically link the "Ntdll.dll" library.
    ------------------------------------------------------------------*/
    ntdll = LoadLibrary( _T( "Ntdll.dll" ) );
    if( ntdll == NULL ) {
        return ERR_WINAPI;
    }

    /*------------------------------------------------------------------
    Retrieve the entry point for the query interface function.
    ------------------------------------------------------------------*/
    instance->nqip = ( proc_NtQueryInformationProcess_fun ) GetProcAddress(
        ntdll,
        _T( "NtQueryInformationProcess" )
    );
    if( instance->nqip == NULL ) {
        FreeLibrary( ntdll );
        return ERR_WINAPI;
    }

    /*------------------------------------------------------------------
    Release the handle to the dynamic library.
    ------------------------------------------------------------------*/
    FreeLibrary( ntdll );

    /*------------------------------------------------------------------
    Attempt to enable process debugging.
    ------------------------------------------------------------------*/
    enable_process_debugging();

    /*------------------------------------------------------------------
    Return status of initialization.
    ------------------------------------------------------------------*/
    return ERR_OK;
}


/*==========================================================================*/
error_type proc_open(               /* open query interface to process      */
    proc_instance_type* instance,   /* process information instance         */
    proc_info_type*     info,       /* process information object           */
    DWORD               id          /* process ID to open                   */
) {                                 /* returns error code                   */

    /*------------------------------------------------------------------
    Local Variables
    ------------------------------------------------------------------*/
    BOOL                wresult;    /* result of Windows API calls          */

    /*------------------------------------------------------------------
    Check interface usage.
    ------------------------------------------------------------------*/
    if( ( instance == NULL ) || ( info == NULL ) || ( id == 0 ) ) {
        return ERR_USAGE;
    }

    /*------------------------------------------------------------------
    Initialize user's memory.
    ------------------------------------------------------------------*/
    memset( info, 0, sizeof( proc_info_type ) );
    info->instance = instance;
    info->id       = id;

    /*------------------------------------------------------------------
    Open a handle to the requested process.
    ------------------------------------------------------------------*/
    info->handle = OpenProcess(
        ( PROCESS_QUERY_INFORMATION | PROCESS_VM_READ ),
        FALSE,
        info->id
    );
    if( info->handle == NULL ) {
        return ERR_WINAPI;
    }

    /*------------------------------------------------------------------
    Open a process query token to the requested process.
    ------------------------------------------------------------------*/
    wresult = OpenProcessToken(
        info->handle,
        TOKEN_QUERY,
        &( info->token )
    );
    if( ( wresult == FALSE ) || ( info->token == NULL ) ) {
        CloseHandle( info->handle );
        info->handle = NULL;
        return ERR_WINAPI;
    }

    /*------------------------------------------------------------------
    Process successfully opened.
    ------------------------------------------------------------------*/
    return ERR_OK;
}


/*==========================================================================*/
void enable_process_debugging(      /* attempt to enable debugging features */
    void
) {

    /*------------------------------------------------------------------
    Local Variables
    ------------------------------------------------------------------*/
    HANDLE              process_handle;
                                    /* handle to current process            */
    HANDLE              token_handle;
                                    /* handle to process token              */
    TOKEN_PRIVILEGES    token_privileges;
                                    /* token privilege object               */
    BOOL                wresult;    /* result of Windows API calls          */

    /*------------------------------------------------------------------
    Initialize local variables.
    ------------------------------------------------------------------*/
    memset( &token_privileges, 0, sizeof( token_privileges ) );

    /*------------------------------------------------------------------
    Retrieve the handle to the current process.
    ------------------------------------------------------------------*/
    process_handle = GetCurrentProcess();

    /*------------------------------------------------------------------
    Open a token to the current process.
    ------------------------------------------------------------------*/
    wresult = OpenProcessToken(
        process_handle,
        ( TOKEN_ADJUST_PRIVILEGES | TOKEN_QUERY ),
        &token_handle
    );
    if( wresult == FALSE ) {
        return;
    }

    /*------------------------------------------------------------------
    Retrieve privilege information for the current process.
    ------------------------------------------------------------------*/
    wresult = LookupPrivilegeValue(
        NULL,
        SE_DEBUG_NAME,
        &token_privileges.Privileges[ 0 ].Luid
    );
    if( wresult == FALSE ) {
        CloseHandle( token_handle );
        return;
    }

    /*------------------------------------------------------------------
    Set desired privilege details to enable additional features.
    ------------------------------------------------------------------*/
    token_privileges.PrivilegeCount = 1;
    token_privileges.Privileges[ 0 ].Attributes = SE_PRIVILEGE_ENABLED;

    /*------------------------------------------------------------------
    Attempt to allow the current process to retrieve debugging info.
    ------------------------------------------------------------------*/
    AdjustTokenPrivileges(
        token_handle,
        FALSE,
        &token_privileges,
        0,
        ( PTOKEN_PRIVILEGES ) NULL,
        0
    );
    //ZIH - leaving this here for documentation purposes
    //DWORD error = GetLastError();
    //if( error != ERROR_SUCCESS ) {
    //    CloseHandle( token_handle );
    //    return;
    //}

    /*------------------------------------------------------------------
    Release the handle to the token.
    ------------------------------------------------------------------*/
    CloseHandle( token_handle );
}

