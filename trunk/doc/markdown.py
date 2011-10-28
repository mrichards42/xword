import sys, os
if sys.platform == 'win32':
        # We have to remove the Scripts dir from path on windows.
        # If we don't, it will try to import itself rather than markdown lib.
        # This appears to *not* be a problem on *nix systems, only Windows.
        try:
            sys.path.remove(os.path.dirname(__file__))
        except (ValueError, NameError):
            pass

import markdown


prefix = """<html>
<head>
<link rel="stylesheet" type="text/css" href="styles.css">
</head>
<body>
"""

suffix = """
</body>
</html>
"""

# Make html dir
if not os.path.exists('html'):
    os.mkdir('html')

# Convert files
for filename in os.listdir(os.getcwd()):
    if not filename.endswith(".md"):
        continue
    outfile = os.path.join('html', os.path.splitext(filename)[0] + '.html')
    print 'converting', filename
    with open(filename, 'r') as input:
        with open(outfile, 'w') as output:
            output.write(prefix + markdown.markdown(input.read(), ['footnotes', 'tables']) + suffix)

#raw_input("Press any key to continue...")
