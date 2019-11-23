import os
import re

print("Checking links . . .")

links = {}
ids = set()

# Extract links from all the html files
for htmlfile in os.listdir('html'):
    if not htmlfile.endswith(".html"):
        continue
    with open(os.path.join('html', htmlfile), 'r') as input:
        text = input.read()
        for href in re.findall(r'<a href="(.*?)"', text):
            if href.startswith('http'):
                continue
            if href.startswith('#'):
                href = htmlfile + href
            if href not in links:
                links[href] = []
            links[href].append(htmlfile)
        for id in re.findall(r'id="(.*?)"', text):
            ids.add(htmlfile + '#' + id)
        ids.add(htmlfile)

# Check broken links
missing = set(links.keys()) - ids
if len(missing) > 0:
    print("Broken links:")
    for m in missing:
        print(m, links[m])
else:
    print("No broken links")
