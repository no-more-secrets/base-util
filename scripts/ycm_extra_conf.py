import os, json

files = json.loads( file( '.builds/current/compile_commands.json', 'r' ).read() )

flags = {}

for f in files:
    flags[str( f['file'] )] = str( f['command'] ).split()

def FlagsForFile( filename, **kwargs ):
    try:
        result = flags[filename]
    except:
        # Try to find a file in the same folder and use those
        for f,cmd in flags.iteritems():
            if os.path.dirname( f ) == os.path.dirname( filename ):
                result = cmd
                break
        else:
            result = []
    return { 'flags': result }
