import os

scripts_dir = '..\\scripts'

for root, dirs, files in os.walk(scripts_dir):
    if '_' not in root and 'debug' not in root:
        print('SetOutPath "$SCRIPTSDIR' + root[len(scripts_dir):] + '"')
        for filename in files:
            if filename.endswith('.lua') or filename.endswith('.dll') or filename.endswith('.png'):
                print('    File "${XWORD_TRUNK}' + os.path.join(root[2:],filename) + '"')
        print()
