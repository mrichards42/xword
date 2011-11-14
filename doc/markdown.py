#============================================================================#
# Configuration
#============================================================================#

prefix = """<html>
<head>
<link rel="stylesheet" type="text/css" href="styles.css">
<title>%s</title>
<script type="text/javascript">
// Highlight the currently focused heading (based on location.hash)
var hash;
function updateHash()
{
    var id = location.hash.substr(1);
    // Remove "hash" from the old element's className
    var oldEl = document.getElementById(hash);
    if (oldEl)
    {
        var className = oldEl.className;
        if (className.length > 4 &&
            className.substr(className.length - 5) == " hash")
        {
            oldEl.className = className.substr(0, className.length - 5);
        }
        else if (className == "hash")
            oldEl.className = "";
    }
    // Add "hash" to the new element's className
    var newEl = document.getElementById(id)
    if (newEl)
    {
        if (newEl.className.length != 0)
            newEl.className = newEl.className = " hash";
        else
            newEl.className = "hash";
    }
    hash = id;
}
</script>
</head>
<body>
"""

suffix = """
<script>
updateHash();
</script>
</body>
</html>
"""

links = """[Index](index.html) |
[Back](javascript: history.go(-1);) |
[Sourceforge](https://sourceforge.net/projects/wx-xword/) |
[Download](http://sourceforge.net/projects/wx-xword/files/Binary/)"""

no_ids = set(['license'])

contents_order = [
    'overview',
    'window',
    'solving',
    'navigation',
    'check',
    'diagramless',
    'layout',
    'preferences',
    'features',
    'acrosslite',
    'crosswordsolver',
    'license',
]


#============================================================================#
# Helpers
#============================================================================#

html_escape_table = {
    "&": "&amp;",
    '"': "&quot;",
    "'": "&apos;",
    ">": "&gt;",
    "<": "&lt;",
    }

def html_escape(text):
    """Produce entities within text."""
    return "".join(html_escape_table.get(c,c) for c in text)

def convert(t):
    return markdown.markdown(t, ['footnotes', 'tables'])

import sys, os, re
if sys.platform == 'win32':
        # We have to remove the Scripts dir from path on windows.
        # If we don't, it will try to import itself rather than markdown lib.
        # This appears to *not* be a problem on *nix systems, only Windows.
        try:
            sys.path.remove(os.path.dirname(__file__))
        except (ValueError, NameError):
            pass

import markdown

links = convert(links)

#============================================================================#
# The code
#============================================================================#

# Files for chms


class ContentsItem:
    def __init__(self, name, link, sublist = None):
        self.name = name
        self.link = link
        self.sublist = []
        self.parent = None
        for item in (sublist or []):
            if isinstance(item, (tuple, list)):
                item = ContentsItem(*item)
            self.sublist.append(item)

    def append(self, name, link, sublist = None):
        item = ContentsItem(name, link, sublist)
        item.parent = self
        self.sublist.append(item)

contents = {}
files = []

# Make html dir
if not os.path.exists('html'):
    os.mkdir('html')

# Convert files
for mdfile in os.listdir(os.getcwd()):
    if not mdfile.endswith(".md"):
        continue
    filename = os.path.splitext(mdfile)[0]
    htmlfile = filename + '.html'
    outfile = os.path.join('html', htmlfile)
    files.append(htmlfile)
    print 'converting', mdfile
    with open(mdfile, 'r') as input:
        with open(outfile, 'w') as output:
            # Read the first line and make it the title
            title = input.readline().strip()
            text = '\n'.join((title, input.read()))
            text = '\n'.join((
                (prefix % title),
                links + "<hr />",
                convert(text),
                "<hr />" + links,
                suffix,
            ))

            # Search the text for table of contents headings
            contents[filename] = ContentsItem(title, htmlfile)
            if filename not in contents_order:
                print filename + "not in contents"
            if filename not in no_ids:
                c = contents[filename]
                level = -1
                for h, headinghtml in re.findall(r'<h(\d)>(.*?)</h\1>', text):
                    # Extract the heading text from the html
                    heading = re.sub('<[^>]*>', '', headinghtml)
                    if heading == title:
                        continue
                    # Make an id
                    id = heading.replace(' ', '_').replace('"', '').replace("'", '').lower()
                    # Add the id in the html
                    text = text.replace(
                        '<h%s>%s</h%s>' % (h, headinghtml, h),
                        '<h%s id="%s">%s</h%s>' % (h, id, headinghtml, h)
                    )

                    # Figure out the sublevel of this heading
                    h = int(h)
                    if level > h:
                        if c.parent:
                            c = c.parent
                    elif level < h and level != -1:
                        if c.sublist:
                            c = c.sublist[-1]
                    level = h
                    # Write the contents item
                    c.append(html_escape(heading), htmlfile + "#" + id)

            # Set a class for external links
            text = re.sub(r'(href="http.*?")', r'\1 class="external"', text)
            # Update hash style for internal links
            text = re.sub(r'href="#(.*?)"', r'''href="#\1" onclick="location='#\1'; updateHash();"''', text)
            output.write(text)

# Create the html help project file
with open(os.path.join('html', 'help.hhp'), 'w') as f:
    f.write(
"""[OPTIONS]
Compatibility=1.1 or later
Compiled file=XWord.chm
Contents file=contents.hhc
Default topic=index.html
Display compile progress=No
Language=0x409 English (United States)


[FILES]
""")
    for filename in files:
        f.write(filename + '\n')

    f.write("""\n\n[INFOTYPES]\n""")


# Create the table of contents
with open(os.path.join('html', 'contents.hhc'), 'w') as f:

    def write_list(l):
        f.write("<UL>\n")
        for item in l:
            write_item(item)
        f.write("</UL>\n")

    def write_item(item):
        f.write("<LI>")
        f.write('<OBJECT type="text/sitemap">\n' + 
                '    <param name="Name" value="%s">\n'
                '    <param name="Local" value="%s">\n'
                '</OBJECT>\n' % (item.name, item.link))
        if item.sublist:
            write_list(item.sublist)


    f.write("""
<!DOCTYPE HTML PUBLIC "-//IETF//DTD HTML//EN">
<HTML>
<HEAD>
<meta name="GENERATOR" content="Microsoft&reg; HTML Help Workshop 4.1">
<!-- Sitemap 1.0 -->
</HEAD><BODY>
<OBJECT type="text/site properties">
	<param name="ImageType" value="Folder">
</OBJECT>
""")

    write_list([contents[fn] for fn in contents_order])

    f.write("""</BODY></HTML>""")


#raw_input("Press any key to continue...")
