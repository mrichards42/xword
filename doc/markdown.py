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

def convert(t):
    return markdown.markdown(t, ['footnotes', 'headerid', 'tables'])


prefix = """<html>
<head>
<link rel="stylesheet" type="text/css" href="styles.css">
<title>%s</title>
</head>
<body>
"""

suffix = """
</body>
</html>
"""

links = convert("""[Index](index.html) |
[Back](javascript: history.go(-1);) |
[Sourceforge](https://sourceforge.net/projects/wx-xword/) |
[Download](http://sourceforge.net/projects/wx-xword/files/Binary/)""")

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
            # Read the first line and make it the title
            title = input.readline().strip()
            text = '\n'.join((title, input.read()))
            output.write('\n'.join((
                (prefix % title),
                links + "<hr />",
                convert(text),
                "<hr />" + links,
                suffix,
            )))

#raw_input("Press any key to continue...")
