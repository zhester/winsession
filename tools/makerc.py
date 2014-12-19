#!/usr/bin/env python


"""
Builds a Windows Resource Control Script
"""


import datetime
import getpass
import os


__version__ = '0.0.0'


#=============================================================================
default_template = """
/****************************************************************************

Windows Resource Control Script for "{project}"

List of available fields in templates:
{fields}

****************************************************************************/

//Set the program's application icon.
// e.g. a ICON "{project}.ico"
{icon_resource}

//Declare embedded executable information.
1 VERSIONINFO

FILEVERSION     0,0,0,0
PRODUCTVERSION  {version_commas},0

BEGIN

  BLOCK "StringFileInfo"
  BEGIN
    BLOCK "040904E4"
    BEGIN
      VALUE "CompanyName", "{author}"
      VALUE "FileDescription", "{title}"
      VALUE "FileVersion", "0.0.0"
      VALUE "InternalName", "{name}"
      VALUE "LegalCopyright", "{year} {author}"
      VALUE "OriginalFilename", "{project}.exe"
      VALUE "ProductName", "{project}"
      VALUE "ProductVersion", "{version}"
    END
  END

  BLOCK "VarFileInfo"
  BEGIN
    VALUE "Translation", 0x409, 1252
  END

END
"""


#=============================================================================
def makerc( target = None, template = None, fields = None ):
    """
    Creates a new Windows resource control script.
    """

    # set a default project name
    project = 'project'

    # check for an unspecified target file
    if target is None:
        target = project + '.rc'

    # get today's date
    today = datetime.date.today()

    # set default field values
    _fields = {
        'project' : project,
        'title'   : project,
        'name'    : project,
        'author'  : getpass.getuser(),
        'version' : '0.0.0',
        'date'    : today.strftime( '%Y-%m-%d' ),
        'year'    : today.strftime( '%Y' ),
        'icon'    : project + '.ico'
    }

    # override defaults with user-supplied information
    if fields is not None:
        for k, v in fields.items():
            _fields[ k ] = v

    # version string with commas
    _fields[ 'version_commas' ] = _fields[ 'version' ].replace( '.', ',' )

    # check for an icon
    if os.path.isfile( _fields[ 'icon' ] ) == True:

        # set the appropriate icon resource target
        _fields[ 'icon_resource' ] = 'a ICON "{}"'.format( _fields[ 'icon' ] )

    # no icon easily found
    else:

        # set a comment in the script
        _fields[ 'icon_resource' ] = '// ### no icon found at {} ###'.format(
            _fields[ 'icon' ]
        )

    # provide the list of fields for would-be template writers
    max_key = max( len( k ) for k in _fields.keys() )
    field_format = '{{0:<{}}} : {{1}}'.format( max_key )
    _fields[ 'fields' ] = '\n'.join(
        field_format.format( k, v ) for k,v in _fields.items()
    )

    # see if the user supplied a script template
    if template is not None:
        if 'read' in template:
            _template = template.read()
        else:
            _template = template
    else:
        _template = default_template

    # format and write the template to the target file
    with open( target, 'w' ) as tfh:
        tfh.write( _template.format( **_fields ) )

    # return success
    return True


#=============================================================================
def main( argv ):
    """
    Script execution entry point
    @param argv         Arguments passed to the script
    @return             Exit code (0 = success)
    """

    # imports when using this as a script
    import argparse

    # create and configure an argument parser
    parser = argparse.ArgumentParser(
        description = 'Builds a Windows Resource Control Script',
        add_help    = False
    )
    parser.add_argument(
        '-a',
        '--author',
        default = 'Zac Hester',
        help    = 'Specify program author name.'
    )
    parser.add_argument(
        '-h',
        '--help',
        default = False,
        help    = 'Display this help message and exit.',
        action  = 'help'
    )
    parser.add_argument(
        '-p',
        '--project',
        default = None,
        help    = 'Specify project name.'
    )
    parser.add_argument(
        '-r',
        '--revision',
        default = '0.0.0',
        help    = 'Specify program revision (version).'
    )
    parser.add_argument(
        '-v',
        '--version',
        default = False,
        help    = 'Display script version and exit.',
        action  = 'version',
        version = __version__
    )
    parser.add_argument(
        'target',
        nargs   = '?',
        default = None,
        help    = 'Path to output file.'
    )

    # parse the arguments
    args = parser.parse_args( argv[ 1 : ] )


    # read fields from the command line
    fields = { 'author' : args.author, 'version' : args.revision }

    # look for a project name (overrides a few things)
    if args.project is not None:
        project = args.project

    # otherwise, make a sane default
    else:
        project = os.path.basename( os.getcwd() )

    # set the fields that use the project name
    fields[ 'project' ] = project
    fields[ 'title' ]   = project
    fields[ 'name' ]    = project
    fields[ 'icon' ]    = project + '.ico'

    # build the resource control script
    result = makerc( target = args.target, fields = fields )

    # return result of function
    return os.EX_OK if result == True else os.EX_SOFTWARE


#=============================================================================
if __name__ == "__main__":
    import sys
    sys.exit( main( sys.argv ) )


